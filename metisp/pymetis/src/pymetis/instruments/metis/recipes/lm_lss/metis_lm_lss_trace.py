"""
This file is part of the METIS Pipeline.
Copyright (C) 2025 European Southern Observatory

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

from pymetis.engine.recipes import Recipe
from pymetis.engine.core.parameter import (ParameterList, ParameterEnum, 
                                        ParameterRange, ParameterValue)

from pymetis.instruments.metis.mixins import BandLmMixin, Detector2rgMixin
from pymetis.instruments.metis.recipes.prefab.lss.trace import MetisLssTraceImpl


class MetisLmLssTraceImpl(BandLmMixin, Detector2rgMixin, MetisLssTraceImpl):
    class InputSet(MetisLssTraceImpl.InputSet):
        pass

    class ProductSet(MetisLssTraceImpl.ProductSet):
        pass

    class QcParameterSet(MetisLssTraceImpl.QcParameterSet):
        pass

class MetisLmLssTrace(Recipe):
    # The information about the recipe needs to be set. The base class
    # cpl.ui.PyRecipe provides the class variables to be set.
    # The recipe name must be unique, because it is this name which is
    # used to identify a particular recipe among all installed recipes.
    # The name of the python source file where this class is defined
    # is not at all used in this context.
    _name: str = "metis_lm_lss_trace"
    _version: str = "0.1"
    _author: str = "Wolfgang Kausch, A*"
    _email: str = "wolfgang.kausch@uibk.ac.at"
    _copyright: str = "GPL-3.0-or-later"
    _synopsis: str = "Detection of LM order location on the 2RG detector"

    _matched_keywords: frozenset[str] = frozenset({'DET.DIT', 'DET.NDIT', 'DRS.SLIT'})
    _algorithm = """Fancy algorithm description follows ***TBD*** """

    # ++++++++++++++++++ Define parameters ++++++++++++++++++
    parameters = ParameterList([
        ParameterRange(
            name=f"{_name}.trace.degree",
            context=_name,
            description="Polynomial degree of the fit to each slice",
            default=4,
            min=1, # TODO what are the correct ranges to use
            max=5,
        ),

        # TODO max_error

        ParameterValue(
            name=f"{_name}.trace.degree_before_merge",
            context=_name,
            description="Polynomial degree used while rating candidate merges, where "
                        "the fits are still poorly constrained. An integer, or 'best' "
                        "to choose one per cluster",
            default="best",
        ),

        # TODO bias scaling
        # TODO norm scaling
        # TODO regularisaton
        # TODO closing shape
        # TODO opening shape

        ParameterRange(
            name=f"{_name}.trace.auto_merge_threshold",
            context=_name,
            description="Overlap rating at or above which two clusters are merged "
                        "into one slice. 1 disables merging",
            default=0.9,
            min=0.0,
            max=1.0,
        ),

        ParameterRange(
            name=f"{_name}.trace.merge_min_threshold",
            context=_name,
            description="Overlap rating below which a pair of clusters is not "
                        "considered for merging",
            default=0.01,
            min=0.0,
            max=1.0,
        ),

        # TODO double check this is the same as split_sigma in config
        ParameterValue(
            name=f"{_name}.trace.sigma",
            context=_name,
            description="If positive, split clusters that deviate from the common "
                        "trace shape by more than this many standard deviations",
            default=0.0,
        ),

        ParameterValue(
            name=f"{_name}.trace.filter_x",
            context=_name,
            description="Smoothing width along the dispersion direction, applied "
                        "before thresholding. 0 disables it",
            default=0,
        ),

        ParameterValue(
            name=f"{_name}.trace.filter_y",
            context=_name,
            description="Smoothing width along the cross-dispersion direction, "
                        "used to estimate the local background",
            default=20,
        ),

        ParameterEnum(
            name=f"{_name}.trace.filter_type",
            context=_name,
            description="Smoothing kernel used to estimate the local background. "
                        "'whittaker' preserves edges best, 'boxcar' is cheapest",
            default="boxcar",
            alternatives=("boxcar", "gaussian", "whittaker"),
        ),

        ParameterValue(
            name=f"{_name}.trace.min_cluster",
            context=_name,
            description="Smallest acceptable cluster of illuminated pixels, in pixels",
            default=100,
        ),

        # TODO ive copied this from IFU, but im not sure if it is the same in LSS
        # the default seems to be None, which is what raised an alarm bell to me
        # Need to double check this before including
        # I also dont see this in the input in pyreduce
#        ParameterRange(
#            name=f"{_name}.trace.min_width",
#            context=_name,
#            description="Smallest acceptable cluster extent along the dispersion "
#                        "direction, as a fraction of the detector width. "
#                        "0 disables the check",
#            default=0.25,
#            min=0.0,
#            max=1.0,
#        ),

        ParameterValue(
            name=f"{_name}.trace.noise",
            context=_name,
            description="Absolute detection threshold above the local background",
            default=120.0,
        ),

        ParameterValue(
            name=f"{_name}.trace.noise_relative",
            context=_name,
            description="Detection threshold as a fraction of the local background. "
                        "If this and the absolute threshold are both 0, 0.1% is used",
            default=0.0,
        ),

        ParameterValue(
            name=f"{_name}.trace.border_width",
            context=_name,
            description="Number of pixels to ignore at each detector edge",
            default=6,
        ),

        # TODO manual
        # TODO plot
        # TODO plot title

    ])
    # Only dummy values for the time being!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    # ++++++++++++++++++ Finalisation ++++++++++++++++++
    Impl = MetisLmLssTraceImpl

