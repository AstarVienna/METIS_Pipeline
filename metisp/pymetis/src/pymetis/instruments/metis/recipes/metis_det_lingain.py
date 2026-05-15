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

import re

from abc import ABC
from typing import Literal, Dict, Any

import cpl
from cpl.core import Msg

from pymetis.engine.dataitems import DataItem, Hdu, PipelineProductSet
from pymetis.engine.qc import QcParameterSet
from pymetis.engine.core.dummy import create_dummy_header
from pymetis.engine.recipes import Recipe
from pymetis.engine.core.parameter import ParameterList, ParameterEnum, ParameterValue

from pymetis.instruments.metis.dataitems.badpixmap import BadPixMap
from pymetis.instruments.metis.dataitems.gainmap import GainMap
from pymetis.instruments.metis.dataitems.linearity.linearity import LinearityMap
from pymetis.instruments.metis.dataitems.linearity.raw import LinearityRaw
from pymetis.instruments.metis.inputs import RawInput, BadPixMapInput, OptionalInputMixin
from pymetis.instruments.metis.inputs.common import WcuOffInput
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

    def _process_single_detector(self, detector: Literal[1, 2, 3, 4]) -> dict[str, Hdu]:
        det = rf'DET{detector:1d}'

        raw_images = self.inputset.raw.load_data(rf'{det}.DATA') # this is an ImageList
        length=len(raw_images)
        
        # TODO we would likely need to apply "bias" corrections based on the masked regions of the images here when we read in the data        
        
        fws=[]
        dits=[]
        images=[]
        for i_frame in range(length):
            header_linearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[i_frame].file, 0)
            #print(header_linearity['ESO DET1 DIT'].value)
            #print(header_linearity['ESO DRS NDFILTER'].value)
            #print(header_linearity['ESO DRS FILTER'].value)
            fws.append(header_linearity['ESO DRS FILTER'].value)
            dits.append(header_linearity['ESO DET DIT'].value)
            images.append(raw_images[i_frame].as_array())
        tech=header_linearity['ESO DPR TECH'].value
        
        dits=np.array(dits)
        fws=np.array(fws)
        
        images=np.array(images)
        
        uni_on,uni_on_counts=np.unique(dits[(fws != 'closed')],return_counts=True)
        uni_off,uni_off_counts=np.unique(dits[(fws == 'open')],return_counts=True) # this is a hack because METIS_Simulations overrides the closed position
        
        xx,yy=np.meshgrid(np.arange(2048),np.arange(2048))
        xx2,yy2=np.meshgrid(np.arange(1920),np.arange(1920))

        meanflux=np.zeros([len(uni_on)])
        varflux=np.zeros([len(uni_on)])
        #fluxes=np.zeros([len(uni_on),1920,1920])
        fluxes_on=np.zeros([len(uni_on),1920,1920])
        dits_fluxrates=[]
        
        # IPC parameters, probably should be part of an external calibration product. https://ui.adsabs.harvard.edu/abs/2016PASP..128i5001K/abstract for H4RG. see also IRDB METIS.
        # IPC matrix (3x3), equation 8 of https://ui.adsabs.harvard.edu/abs/2016PASP..128i5001K/abstract 
        # alpha0_prime, alpha0                    , alpha0_prime
        # alpha0      , 1-4*alpha0_prime-4*alpha0 , alpha0
        # alpha0_prime, alpha0                    , alpha0_prime
        
        ipc_alpha0=0.02 #alpha_edge EXTERNAL CALIBRATION
        ipc_alpha0_prime=0.002 # alpha_corner EXTERNAL CALIBRATION
            
        
        if "LM" in tech:
            # equation 9 of https://arxiv.org/abs/2509.08810 (Euclid)
            gaincorrection_factor=1.-2*(4*ipc_alpha0+4*ipc_alpha0_prime)+(4*ipc_alpha0+4*ipc_alpha0_prime)**2+(4*(ipc_alpha0)**2+4*(ipc_alpha0_prime)**2) # this needs to depend on detector
            gain=4.0 # EXTERNAL CALIBRATION this needs to depend on detector, gain in electrons/ADU
            RN=70./gain # EXTERNAL CALIBRATION, readnoise and gain value just used for noise calculation ? this needs to depend on detector, readnoise in ADU
            DC=0.05/gain # dark current in electrons/s
            sel_mask=(((xx >= 64) & (xx<(2048-64))) & ((yy >= 64) & (yy<(2048-64)))) # a mask to ignore the masked pixels at the edge of the detector. EXTERNAL CALIBRATION, in case of the IFU the mask needs to only cover the visible traces. this needs to depend on detector because the LMS mask varies
        elif "N" in tech:
            gaincorrection_factor=1. # no IPC in scopesim for N band
            gain=201. #e/ADU
            RN=300./gain #ADU
            DC=1e5/gain #ADU/s
            sel_mask=(((xx >= 28) & (xx<(2048-28))) & ((yy >= 28) & (yy<(2048-28)))) # a mask to ignore the masked pixels at the edge of the detector. EXTERNAL CALIBRATION, in case of the IFU the mask needs to only cover the visible traces. this needs to depend on detector because the LMS mask varies
        elif "IFU" in tech:
            gaincorrection_factor=1.-2*(4*ipc_alpha0+4*ipc_alpha0_prime)+(4*ipc_alpha0+4*ipc_alpha0_prime)**2+(4*(ipc_alpha0)**2+4*(ipc_alpha0_prime)**2) # this needs to depend on detector
            gain=2.0 #e/ADU
            DC=0.1/gain # ADU/s
            RN=70/gain # ADU
            if detector == 1:
                sel_mask=(((xx >= 64) & (xx<(2048))) & ((yy >= 32) & (yy<(2048-32)))) #detector 1 and 2 are butted against eachother in 1 dimension. same for detectors 3 and 4.
            elif detector == 2:
                sel_mask=(((xx >= 0) & (xx<(2048-64))) & ((yy >= 32) & (yy<(2048-32))))
            elif detector == 3:
                sel_mask=(((xx >= 64) & (xx<(2048))) & ((yy >= 32) & (yy<(2048-32))))
            elif detector == 4:
                sel_mask=(((xx >= 0) & (xx<(2048-64))) & ((yy >= 32) & (yy<(2048-32))))
            else:
                Msg.error(self.__class__.__qualname__, "Do not recognize detector ID")
        else:
            Msg.error(self.__class__.__qualname__, "Do not recognize TECH")
                                
        # calculation of nominal gain value using all pixels
        # a lot of this code is replicated later, so this should be a function.
        
        for i_on,un_on in enumerate(uni_on): 
    
            if uni_on_counts[i_on] >= 2:
                if uni_off_counts[uni_off == un_on] >= 2:
                    sel_dits_on=((dits == un_on) & (fws != 'closed'))
                    sel_dits_off=((dits == un_on) & (fws == 'open'))

                    data_on1=images[sel_dits_on][0][sel_mask].reshape([1920,1920]) # not sure why I reshape for the gain calculation.
                    data_on2=images[sel_dits_on][1][sel_mask].reshape([1920,1920])
                    data_off1=images[sel_dits_off][0][sel_mask].reshape([1920,1920])
                    data_off2=images[sel_dits_off][1][sel_mask].reshape([1920,1920])
                    #fluxes[i_on,:,:]=(data_on1/2.+data_on2/2.-data_off1/2.-data_off2/2.)
                    fluxes_on[i_on,:,:]=(data_on1/2.+data_on2/2.)
                    dits_fluxrates.append(un_on)
                    
                    # TODO additional optional bad pixel masking should go here
                    
                    # TODO there should likely also be some logic to compensate the gain for the linearity (see the paper on Euclid detector characterization)
            
                    # Mortara and Fowler mean-variance method. https://ui.adsabs.harvard.edu/abs/1981SPIE..290...28M/abstract
                
                    meanflux[i_on]=np.mean(data_on1-data_off1)/2+np.mean(data_on2-data_off2)/2
                    varflux[i_on]=np.array(np.std(data_on1-data_on2)**2-np.std(data_off1-data_off2)**2)/2                
                                        
                else: 
                    Msg.warning(self.__class__.__qualname__, "This combination does not have enough OFF frames")
                    #print("Warning: this combination does not have enough OFF frames")
            else:
                Msg.warning(self.__class__.__qualname__, "This combination does not have enough ON frames")
        dits_fluxrates=np.array(dits_fluxrates)
        if np.sum(meanflux < self.linlimit) < 2:
            Msg.error(self.__class__.__qualname__, "This dataset does not have enough data within the linlimit to determine the gain")
            # TODO force stop of program
        p,cov_p=np.polyfit(meanflux[meanflux < self.linlimit],varflux[meanflux < self.linlimit],deg=1,cov=True) # this calculates the nominal gain
        Msg.info(self.__class__.__qualname__, f"nominal gain [e/ADU]: {1/p[0]*gaincorrection_factor}")
        
        linearity=np.zeros([self.fitdegree+1,1920,1920])
        err_linearity=np.zeros([self.fitdegree+1,1920,1920])
        gainarr=np.ones([1920,1920])*1/p[0]*gaincorrection_factor
        gainval=1/p[0]*gaincorrection_factor
        
        # bootstrapping the statistical error on the nominal gain value
        
        draws=100
        storegain=np.zeros(draws)

        for i_win in np.arange(draws):
            window=np.random.choice(1920*1920,size=1920*1920,replace=True)
            meanflux=np.zeros([len(uni_on)])
            varflux=np.zeros([len(uni_on)])
            for i_on,un_on in enumerate(uni_on):
                if uni_on_counts[i_on] >= 2:
                    if uni_off_counts[uni_off == un_on] >= 2:

                        sel_dits_on=((dits == un_on) & (fws != 'closed'))
                        sel_dits_off=((dits == un_on) & (fws == 'open'))

                        data_on1=images[sel_dits_on][0][sel_mask][window].reshape([1920,1920])
                        data_on2=images[sel_dits_on][1][sel_mask][window].reshape([1920,1920])
                        data_off1=images[sel_dits_off][0][sel_mask][window].reshape([1920,1920])
                        data_off2=images[sel_dits_off][1][sel_mask][window].reshape([1920,1920])

                        # TODO additional optional bad pixel masking should go here
                        
                        # TODO there should likely also be some logic to compensate the gain for the linearity (see the paper on Euclid detector characterization)

                        # Mortara and Fowler mean-variance method. https://ui.adsabs.harvard.edu/abs/1981SPIE..290...28M/abstract
                        
                        meanflux[i_on]=np.mean(data_on1-data_off1)/2+np.mean(data_on2-data_off2)/2
                        varflux[i_on]=np.array(np.std(data_on1-data_on2)**2-np.std(data_off1-data_off2)**2)/2
                        
                    else:
                        Msg.warning(self.__class__.__qualname__, "This combination does not have enough OFF frames")
                else:
                    Msg.warning(self.__class__.__qualname__, "This combination does not have enough ON frames")
            if np.sum(meanflux < self.linlimit) < 2:
                Msg.error(self.__class__.__qualname__, "This dataset does not have enough data within the linlimit to determine the gain")
                # TODO force stop of program
            
            p,cov_p=np.polyfit(meanflux[meanflux < self.linlimit],varflux[meanflux < self.linlimit],deg=1,cov=True)
            storegain[i_win]=1/p[0]*gaincorrection_factor

        gain_err=np.std(storegain)
                
        # linearity calculation
        
        for i_x in range(0,1920):
            
            # TODO additional optional bad pixel masking should go here
            
            fluxes_x=fluxes_on[:,i_x,:] # note that for the linearity calculation this is not dark-subtracted.
                
            for i_y in range(0,1920):
                fluxes_x_y=fluxes_x[:,i_y] # this is quicker than directly indexing the whole array
                
                sel=(fluxes_x_y<self.linlimit) # only fit pixel values within the linlimit
                truesel=(fluxes_x_y<self.truelimit)
                if np.sum(sel) < (self.fitdegree+1):
                    Msg.error(self.__class__.__qualname__, "This pixel does not have enough data within the linlimit to perform a fit at this fitorder") # TODO this might be too restrictive, it probably needs to capture the error in poly fit and then flag these as bad pixels. And then there can be a proper error if too many pixels were flagged in this way, which could indicate that the dataset is bad.
                    # TODO force stop of program
                    
                # define a weighted average of the pixels below a certain cutoff. this defines a weighted average flux rate to which all flux rates are corrected.
                trueflux=np.average(fluxes_x_y[truesel]/dits_fluxrates[truesel],weights=1/(np.sqrt(gaincorrection_factor)*np.sqrt(RN**2+fluxes_x_y[truesel]/(2*gain))/dits_fluxrates[truesel])**2)
                # define standard error on the weighted average
                e_trueflux=np.sqrt(1./np.sum((1./np.sqrt(gaincorrection_factor)*np.sqrt(RN**2+fluxes_x_y[truesel]/(2*gain))/dits_fluxrates[truesel])**2))
                #print("weighted average flux rate and error [adu/s]",trueflux,e_trueflux)

                p,cov_p=np.polyfit(fluxes_x_y[sel],trueflux/(fluxes_x_y[sel]/dits_fluxrates[sel]),deg=self.fitdegree,w=trueflux/(np.sqrt(gaincorrection_factor)*np.sqrt(RN**2+fluxes_x_y[sel]/(2*gain))/dits_fluxrates[sel]),cov='unscaled')
                
                linearity[:,i_x,i_y]=p
                err_linearity[:,i_x,i_y]=np.sqrt(np.diag(cov_p)) # this ignores the covariance term, but HDRL error propagation doesn't do covariance and it would be hard to store a covariance matrix per pixel as CPL only does 3D objects per extension.
        
        bpm=np.zeros([1920,1920])
        for i in range(self.fitdegree+1): # check every polynomial coefficient
            bpm=bpm+astropy.stats.sigma_clip(linearity[i,:,:],sigma=self.kappa).mask # reject outliers, TODO: need to investigate the HDRL equivalent
            bpm[bpm>1]=1 # truncate so it can act as a boolean
        
        # TODO: QC parameters should be populated here
        
        header_linearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_errlinearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_dqlinearity = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_gain = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)
        header_badpix = cpl.core.PropertyList.load(self.inputset.raw.frameset[0].file, 0)

        gain_table = cpl.core.Table(input=np.rec.fromarrays(np.array([[gainval],[gain_err]]),names=["gain","gain_err"]))
        
        linearity_image = cpl.core.ImageList([cpl.core.Image(data=linearity[i,:,:]) for i in range(self.fitdegree+1)])
        err_linearity_image = cpl.core.ImageList([cpl.core.Image(data=err_linearity[i,:,:]) for i in range(self.fitdegree+1)])
        dq_linearity_image = cpl.core.Image(data=bpm)

        return {
            'gain_map': Hdu(header_gain, gain_table, name=rf'{det}.SCI'),
            'linearity_map': Hdu(header_linearity, linearity_image, name=rf'{det}.SCI'),
            'err_linearity_map': Hdu(header_errlinearity, err_linearity_image, name=rf'{det}.ERR'),
            'dq_linearity_map': Hdu(header_dqlinearity, dq_linearity_image, name=rf'{det}.DQ'),
            'badpix_map': Hdu(header_badpix, dq_linearity_image, name=rf'{det}.SCI'),
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
        items=[output['linearity_map'] for output in all_hdus]+[output['err_linearity_map'] for output in all_hdus]+[output['dq_linearity_map'] for output in all_hdus]
        
        product_linearity = self.ProductSet.Linearity(
            primary_header_linearity,
            *items
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

    _algorithm = """We expect two on and two dark (here named off) images per DIT for the Gain and Linearity calculation, similar to ESO's DETMON.
    The gain is determined from the slope of the average flux of the on-off maps and the variance of the on-on and off-off maps. A gain correction factor is applied to correct for the inter pixel capacitance. The noise on the gain is derived by resampling with replacement.
    The linearity is determined following the CRIRES approach where for every pixel the flux rate (ADU/s; including dark current) is calculated and the weighted average of the flux rate at the lowest (most linear) fluxes are taken as the true ADU/s flux rate to which all flux rates should be corrected. This directly defines a correction factor as a function of flux with the most linear fluxes having a correction close to 1. An Nth order polynomial is fit (np.polyfit; taking into account errors) to these correction values as function of flux. We add pixels with linearity coefficients that are significantly different from the mean to the BPM (astropy.stats.sigma_clip)."""

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
            description="kappa factor for sigma clipping for BPM. Values that deviate more than kappa*sigma from the median linearity are flagged.",
            default=3,
        ),
        ParameterValue(
            name=rf"{_name}.linlimit",
            context=_name,
            description="Limit in ADU above which counts are excluded from both gain calculation and linearity correction function",
            default=22000., # this should be dependent on read out mode and detector
        ),
        ParameterValue(
            name=rf"{_name}.truelimit",
            context=_name,
            description="Limit in ADU to consider as true linear. All flux rates will be corrected to the weighted average flux rate defined by the pixel values below this limit.",
            default=10000., # this should be dependent on read out mode and detector
        ),
    ])

    Impl = MetisDetLinGainImpl
