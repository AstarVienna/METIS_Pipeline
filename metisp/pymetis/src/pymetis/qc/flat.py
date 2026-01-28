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

class MFlatRms(QcParameter):
    _name_template = "QC {band} MFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "RMS of the {band} master flat"

class MFlatMedian(QcParameter):
    _name_template = "QC {band} MFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Median of the {band} master flat"

class MFlatNbadpix(QcParameter):
    _name_template = "QC {band} MFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description_template = "Number of bad pixels in the {band} master flat"

# -------------------------------
# MFLAT LM/N variants
# -------------------------------

class LmMFlatRms(BandLmMixin, MFlatRms):
    pass

class NMFlatRms(BandNMixin, MFlatRms):
    pass

class LmMFlatMedian(BandLmMixin, MFlatMedian):
    pass

class NMFlatMedian(BandNMixin, MFlatMedian):
    pass

class LmMFlatNbadpix(BandLmMixin, MFlatNbadpix):
    pass

class NMFlatNbadpix(BandNMixin, MFlatNbadpix):
    pass

# -------------------------------
# MLFLAT base classes
# -------------------------------

class MlFlatRms(QcParameter):
    _name_template = "QC {band} MLFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "RMS of the {band} lamp master flat"

class MlFlatMedian(QcParameter):
    _name_template = "QC {band} MLFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Median of the {band} lamp master flat"

class MlFlatNbadpix(QcParameter):
    _name_template = "QC {band} MLFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description_template = "Number of bad pixels in the {band} lamp master flat"

# -------------------------------
# MLFLAT LM/N variants
# -------------------------------

class LmMlFlatRms(BandLmMixin, MlFlatRms):
    pass


class NMlFlatRms(BandNMixin, MlFlatRms):
    pass


class LmMlFlatMedian(BandLmMixin, MlFlatMedian):
    pass


class NMlFlatMedian(BandNMixin, MlFlatMedian):
    pass


class LmMlFlatNbadpix(BandLmMixin, MlFlatNbadpix):
    pass


class NMlFlatNbadpix(BandNMixin, MlFlatNbadpix):
    pass

# -------------------------------
# MTFLAT base classes
# -------------------------------

class MtFlatRms(QcParameter):
    _name_template = "QC {band} MTFLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "RMS of the {band} twilight master flat"


class MtFlatMedian(QcParameter):
    _name_template = "QC {band} MTFLAT MEDIAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Median of the {band} twilight master flat"


class MtFlatNbadpix(QcParameter):
    _name_template = "QC {band} MTFLAT NBADPIX"
    _type = cpl.core.Type.INT
    _unit = None
    _description_template = "Number of bad pixels in the {band} twilight master flat"

# -------------------------------
# MTFLAT LM/N variants
# -------------------------------

class LmMtFlatRms(BandLmMixin, MtFlatRms):
    pass


class NMtFlatRms(BandNMixin, MtFlatRms):
    pass


class LmMtFlatMedian(BandLmMixin, MtFlatMedian):
    pass


class NMtFlatMedian(BandNMixin, MtFlatMedian):
    pass


class LmMtFlatNbadpix(BandLmMixin, MtFlatNbadpix):
    pass


class NMtFlatNbadpix(BandNMixin, MtFlatNbadpix):
    pass

# -------------------------------
# Flat general base class
# -------------------------------

class FlatMean(QcParameter):
    _name_template = "QC {band} FLAT MEAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Mean value of a single flat field image"

class FlatRms(QcParameter):
    _name_template = "QC {band} FLAT RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "RMS value of a single flat field image"

class FlatMedianMean(QcParameter):
    _name_template = "QC {band} FLAT MEDIAN MEAN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Mean value of the medians of input flat frames"

class FlatMedianMin(QcParameter):
    _name_template = "QC {band} FLAT MEDIAN MIN"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Minimum value of the medians of input flat frames"

class FlatMedianMax(QcParameter):
    _name_template = "QC {band} FLAT MEDIAN MAX"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "Maximum value of the medians of input flat frames"

class FlatMedianRms(QcParameter):
    _name_template = "QC {band} FLAT MEDIAN RMS"
    _type = cpl.core.Type.DOUBLE
    _unit = "Counts"
    _description_template = "RMS value of the medians of input flat frames"

# -------------------------------
# Flat LM/N variants
# -------------------------------

class LmFlatMean(BandLmMixin, FlatMean):
    pass

class NFlatMean(BandNMixin, FlatMean):
    pass

class LmFlatRms(BandLmMixin, FlatRms):
    pass

class NFlatRms(BandNMixin, FlatRms):
    pass

class LmFlatMedianMean(BandLmMixin, FlatMedianMean):
    pass

class NFlatMedianMean(BandNMixin, FlatMedianMean):
    pass

class LmFlatMedianMin(BandLmMixin, FlatMedianMin):
    pass

class NFlatMedianMin(BandNMixin, FlatMedianMin):
    pass

class LmFlatMedianMax(BandLmMixin, FlatMedianMax):
    pass

class NFlatMedianMax(BandNMixin, FlatMedianMax):
    pass

class LmFlatMedianRms(BandLmMixin, FlatMedianRms):
    pass

class NFlatMedianRms(BandNMixin, FlatMedianRms):
    pass
