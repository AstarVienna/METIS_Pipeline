"""
HDRL - High Level Data Reduction Library

  Python bindings for the ESO High Level Data Reduction Library (HDRL).
  Provides algorithms for astronomical data reduction including airmass
  calculation, bad pixel detection, flat fielding, fringing correction,
  overscan correction, image resampling, and more.
  
"""
from __future__ import annotations
from . import core
from . import debug
from . import func
__all__: list[str] = ['core', 'debug', 'func']
