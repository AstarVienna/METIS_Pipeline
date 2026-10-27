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

from dataclasses import dataclass

from ..dataitems import DataItem
from ..inputs import PipelineInput


@dataclass(frozen=True)
class Step:
    """
    One box of a recipe's DRLD flowchart: what the step is called, which inputs it consumes
    and which products it produces, both as classes. Declared in the `RecipeImpl` body after
    its `InputSet` and `ProductSet`, so a step refers to them directly::

        _steps = (
            Step('Apply persistence correction', inputs=(InputSet.PersistenceMapInput,)),
            Step('Per pixel median / mean filtering', products=(ProductSet.MasterDark,)),
        )

    An input is identified by its class (or any class it derives from, so an input declared
    in a prefab's InputSet can be named by the shared class); a product by its data item (or a
    template it is specialized from). `RecipeImpl.input_name` and `product_name` resolve them
    to the attribute names and refuse a reference that matches none or several.
    Inputs no step names hang above the first step, products no step names below the last.
    """
    label: str
    inputs: tuple[type[PipelineInput], ...] = ()
    products: tuple[type[DataItem], ...] = ()
