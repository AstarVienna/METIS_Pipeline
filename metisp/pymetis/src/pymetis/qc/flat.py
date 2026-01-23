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

import cpl

from pymetis.classes.mixins import BandLmMixin, BandNMixin
from pymetis.classes.qc.parameter import QcParameter


# -------------------------------
# MFLAT base classes (LM/N merged)
# -------------------------------

class MFlatRmsBase(QcParameter):
    _name = "QC {band} MFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "RMS of the {band} master flat"

class MFlatMedianBase(QcParameter):
    _name = "QC {band} MFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Median of the {band} master flat"

class MFlatNbadpixBase(QcParameter):
    _name = "QC {band} MFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description = "Number of bad pixels in the {band} master flat"

# -------------------------------
# MFLAT LM/N variants
# -------------------------------

class LmMFlatRms(BandLmMixin, MFlatRmsBase):
    pass

class NMFlatRms(BandNMixin, MFlatRmsBase):
    pass

class LmMFlatMedian(BandLmMixin, MFlatMedianBase):
    pass

class NMFlatMedian(BandNMixin, MFlatMedianBase):
    pass

class LmMFlatNbadpix(BandLmMixin, MFlatNbadpixBase):
    pass

class NMFlatNbadpix(BandNMixin, MFlatNbadpixBase):
    pass

# -------------------------------
# MLFLAT base classes
# -------------------------------

class MlFlatRmsBase(QcParameter):
    _name = "QC {band} MLFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "RMS of the {band} lamp master flat"

class MlFlatMedianBase(QcParameter):
    _name = "QC {band} MLFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Median of the {band} lamp master flat"

class MlFlatNbadpixBase(QcParameter):
    _name = "QC {band} MLFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description = "Number of bad pixels in the {band} lamp master flat"

# -------------------------------
# MLFLAT LM/N variants
# -------------------------------

class LmMlFlatRms(BandLmMixin, MlFlatRmsBase):
    pass


class NMlFlatRms(BandNMixin, MlFlatRmsBase):
    pass


class LmMlFlatMedian(BandLmMixin, MlFlatMedianBase):
    pass


class NMlFlatMedian(BandNMixin, MlFlatMedianBase):
    pass


class LmMlFlatNbadpix(BandLmMixin, MlFlatNbadpixBase):
    pass


class NMlFlatNbadpix(BandNMixin, MlFlatNbadpixBase):
    pass

# -------------------------------
# MTFLAT base classes
# -------------------------------

class MtFlatRmsBase(QcParameter):
    _name = "QC {band} MTFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "RMS of the {band} twilight master flat"


class MtFlatMedianBase(QcParameter):
    _name = "QC {band} MTFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Median of the {band} twilight master flat"


class MtFlatNbadpixBase(QcParameter):
    _name = "QC {band} MTFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description = "Number of bad pixels in the {band} twilight master flat"

# -------------------------------
# MTFLAT LM/N variants
# -------------------------------

class LmMtFlatRms(BandLmMixin, MtFlatRmsBase):
    pass


class NMtFlatRms(BandNMixin, MtFlatRmsBase):
    pass


class LmMtFlatMedian(BandLmMixin, MtFlatMedianBase):
    pass


class NMtFlatMedian(BandNMixin, MtFlatMedianBase):
    pass


class LmMtFlatNbadpix(BandLmMixin, MtFlatNbadpixBase):
    pass


class NMtFlatNbadpix(BandNMixin, MtFlatNbadpixBase):
    pass

# -------------------------------
# Flat general base class
# -------------------------------

class FlatMeanBase(QcParameter):
    _name = "QC {band} FLAT MEAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Mean value of a single flat field image"

class FlatRmsBase(QcParameter):
    _name = "QC {band} FLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "RMS value of a single flat field image"

class FlatMedianMeanBase(QcParameter):
    _name = "QC {band} FLAT MEDIAN MEAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Mean value of the medians of input flat frames"

class FlatMedianMinBase(QcParameter):
    _name = "QC {band} FLAT MEDIAN MIN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Minimum value of the medians of input flat frames"

class FlatMedianMaxBase(QcParameter):
    _name = "QC {band} FLAT MEDIAN MAX"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "Maximum value of the medians of input flat frames"

class FlatMedianRmsBase(QcParameter):
    _name = "QC {band} FLAT MEDIAN RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description = "RMS value of the medians of input flat frames"

# -------------------------------
# Flat LM/N variants
# -------------------------------

class LmFlatMean(BandLmMixin, FlatMeanBase):
    pass

class NFlatMean(BandNMixin, FlatMeanBase):
    pass

class LmFlatRms(BandLmMixin, FlatRmsBase):
    pass

class NFlatRms(BandNMixin, FlatRmsBase):
    pass

class LmFlatMedianMean(BandLmMixin, FlatMedianMeanBase):
    pass

class NFlatMedianMean(BandNMixin, FlatMedianMeanBase):
    pass

class LmFlatMedianMin(BandLmMixin, FlatMedianMinBase):
    pass

class NFlatMedianMin(BandNMixin, FlatMedianMinBase):
    pass

class LmFlatMedianMax(BandLmMixin, FlatMedianMaxBase):
    pass

class NFlatMedianMax(BandNMixin, FlatMedianMaxBase):
    pass

class LmFlatMedianRms(BandLmMixin, FlatMedianRmsBase):
    pass

class NFlatMedianRms(BandNMixin, FlatMedianRmsBase):
    pass

# -------------------------------
# QC CalChophome parameters
# -------------------------------