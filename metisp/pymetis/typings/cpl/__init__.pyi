from __future__ import annotations
from . import core
from . import dfs
from . import drs
from . import hdrl
from . import ui
__all__: list[str] = ['DESCRIPTION', 'core', 'dfs', 'drs', 'hdrl', 'ui']
DESCRIPTION: str = 'CPL = 7.4, CFITSIO = 4.6.2, WCSLIB = 8.2.2, FFTW (normal precision) = 3.3.9, FFTW (single precision) = 3.3.9, OPENMP = 201511'
__version__: str = '1.0.4.post4'
