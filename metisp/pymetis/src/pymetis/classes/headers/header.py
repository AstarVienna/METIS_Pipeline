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


class Header:
    def __init__(self,
                 name: str = None,
                 *,
                 description: str = None):
        self.name = name
        self.description = description

    @staticmethod
    def load(filename):
        return Header(name='a', description='b')


InsOpti3Name = Header(name="INS.OPTI3.NAME", description="LSS slit name")
InsOpti6Name = Header(name="INS.OPTI6.NAME", description="LMS spectral IFU mechanism")
InsOpti9Name = Header(name="INS.OPTI9.NAME", description="LM-LSS mask / grism name")
InsOpti10Name = Header(name="INS.OPTI10.NAME", description="LM-LSS filter name")
InsOpti11Name = Header(name="INS.OPTI11.NAME", description="ND filter name")
InsOpti12Name = Header(name="INS.OPTI12.NAME", description="N-LSS mask / grism name")

ProCatg = Header(name="PRO.CATG", description="Processed data product category")
DrsIfu = Header(name="DRS.IFU", description="Keyword alias for ND filter settings")