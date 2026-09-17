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

QC parameters of the high-contrast imaging (ADI) recipes.

The DRLD defines one set of six parameters per band and coronagraph,
`QC <det> <cgrph> SCI ...` (ADI_QC_items.tex), where `det` is the band (LM or N)
and `cgrph` the coronagraph (RAVC, CVC, APP). The templates below carry both axes;
the recipes use the leaves for the combinations they produce.
"""

from pymetis.engine.qc import QcParameter
from ..mixins import BandLmMixin, CgrphAppMixin


# -------------------------------
# Templates over band and coronagraph
# -------------------------------

class HciSciNExp(QcParameter):
    _name_template = "QC {band} {cgrph} SCI NEXP"
    _type = int
    _unit = "1"
    _description_template = "Effective number of exposures used to create the ADI data products"


class HciSciSnrMean(QcParameter):
    _name_template = "QC {band} {cgrph} SCI SNR MEAN"
    _type = float
    _unit = "1"
    _description_template = "Mean value in ADI SNR map"


class HciSciSnrPeak(QcParameter):
    _name_template = "QC {band} {cgrph} SCI SNR PEAK"
    _type = float
    _unit = "1"
    _description_template = "Peak value in ADI SNR map"


class HciSciContrastRawLamd(QcParameter):
    _name_template = "QC {band} {cgrph} SCI CONTRAST RAW LAMD"
    _type = float
    _unit = "mag"
    _description_template = "Raw contrast curve value at separation LAMD LDD"


class HciSciContrastAdiLamd(QcParameter):
    _name_template = "QC {band} {cgrph} SCI CONTRAST ADI LAMD"
    _type = float
    _unit = "mag"
    _description_template = "Post-ADI contrast curve value at separation LAMD LDD"


class HciSciFwhm(QcParameter):
    _name_template = "QC {band} {cgrph} SCI FWHM {nn}"
    _type = float
    _unit = "pixels"
    _description_template = "FWHM of the PSF in frame {nn}"


# -------------------------------
# LM-band APP leaves (metis_lm_adi_app)
# -------------------------------

class LmAppSciNExp(BandLmMixin, CgrphAppMixin, HciSciNExp):
    pass


class LmAppSciSnrMean(BandLmMixin, CgrphAppMixin, HciSciSnrMean):
    pass


class LmAppSciSnrPeak(BandLmMixin, CgrphAppMixin, HciSciSnrPeak):
    pass


class LmAppSciContrastRawLamd(BandLmMixin, CgrphAppMixin, HciSciContrastRawLamd):
    pass


class LmAppSciContrastAdiLamd(BandLmMixin, CgrphAppMixin, HciSciContrastAdiLamd):
    pass


class LmAppSciFwhm(BandLmMixin, CgrphAppMixin, HciSciFwhm):
    pass
