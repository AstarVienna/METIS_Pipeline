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

    def __str__(self):
        return self.name

    @staticmethod
    def load(filename):
        return Header(name='a', description='b')


HeaderInsOpti3Name = Header(name="INS.OPTI3.NAME", description="LSS slit name")
HeaderInsOpti6Name = Header(name="INS.OPTI6.NAME", description="LMS spectral IFU mechanism")
HeaderInsOpti9Name = Header(name="INS.OPTI9.NAME", description="LM-LSS mask / grism name")
HeaderInsOpti10Name = Header(name="INS.OPTI10.NAME", description="LM-LSS filter name")
HeaderInsOpti11Name = Header(name="INS.OPTI11.NAME", description="ND filter name")
HeaderInsOpti12Name = Header(name="INS.OPTI12.NAME", description="N-LSS mask / grism name")

HeaderProCatg = Header(name="PRO.CATG", description="Processed data product category")
HeaderDrsIfu = Header(name="DRS.IFU", description="Keyword alias for ND filter settings")
HeaderDrsPupil = Header(name="DRS.PUPIL", description="Keyword alias for pupil settings")
HeaderDrsFilter = Header(name="DRS.FILTER", description="Keyword alias for filter settings")

HeaderDetDit = Header(name="DET.DIT", description="Detector integration time (average when NDIT > 1)")
HeaderDetNDit = Header(name="DET.NDIT", description="Number of detector integrations")
