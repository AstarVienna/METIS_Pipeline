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
import pprint

import yaml


class Header:
    def __init__(self,
                 name: str = None,
                 *,
                 _cls: str = None,
                 _context: str = None,
                 _type: type = None,
                 _value: str = None,
                 _unit: str = None,
                 _comment: str = None,
                 _default: bool = None,
                 _range: set = None,
                 _description: str = None):
        self.name = name
        self.cls = _cls
        self.description = _description

    def __str__(self):
        return self.name

    @staticmethod
    def from_dict(chunk):

        if 'range' in chunk:
            range = set(chunk['range']) if isinstance(chunk['range'], list) else (chunk['range']['min'], chunk['range']['max']),
        else:
            range = None

        return Header(
            chunk['name'],
            _cls=chunk['class'],
            _context=chunk['context'],
            _type=chunk['type'],
            _value=chunk['value'],
            _unit=chunk['unit'],
            _comment=chunk.get('comment', ""),
            _default=chunk.get('default', ""),
            _range=range,
            _description=chunk.get('description', "")
        )

    @staticmethod
    def load(filename) -> {str: 'Header'}:
        with open(filename) as f:
            data = yaml.safe_load(f)
            pprint.pprint(data)

            return {
                chunk: Header.from_dict(data[chunk])
                for chunk in data
            }

Header.load('pymetis/headers/headers.yaml')

HeaderInsOpti3Name = Header(name="INS.OPTI3.NAME", _description="LSS slit name")
HeaderInsOpti6Name = Header(name="INS.OPTI6.NAME", _description="LMS spectral IFU mechanism")
HeaderInsOpti9Name = Header(name="INS.OPTI9.NAME", _description="LM-LSS mask / grism name")
HeaderInsOpti10Name = Header(name="INS.OPTI10.NAME", _description="LM-LSS filter name")
HeaderInsOpti11Name = Header(name="INS.OPTI11.NAME", _description="ND filter name")
HeaderInsOpti12Name = Header(name="INS.OPTI12.NAME", _description="N-LSS mask / grism name")

HeaderProCatg = Header(name="PRO.CATG", _description="Processed data product category")
HeaderDrsIfu = Header(name="DRS.IFU", _description="Keyword alias for ND filter settings")
HeaderDrsPupil = Header(name="DRS.PUPIL", _description="Keyword alias for pupil settings")
HeaderDrsFilter = Header(name="DRS.FILTER", _description="Keyword alias for filter settings")

HeaderDetDit = Header(name="DET.DIT", _description="Detector integration time (average when NDIT > 1)")
HeaderDetNDit = Header(name="DET.NDIT", _description="Number of detector integrations")
