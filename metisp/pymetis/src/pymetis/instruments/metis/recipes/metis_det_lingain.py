"""
This file is part of the METIS Pipeline.
Copyright (C) 2024 European Southern Observatory

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
"""
import itertools
import re

from typing import Literal, Dict, Any, Self

import cpl
from cpl.core import Msg
from numpy._typing import NDArray

from pymetis.engine.core.classes.image import EnhancedImage
from pymetis.engine.core.classes.utilities import Stopwatch
from pymetis.engine.core.functions.polyfit import weighted_polyfit
from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.functions.dummy import create_dummy_header
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.instruments.metis.dataitems.badpixmap import BadPixMap
from pymetis.instruments.metis.dataitems.gainmap import GainMap
from pymetis.instruments.metis.dataitems.linearity.linearity import LinearityMap
from pymetis.instruments.metis.dataitems.linearity.raw import LinearityRaw
from pymetis.instruments.metis.inputs import RawInput, BadPixMapInput, OptionalInputMixin
from pymetis.instruments.metis.recipes.base import MetisRecipeImpl
from pymetis.instruments.metis.recipes.prefab import RawImageProcessor
from pymetis.instruments.metis.qc.lingain import (LinGainMean, LinGainRms, LinNumBadpix, LinMinFlux, LinMaxFlux,
                                                  GainLin, GainCoeff)
import numpy as np
import astropy.stats



class MetisDetLinGainImpl(RawImageProcessor, MetisRecipeImpl):
    class InputSet(RawImageProcessor.InputSet):
        class RawInput(RawInput):
            Item = LinearityRaw
        class BadPixMapInput(OptionalInputMixin, BadPixMapInput):
            Item = BadPixMap

    class ProductSet(PipelineProductSet):
        GainMap = GainMap
        Linearity = LinearityMap
        BadPixMap = BadPixMap

    class Qc(QcParameterSet):
        LinGainMean = LinGainMean
        LinGainRms = LinGainRms
        LinNumBadpix = LinNumBadpix
        LinMinFlux = LinMinFlux
        LinMaxFlux = LinMaxFlux
        GainLin = GainLin
        GainCoeff = GainCoeff

    def __init__(self,
                 recipe: 'Recipe',
                 frameset: cpl.ui.FrameSet,
                 settings: Dict[str, Any]) -> None:
        super().__init__(recipe, frameset, settings)
       
        self.kappa = self.parameters["metis_det_lingain.kappa"].value
        self.fitdegree = self.parameters["metis_det_lingain.fitdegree"].value
        self.linlimit = self.parameters["metis_det_lingain.linlimit"].value
        self.truelimit = self.parameters["metis_det_lingain.truelimit"].value

        # ToDo Move this out to instrument/metis or even IRDB.
        self.detector_size = 2048
        self.border_2rg = 64
        self.border_geo = 28
        self.border_ifu_x = 64
        self.border_ifu_y = 32

        self.median_cutoff = 2000
        self.ipc_alpha0 = 0.02  # alpha_edge EXTERNAL CALIBRATION
        self.ipc_alpha0_prime = 0.002  # alpha_corner EXTERNAL CALIBRATION

    def _gain_correction_factor(self, tech):
        """
        IPC parameters, probably should be part of an external calibration product.
        https://ui.adsabs.harvard.edu/abs/2016PASP..128i5001K/abstract for H4RG. see also IRDB METIS.
        IPC matrix (3x3), equation 8 of https://ui.adsabs.harvard.edu/abs/2016PASP..128i5001K/abstract
        alpha0_prime, alpha0                    , alpha0_prime
        alpha0      , 1-4*alpha0_prime-4*alpha0 , alpha0
        alpha0_prime, alpha0                    , alpha0_prime
        """
        if 'LM' in tech or 'IFU' in tech:
            # equation 9 of https://arxiv.org/abs/2509.08810 (Euclid)
            return (1 - 2 * (4 * self.ipc_alpha0 + 4 * self.ipc_alpha0_prime) +
                    (4 * self.ipc_alpha0 + 4 * self.ipc_alpha0_prime) ** 2 +
                    (4 * self.ipc_alpha0 ** 2 + 4 * self.ipc_alpha0_prime ** 2))  # this needs to depend on detector
        elif 'N' in tech:
            return 1
        else:
            raise cpl.core.IllegalInputError(f"Unknown ESO DPR TECH {tech}")

    @staticmethod
    def split_dits(fws: NDArray, dits: NDArray[np.float64], un_on: NDArray[int]) -> tuple[NDArray[np.bool_], NDArray[np.bool_]]:
        return (dits == un_on) & (fws != 'closed'), (dits == un_on) & (fws == 'closed')

    def _get_detector_mask(self, tech, detector) -> NDArray[bool]:
        """
        A mask to ignore the masked pixels at the edge of the detector.
        EXTERNAL CALIBRATION, in case of the IFU the mask needs to only cover the visible traces.
        This needs to depend on detector because the LMS mask varies.
        """
        xx, yy = np.meshgrid(np.arange(self.detector_size), np.arange(self.detector_size))

        if 'LM' in tech:
            return (((xx >= self.border_2rg) & (xx < (self.detector_size - self.border_2rg))) &
                    ((yy >= self.border_2rg) & (yy < (self.detector_size - self.border_2rg))))
        elif 'N' in tech:
            return (((xx >= self.border_geo) & (xx < (self.detector_size - self.border_geo))) &
                    ((yy >= self.border_geo) & (yy < (self.detector_size - self.border_geo))))
        elif 'IFU' in tech:
            # Detector 1 and 2 are butted against each other in 1 dimension. Same for detectors 3 and 4.
            if detector in [1, 3]:
                return (((xx >= self.border_ifu_x) & (xx < self.detector_size)) &
                        ((yy >= self.border_ifu_y) & (yy < (self.detector_size - self.border_ifu_y))))
            elif detector in [2, 4]:
                return (((xx >= 0) & (xx < (self.detector_size - self.border_ifu_x))) &
                        ((yy >= self.border_ifu_y) & (yy < (self.detector_size - self.border_ifu_y))))
            else:
                raise cpl.core.IllegalInputError(f"Detector ID {detector} not recognised")
        else:
            raise cpl.core.IllegalInputError(f"Unknown ESO DPR TECH {tech}")

    def set_detector_characteristics(self, tech) -> Self:
        """
        Get detector characteristics:
        - gain correction factor (dimensionless)
        - gain (e- / ADU)
        - read noise (ADU)
        - dark current (ADU / s)
        """
        self.gcf = self._gain_correction_factor(tech)
        if 'LM' in tech:
            self.gain = 4.0
            self.read_noise = 70 / self.gain
            self.dark_current = 0.05 / self.gain
        elif 'N' in tech:
            self.gain = 201
            self.read_noise = 300 / self.gain
            self.dark_current = 1e5 / self.gain
        elif 'IFU' in tech:
            self.gain = 2.0
            self.read_noise = 70 / self.gain
            self.dark_current = 0.1 / self.gain
        else:
            raise cpl.core.IllegalInputError(f"Unknown ESO DPR TECH {tech}")
        return self

    def bootstrap_gain_error(
            self,
            images,
            fws,
            dits,
            sel_mask,
            *,
            draws: int = 100,
    ) -> np.floating[Any]:
        storegain = np.zeros(draws)

        for i_win in np.arange(draws):
            window = np.random.choice(np.sum(sel_mask), size=np.sum(sel_mask),
                                      replace=True)  # this redraws the valid pixels
            meanflux = np.zeros_like(self.unique_on)
            varflux = np.zeros_like(self.unique_on)

            Msg.debug(self.__class__.__qualname__,
                      f"Bootstrap iteration {i_win:3d} of {draws:3d}")

            for i_on, un_on in enumerate(self.unique_on):
                if self.unique_on_counts[i_on] >= 2:
                    # Safe-slice: see note at the equivalent check above.
                    off_match = self.unique_off_counts[self.unique_off == un_on]

                    if off_match.size > 0 and off_match[0] >= 2:
                        sel_dits_on, sel_dits_off = self.split_dits(fws, dits, un_on)

                        data_on1 = images[sel_dits_on][0][sel_mask][window]
                        data_on2 = images[sel_dits_on][1][sel_mask][window]
                        data_off1 = images[sel_dits_off][0][sel_mask][window]
                        data_off2 = images[sel_dits_off][1][sel_mask][window]

                        # TODO there should likely also be some logic to compensate the gain for the linearity (see the paper on Euclid detector characterization)

                        # Mortara and Fowler mean-variance method. https://ui.adsabs.harvard.edu/abs/1981SPIE..290...28M/abstract

                        meanflux[i_on] = (np.mean(data_on1 - data_off1) + np.mean(data_on2 - data_off2)) / 2
                        varflux[i_on] = np.array(np.std(data_on1 - data_on2) ** 2 - np.std(data_off1 - data_off2) ** 2) / 2
                    else:
                        Msg.warning(self.__class__.__qualname__, "This combination does not have enough OFF frames")
                else:
                    Msg.warning(self.__class__.__qualname__, "This combination does not have enough ON frames")

            if np.sum(meanflux < self.linlimit) < 2:
                raise cpl.core.IllegalInputError(
                    "metis_det_lingain (bootstrap iter): not enough data "
                    f"points below linlimit ({self.linlimit}) to determine "
                    "the gain in this bootstrap window. See the comment on "
                    "the equivalent nominal-gain check above.")

            p, cov_p = np.polyfit(meanflux[meanflux < self.linlimit],
                                  varflux[meanflux < self.linlimit],
                                  deg=1, cov=True)
            storegain[i_win] = 1 / p[0] * self.gcf

        return np.std(storegain)

    def _fit_linearity_loop(
        self,
        fluxes_on: NDArray[np.float64],
        dits_fluxrates: NDArray[np.float64],
        sel_mask: NDArray[np.bool_],
    ) -> tuple[NDArray[np.float64], NDArray[np.float64], NDArray[np.bool_]]:
        """
        Per-pixel linearity fit via an explicit Python loop over the detector.

        For every pixel selected by ``sel_mask`` the correction function (~1 in the linear
        regime) is fit against flux, using only samples below ``self.linlimit`` and
        normalising to a weighted-average 'true flux' built from samples below
        ``self.truelimit``.

        Inputs:
        - fluxes_on: (N, H, W) on-frame flux cube (not dark-subtracted)
        - dits_fluxrates: (N,) DIT rate per frame
        - sel_mask: (H, W) True where the pixel is worth fitting
        - gaincorrection_factor, gain, read_noise: detector scalars

        Returns ``(linearity, err_linearity, bpm)``:
        - linearity: (fitdegree+1, H, W) coefficients, highest degree first
        - err_linearity: (fitdegree+1, H, W) 1-sigma coefficient errors (diagonal only)
        - bpm: (H, W) bad-pixel mask, initialised from ``~sel_mask`` plus fit failures
        """
        n, height, width = fluxes_on.shape
        linearity = np.zeros((self.fitdegree + 1, height, width))
        err_linearity = np.zeros((self.fitdegree + 1, height, width))
        bpm = ~sel_mask

        for i_x in range(0, height):

            # TODO additional optional bad pixel masking should go here

            fluxes_x = fluxes_on[:, i_x, :] # note that for the linearity calculation this is not dark-subtracted.

            if i_x % 64 == 0:
                Msg.debug(self.__class__.__qualname__,
                          f"Now at row {i_x:4d} of {height:4d}")

            for i_y in range(0, width):
                fluxes_x_y = fluxes_x[:, i_y] # working on a subarray is faster than directly indexing the 3D array
                if sel_mask[i_x, i_y] == 1: # only fit pixels that are not known to be bad
                    sel = (fluxes_x_y < self.linlimit) # only fit pixel values within the linlimit
                    truesel = (fluxes_x_y < self.truelimit)

                    if np.sum(sel) < self.fitdegree + 1:
                        # If there are not enough below-linlimit samples to fit this pixel mark it bad and skip
                        Msg.debug(self.__class__.__qualname__,
                                  f"Pixel ({i_x}, {i_y}): too few below-linlimit samples; marking bad")
                        bpm[i_x, i_y] = 1
                        continue

                    # Define a weighted average of the pixels below a certain 'true flux' cutoff.
                    # This defines a weighted average flux rate to which all flux rates are corrected.
                    trueflux = np.average(
                        fluxes_x_y[truesel] / dits_fluxrates[truesel],
                        weights=1 / (np.sqrt(self.gcf) *
                                     np.sqrt(self.read_noise ** 2 + fluxes_x_y[truesel] / (2 * self.gain)) /
                                     dits_fluxrates[truesel]
                        )**2
                    )

                    # define standard error on the weighted average
                    e_trueflux = np.sqrt(
                        1 / np.sum(
                            (1 / np.sqrt(self.gcf) *
                             np.sqrt(self.read_noise **2 + fluxes_x_y[truesel] / (2 * self.gain)) /
                             dits_fluxrates[truesel])**2
                        )
                    )

                    # the function that is fit is the correction function (which is ~1 when in the most linear regime) as function of the flux
                    try:
                        p, cov_p = np.polyfit(fluxes_x_y[sel],
                                              trueflux / (fluxes_x_y[sel] / dits_fluxrates[sel]),
                                              deg=self.fitdegree,
                                              w=trueflux / (
                                                  np.sqrt(self.gcf) *
                                                  np.sqrt(self.read_noise ** 2 + fluxes_x_y[sel] / (2 * self.gain)) /
                                                  dits_fluxrates[sel]
                                              ),
                                              cov='unscaled')
                        linearity[:, i_x, i_y] = p
                        err_linearity[:, i_x, i_y] = np.sqrt(np.diag(cov_p))
                        # This ignores the covariance term, but HDRL error propagation doesn't do covariance,
                        # and it would be hard to store a covariance matrix per pixel as CPL only
                        # does 3D objects per extension.
                    except:
                        bpm[i_x, i_y] = 1 # this pixel failed for some reason, so let's add it to the BPM

        # Reject pixels whose fitted coefficients are statistical outliers, adding them to the BPM.
        for i in range(self.fitdegree + 1): # check every polynomial coefficient
            linearity_ma = np.ma.masked_array(linearity[i, :, :], mask=~sel_mask) # only consider pixels with good values
            bpm += astropy.stats.sigma_clip(linearity_ma, sigma=self.kappa).mask # reject outliers, TODO: need to investigate the HDRL equivalent

        bpm[bpm > 1] = 1 # truncate so it can act as a boolean, maybe this can be done with an OR in the previous line

        return linearity, err_linearity, bpm

    def _fit_linearity_vec(
        self,
        fluxes_on: NDArray[np.float64],
        dits_fluxrates: NDArray[np.float64],
        sel_mask: NDArray[np.bool_],
    ) -> tuple[NDArray[np.float64], NDArray[np.float64], NDArray[np.bool_]]:
        """
        Vectorized equivalent of :meth:`_fit_linearity_loop`: fits every pixel in a single
        :func:`weighted_polyfit` call. Same inputs and outputs.

        The below-linlimit cut is ragged across pixels, so it is applied as zero weight
        rather than by indexing; non-finite corrections (e.g. zero-flux samples) are
        likewise dropped so they cannot poison a pixel's fit.
        """
        n, height, width = fluxes_on.shape
        linearity = np.zeros((self.fitdegree + 1, height, width))
        err_linearity = np.zeros((self.fitdegree + 1, height, width))
        bpm = ~sel_mask

        dits = dits_fluxrates[:, None, None]                     # (N, 1, 1)
        flux_rate = fluxes_on / dits                             # (N, H, W)
        sel = fluxes_on < self.linlimit                          # samples used in the fit
        truesel = fluxes_on < self.truelimit                     # samples used for the trueflux average

        with np.errstate(divide="ignore", invalid="ignore"):
            # Weighted-average flux rate ('true flux') per pixel, over samples below truelimit.
            true_w = np.where(
                truesel,
                1 / (np.sqrt(self.gcf) *
                     np.sqrt(self.read_noise ** 2 + fluxes_on / (2 * self.gain)) / dits) ** 2,
                0.0,
            )
            trueflux = np.sum(true_w * flux_rate, axis=0) / np.sum(true_w, axis=0)   # (H, W)

            # standard error on the weighted average (per pixel)
            e_trueflux = np.sqrt(1 / np.sum(
                np.where(
                    truesel,
                    (1 / np.sqrt(self.gcf) *
                     np.sqrt(self.read_noise ** 2 + fluxes_on / (2 * self.gain)) / dits) ** 2,
                    0.0,
                ),
                axis=0,
            ))                                                   # (H, W)

            corr = trueflux[None] / flux_rate                    # (N, H, W)
            fit_w = trueflux[None] / (np.sqrt(self.gcf) *
                                      np.sqrt(self.read_noise ** 2 + fluxes_on / (2 * self.gain)) / dits)
        good = sel & np.isfinite(corr) & np.isfinite(fit_w)

        p, cov_p, ok = weighted_polyfit(fluxes_on, np.where(good, corr, 0.0),
                                        deg=self.fitdegree, weights=np.where(good, fit_w, 0.0))
        linearity[:] = p
        # Per-pixel 1-sigma coefficient errors (diagonal of each pixel's covariance matrix).
        err_linearity[:] = np.moveaxis(np.sqrt(np.diagonal(cov_p, axis1=0, axis2=1)), -1, 0)
        bpm[sel_mask & ((good.sum(axis=0) < self.fitdegree + 1) | ~ok)] = 1

        # Reject pixels whose fitted coefficients are statistical outliers, adding them to the BPM.
        for i in range(self.fitdegree + 1): # check every polynomial coefficient
            linearity_ma = np.ma.masked_array(linearity[i, :, :], mask=~sel_mask) # only consider pixels with good values
            bpm += astropy.stats.sigma_clip(linearity_ma, sigma=self.kappa).mask # reject outliers, TODO: need to investigate the HDRL equivalent

        bpm[bpm > 1] = 1 # truncate so it can act as a boolean, maybe this can be done with an OR in the previous line

        return linearity, err_linearity, bpm

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        det_prefix = rf'DET{detector:1d}'

        raw_images = self.inputset.raw.load_data(rf'{det_prefix}.DATA') # this is an ImageList
        length = len(raw_images)
       
        # TODO we would likely need to apply "bias" corrections based on the reference pixels when we read in the data
       
        fws = []
        dits = []
        images = []
        headers = []

        for i_frame in range(length):
            header = cpl.core.PropertyList.load(self.inputset.raw.frameset[i_frame].file, 0)
            headers.append(header)
            #print(header_linearity['ESO DET1 DIT'].value)
            #print(header_linearity['ESO DRS NDFILTER'].value)
            fws.append(header['ESO DRS FILTER'].value)
            dits.append(header['ESO DET DIT'].value)
            images.append(raw_images[i_frame].as_array())

        if len(techs := list(set([header['ESO DPR TECH'].value for header in headers]))) != 1:
            raise cpl.core.IllegalInputError(f"More than one ESO DPR TECH detected in {headers}: {techs}")
        else:
            self.tech = techs[0]
            self.set_detector_characteristics(self.tech)

        dits = np.array(dits)
        fws = np.array(fws)
       
        images = np.array(images)

        self.unique_on, self.unique_on_counts = np.unique(dits[fws != 'closed'], return_counts=True)
        self.unique_off, self.unique_off_counts = np.unique(dits[fws == 'closed'], return_counts=True)

        meanflux = np.zeros_like(self.unique_on)
        varflux = np.zeros_like(self.unique_on)
        dits_fluxrates = []

        sel_mask = self._get_detector_mask(self.tech, detector)

        if 'IFU' in self.tech:
            slit_mask = np.percentile(images, 70, axis=0) > 2000
            sel_mask &= slit_mask # TODO additional optional bad pixel masking and windowing should go here

        fluxes_on = np.zeros(shape=(len(self.unique_on), self.detector_size, self.detector_size))
       
        # calculation of nominal gain value using all pixels
        # TODO a lot of this code is replicated later, so this should be a function.

        for i_on, un_on in enumerate(self.unique_on): # loop over unique dit values
            if self.unique_on_counts[i_on] >= 2: # check if there are at least 2 frames of each DIT.
                # Safe-slice; otherwise returns "truth value of an array ... is ambiguous" error
                off_match = self.unique_off_counts[self.unique_off == un_on]

                if off_match.size > 0 and off_match[0] >= 2:
                    sel_dits_on, sel_dits_off = self.split_dits(fws, dits, un_on)

                    data_on1 = images[sel_dits_on][0][sel_mask]
                    data_on2 = images[sel_dits_on][1][sel_mask]
                    data_off1 = images[sel_dits_off][0][sel_mask]
                    data_off2 = images[sel_dits_off][1][sel_mask]

                    # note that this doesn't dark subtract the fluxes. linearity also acts on dark current according to CRIRES.
                    fluxes_on[i_on, :, :][sel_mask] = (data_on1 + data_on2) / 2
                    dits_fluxrates.append(un_on)
                   
                    # TODO there should likely also be some logic to compensate the gain for the linearity (see the paper on Euclid detector characterization)
           
                    # Mortara and Fowler mean-variance method. https://ui.adsabs.harvard.edu/abs/1981SPIE..290...28M/abstract
               
                    meanflux[i_on] = (np.mean(data_on1 - data_off1) + np.mean(data_on2 - data_off2)) / 2
                    varflux[i_on] = np.array(np.std(data_on1 - data_on2) ** 2 - np.std(data_off1 - data_off2) ** 2) / 2
                else:
                    Msg.warning(self.__class__.__qualname__, "This combination does not have enough OFF frames")
            else:
                Msg.warning(self.__class__.__qualname__, "This combination does not have enough ON frames")

        dits_fluxrates = np.array(dits_fluxrates)

        if np.sum(meanflux < self.linlimit) < 2:
            raise cpl.core.IllegalInputError(
                "metis_det_lingain: not enough data points below linlimit "
                f"({self.linlimit}) to determine the gain "
                f"(meanflux = {meanflux.tolist()}, self.unique_on = {self.unique_on.tolist()}). "
                "Cause: no ON/OFF frame pairs at matching DIT, or all "
                "frames are above linlimit. If running under EDPS, the "
                "workflow's lingain pre-filter should have caught this earlier.")

        p, cov_p = np.polyfit(meanflux[meanflux < self.linlimit],
                              varflux[meanflux < self.linlimit],
                              deg=1, cov=True) # this calculates the nominal gain
        gainval = 1 / p[0] * self.gcf

        Msg.info(self.__class__.__qualname__,
                 f"Nominal gain [e/ADU]: {gainval}")

        # bootstrapping the statistical error on the nominal gain value
        # we redraw the valid pixels and redetermine the gain based on those pixels
        # the standard deviation of these gains is a measure of the statistical error on the nominal gain calculated above
        gain_err = self.bootstrap_gain_error(images, fws, dits, sel_mask, draws=100)
        Msg.info(self.__class__.__qualname__,
                 f"Gain error [e/ADU]: {gain_err}")

        Msg.debug(self.__class__.__qualname__,
                  f"Now actually determining linearity...")

        # Both compute the same fit; the loop is the production path for now.
        with Stopwatch() as loop_sw:
            linearity, err_linearity, bpm = \
                self._fit_linearity_loop(fluxes_on, dits_fluxrates, sel_mask)

        with Stopwatch() as vec_sw:
            linearity_vec, err_linearity_vec, bpm_vec = \
                self._fit_linearity_vec(fluxes_on, dits_fluxrates, sel_mask)

        m = sel_mask
        # A small nonzero bpm mismatch count does not necessarily mean the fits diverge: the
        # sigma-clip inside each method is a threshold operation, so a pixel sitting right at the
        # kappa boundary can flip between the two paths purely from the minuscule coefficient difference.
        Msg.info(self.__class__.__qualname__,
                 f"Linearity vec-vs-loop: coeff max|d| = {np.max(np.abs(linearity[:, m] - linearity_vec[:, m])):.3e}, "
                 f"err max|d| = {np.max(np.abs(err_linearity[:, m] - err_linearity_vec[:, m])):.3e}, "
                 f"bpm mismatches = {int(np.sum(bpm != bpm_vec))}")
        Msg.info(self.__class__.__qualname__,
                 f"Linearity timing: "
                 f"loop = {loop_sw.elapsed:.3f} s, "
                 f"vectorized = {vec_sw.elapsed:.3f} s, "
                 f"speedup = {loop_sw.elapsed / vec_sw.elapsed:.2f}×")

        # TODO: QC parameters should be populated here
       
        header_linearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_errlinearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_dqlinearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_gain = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_badpix = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
       
        gain_table = cpl.core.Table(input=np.rec.fromarrays(np.array([[gainval], [gain_err]]),
                                                            names=["gain", "gain_err"]))
        linearity_image = cpl.core.ImageList([cpl.core.Image(data=linearity[i, :, :]) for i in range(self.fitdegree + 1)])
        err_linearity_image = cpl.core.ImageList([cpl.core.Image(data=err_linearity[i, :, :]) for i in range(self.fitdegree + 1)])
        dq_linearity_image = cpl.core.Image(data=np.int32(bpm)) # had to cast to integer, boolean gave CPL error
       
        return {
            'gain_map': Hdu(header_gain, gain_table, name=rf'{det_prefix}.SCI'),
            'linearity_map': EnhancedImage(
                linearity_image, err_linearity_image, dq_linearity_image,
                prefix=det_prefix,
                header_image=header_linearity,
                header_error=header_errlinearity,
                header_dq=header_dqlinearity,
            ),
            'badpix_map': Hdu(header_badpix, dq_linearity_image, name=rf'{det_prefix}.SCI'),
        }

    def process(self) -> set[DataItem]:
        Msg.info(self.__class__.__qualname__, f"Loading raw data")
        self.inputset.raw.load_structure()

        Msg.info(self.__class__.__qualname__, f"HDUs found: {list(self.inputset.raw.items[0].hdus.keys())}")

        detector_count = len(list(filter(lambda x: re.match(r'DET[0-9].DATA', x) is not None,
                                         self.inputset.raw.items[0].hdus.keys() - ['PRIMARY'])))

        primary_header_gain_map = create_dummy_header()
        primary_header_linearity = create_dummy_header()
        primary_header_badpix_map = create_dummy_header()

        all_hdus = [self._process_single_detector(detector) for detector in range(1, detector_count + 1)]

        product_gain_map = self.ProductSet.GainMap(
            primary_header_gain_map,
            *[output['gain_map'] for output in all_hdus]
        )

        items = list(itertools.chain.from_iterable([output['linearity_map'].as_list() for output in all_hdus]))

        product_linearity = self.ProductSet.Linearity(
            primary_header_linearity,
            *items,
        )
        product_badpix_map = self.ProductSet.BadPixMap(
            primary_header_badpix_map,
            *[output['badpix_map'] for output in all_hdus]
        )

        return {product_gain_map, product_linearity, product_badpix_map}


class MetisDetLinGain(Recipe):
    # Fill in recipe information
    _name = "metis_det_lingain"
    _version = "0.2"
    _author = "A*, ASIAA, Gilles Otten"
    _email = "hugo@buddelmeijer.nl"
    _synopsis = "Measure detector non-linearity and gain"
    _description = (
        "Function-level code to determine Gain and Linearity for the three subinstruments of METIS."
    )

    _algorithm = """We expect two on and two dark (here named off) images per DIT
    for the Gain and Linearity calculation, similar to ESO's DETMON.
    The gain is determined from the slope of the average flux
    of the on-off maps and the variance of the on-on and off-off maps.
    A gain correction factor is applied to correct for the inter pixel capacitance.
    The noise on the gain is derived by resampling with replacement.
    The linearity is determined following the CRIRES approach where for every pixel the flux rate
    (ADU/s; including dark current) is calculated and the weighted average of the flux rate
    at the lowest (most linear) fluxes are taken as the true ADU/s flux rate
    to which all flux rates should be corrected.
    This directly defines a correction factor as a function of flux with
    the most linear fluxes having a correction close to 1.
    An Nth order polynomial is fit (np.polyfit; taking into account errors) to these correction values as function of flux.
    We add pixels with linearity coefficients that are significantly different from the mean to the BPM (astropy.stats.sigma_clip)."""

    parameters = ParameterList([
        ParameterValue(
            name=rf"{_name}.fitdegree",
            context=_name,
            description="Degree of polynomial that is fit to linearity correction function",
            default=1,
        ),
        ParameterValue(
            name=rf"{_name}.kappa",
            context=_name,
            description="kappa factor for sigma clipping for BPM. "
                        "Values that deviate more than kappa*sigma from the median linearity are flagged.",
            default=3,
        ),
        ParameterValue(
            name=rf"{_name}.linlimit",
            context=_name,
            description="Limit in ADU above which counts are excluded from "
                        "both gain calculation and linearity correction function",
            default=22000., # this should be dependent on read out mode and detector
        ),
        ParameterValue(
            name=rf"{_name}.truelimit",
            context=_name,
            description="Limit in ADU to consider as true linear. All flux rates will be corrected "
                        "to the weighted average flux rate defined by the pixel values below this limit.",
            default=10000., # this should be dependent on read out mode and detector
        ),
    ])

    Impl = MetisDetLinGainImpl

