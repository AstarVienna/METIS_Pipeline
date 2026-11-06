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
"""

from cpl.core import Image as CplImage, Msg


def locate_pinhole(cimg: CplImage, hwidth: int) -> dict[str, float]:
    """
    Locate a single bright pinhole image on `cimg`.

    Parameters
    ----------
    cimg : cpl.core.Image
        image containing one dominant point-like source
    hwidth : int
        half-width of the window around the brightest pixel [pix]

    Returns
    -------
    A dictionary with keys
    - xcen, ycen : [pix] centroid of the pinhole image (1-based, CPL convention)
    - xcenstd, ycenstd : [pix] uncertainty of the centroid (not implemented, always 0.0)
    - fwhm_x, fwhm_y : [pix] full width at half maximum in x and y;
      999.0 if the fit failed
    - snr : total flux in the window divided by the image pixel stdev
      times the window side length

    Note
    ----
    The algorithm is fairly rough. Its robustness depends on the pinhole image
    having high signal-to-noise without being saturated.
    """
    # Rough location: brightest pixel
    y0, x0 = cimg.get_maxpos()

    # Analyse window around maximum position
    llx = max(x0 - hwidth, 0)
    lly = max(y0 - hwidth, 0)
    urx = min(x0 + hwidth, cimg.shape[1] - 1)
    ury = min(y0 + hwidth, cimg.shape[0] - 1)
    win = (llx, lly, urx, ury)

    xcen, ycen = cimg.get_centroid_x(window=win), cimg.get_centroid_y(window=win)
    fwhm_y, fwhm_x = cimg.get_fwhm(y0, x0)

    # Signal-to-noise ratio using flux over the window and pixel noise over the image
    flux = cimg.get_flux(win)
    noise = cimg.get_stdev() * 2 * hwidth

    if fwhm_x is None or fwhm_y is None:
        Msg.warning("locate_pinhole", "Detection of pinhole failed")
        fwhm_x = 999.0
        fwhm_y = 999.0

    return {
        "xcen": xcen,
        "ycen": ycen,
        "fwhm_x": fwhm_x,
        "fwhm_y": fwhm_y,
        "xcenstd": 0.0,  # ToDo fix
        "ycenstd": 0.0,  # ToDo fix
        "snr": flux / noise,
    }
