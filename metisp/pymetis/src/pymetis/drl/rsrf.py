"""
This file is part of an A* Pipeline.
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

-------------------------------------------------------------------------------

Building blocks of relative spectral response function (RSRF) recipes:
a blackbody continuum image from a wavelength map, and extraction of 1D
spectra along spectral traces.
"""

import cpl
import numpy as np

from pymetis.drl.trace_model import Trace

ma = np.ma

# Number of points at which the Planck function is tabulated before being interpolated
# onto the pixels. The function is smooth over the tens of nanometres one detector
# covers, so this is far more than accuracy requires.
BLACKBODY_SAMPLES = 4096


def create_blackbody_image(wavecal_img: cpl.core.Image, bb_temp: float) -> cpl.core.Image:
    """
    Create a blackbody image from the wavelength calibration image.

    Each valid pixel is filled with the Planck flux at that pixel's wavelength. Pixels
    with no wavelength, which the wavelength map marks with a zero, are left at zero and
    then rejected, so that dividing by this image cannot introduce garbage.

    Parameters
    ----------
    wavecal_img : cpl.core.Image
        Wavelength at each pixel, in microns, and zero where there is none.
    bb_temp : float
        Blackbody temperature in Kelvin.

    Returns
    -------
    cpl.core.Image
        The blackbody image, with the invalid pixels rejected.

    Raises
    ------
    ValueError
        If the wavelength map holds no valid pixel at all. There is nothing sensible to
        return in that case, and continuing would divide the flat by an entirely
        rejected image.

    Notes
    -----
    The Planck function is evaluated on a fixed grid spanning the wavelengths present and
    then interpolated, rather than at every distinct value found in the map. A genuine
    two-dimensional wavelength solution gives a different wavelength in almost every
    pixel, so tabulating each one would mean millions of `fill_blackbody` entries and a
    per-pixel search; interpolating a smooth function over a few thousand samples is
    equivalent to well below the numerical precision of the result.
    """
    wdata = wavecal_img.as_array()
    valid = wdata > 0

    if not np.any(valid):
        raise ValueError(
            "The wavelength calibration image contains no valid pixel: every value is "
            "zero, so no blackbody spectrum can be evaluated. This usually means "
            "the wavelength calibration recipe produced an empty map because the "
            "distortion table held no traces; check the trace count QC parameter of the distortion recipe."
        )

    low, high = float(wdata[valid].min()), float(wdata[valid].max())
    if high == low:
        # A single wavelength needs no interpolation, but np.interp requires two points
        grid = np.array([low, low + 1.0e-9])
    else:
        grid = np.linspace(low, high, BLACKBODY_SAMPLES)

    # Calculate the black-body flux across the grid [wavelengths in metres]
    flux = cpl.drs.photom.fill_blackbody(cpl.drs.photom.Unit.LESS,     # output unit
                                         cpl.core.Vector(grid / 1e6),  # input, in metres
                                         cpl.drs.photom.Unit.LENGTH,   # input unit
                                         bb_temp)                      # temperature [K]

    # Interpolate onto every valid pixel at once; invalid pixels stay at zero
    bb_data = np.zeros_like(wdata)
    bb_data[valid] = np.interp(wdata[valid], grid, np.asarray(list(flux), dtype=float))

    # mask the zero values with the bad-pixel mask to avoid division by zero
    bb_img = cpl.core.Image(bb_data)
    bb_img.reject_value({0})

    return bb_img


def extract_1d_spectra(img: cpl.core.Image,
                       trace_list: list[Trace],
                       trace_width: int | None = None) -> list[np.ndarray]:
    """
    Extract 1D spectra from the given image using the provided list of
    spectral trace coordinates.
    TODO: replace with PyReduce extract functionality including trace tilt.

    Parameters
    ----------
    img : cpl.core.Image
        The image from which to extract the spectra.
    trace_list : list[Trace]
        A list of Trace objects representing the spectral traces.
    trace_width : int | None
        Half-height of the band around the trace centre that is averaged.
        If None, half of the trace's own height is used.
        TODO: replace with width factor to be multiplied by the trace height.

    Returns
    -------
    list of 1D arrays
        One spectrum per trace, each as wide as the image; columns outside the
        trace's column range are left at zero.
    """
    # copy data to np.masked_array for processing
    rej_mask = np.array(img.bpm)  # set bad pixels
    mdata = ma.masked_array(img.as_array(), mask=rej_mask)
    imwidth = img.width

    # TODO: check that traces are within the image bounds

    # create a list of 1D RSRF curves (width is the image width)
    rsrf_1d_list = []
    for trace in trace_list:
        x_arr = np.arange(trace.column_range[0], trace.column_range[1])
        y_arr = np.array(trace.y_at_x(x_arr))
        if trace_width is None:
            trace_width = trace.height / 2
        rsrf_1d = np.zeros(imwidth, dtype=float)
        for i, x in enumerate(x_arr):
            yc = y_arr[i]
            rsrf_1d[int(x)] = mdata[int(yc - trace_width):int(yc + trace_width), int(x)].mean()
        rsrf_1d_list.append(rsrf_1d)

    return rsrf_1d_list
