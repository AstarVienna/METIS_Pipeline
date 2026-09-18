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

"""
The METIS data-item catalogue. Every class is reachable flat as
`pymetis.instruments.metis.dataitems.<Class>` -- a DRLD tag has exactly one class, so the
module a class lives in carries no information a user of the catalogue needs. Recipes and
inputs import the package (`from pymetis.instruments.metis import dataitems`) and bind
`Item = dataitems.LmSciBasicReduced`, never a class by name.
"""
import importlib
import pkgutil

from pymetis.engine.dataitems import DataItem

__all__ = []
for _module_info in pkgutil.walk_packages(__path__, __name__ + '.'):
    _module = importlib.import_module(_module_info.name)
    for _name, _obj in list(vars(_module).items()):
        if isinstance(_obj, type) and issubclass(_obj, DataItem) and _obj.__module__ == _module.__name__:
            if _name in globals() and globals()[_name] is not _obj:
                raise ImportError(f"two data item classes are called {_name}: {globals()[_name].__module__} and {_module.__name__}")
            globals()[_name] = _obj
            __all__.append(_name)
__all__.sort()
del _module_info, _module, _name, _obj
