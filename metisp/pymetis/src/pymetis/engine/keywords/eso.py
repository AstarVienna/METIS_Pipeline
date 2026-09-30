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

The ESO-generic FITS keywords the engine itself refers to: the data-product classification,
the detector integration, the observation and template bookkeeping. Nothing here is specific
to METIS; the instrument vocabulary re-exports these next to its own keywords, and engine code
imports only this module.
"""
from .keyword import Keyword

# --- classification: what EDPS and OCA rules read on raws, what the pipeline writes on products ---
DPR_CATG = Keyword('DPR.CATG', str, description='Data product category',
                   range='SCIENCE, CALIB, ACQUISITION, TECHNICAL', comment='Required for the pipeline data association (OCA rules)')
DPR_TECH = Keyword('DPR.TECH', str, description='Data product technique',
                   range='IMAGE,LM; IMAGE,N; LSS,LM; LSS,N; IFU; PUP,LM; PUP,N', comment='Required for the pipeline data association (OCA rules)')
DPR_TYPE = Keyword('DPR.TYPE', str, description='Data product type',
                   range='OBJECT, SKY, STD, DARK, DETLIN, FLAT,LAMP, FLAT,TWILIGHT, WAVE, DISTORTION, RSRF, PSF,OFFAXIS, ...',
                   comment='Required for the pipeline data association (OCA rules)')
PRO_CATG = Keyword('PRO.CATG', str, description='Product category', comment='The tag of the data item, as the DRLD names it')
PRO_TECH = Keyword('PRO.TECH', str, description='Product technique')
DO_CATG = Keyword('DO.CATG', str, description='Data organiser category',
                  comment='Transient: the tag a frame carries in a set of frames; not saved to the product')

# --- detector ---
DET_ID = Keyword('DET.ID', str, description='Detector identifier', range='2RG, GEO, IFU',
                 comment='2RG: Hawaii-2RG of the LM imager and LSS; GEO: GeoSnap of the N imager and LSS; IFU: the four IFU detectors')
DET_DIT = Keyword('DET.DIT', float, unit='s', default=1.0, range=(0, 3600), description='Detector integration time')
DET_NDIT = Keyword('DET.NDIT', int, default=1, range=(1, 10), description='Number of detector integrations')
DET_READOUT = Keyword('DET.READOUT', str, description='Detector readout mode')

# --- observation, template, instrument ---
INSTRUME = Keyword('INSTRUME', str, default='METIS', description='Instrument name')
MJD_OBS = Keyword('MJD-OBS', float, format='%.8f', unit='d', description='Modified Julian date of the start of the observation')
EXPTIME = Keyword('EXPTIME', float, unit='s', description='Total integration time')
ARCFILE = Keyword('ARCFILE', str, description='Archive file name')
TPL_START = Keyword('TPL.START', str, description='Start of the template, as a date string')
TPL_EXPNO = Keyword('TPL.EXPNO', int, description='Exposure number within the template')
TPL_NEXP = Keyword('TPL.NEXP', int, description='Number of exposures in the template')
OBS_TPLNO = Keyword('OBS.TPLNO', int, description='Template number within the observation block')
OBS_ID = Keyword('OBS.ID', int, description='Observation block identifier')
OBS_RA = Keyword('OBS.RA', float, unit='deg', format='%.6f', description='Right ascension of the target')
OBS_DEC = Keyword('OBS.DEC', float, unit='deg', format='%.6f', description='Declination of the target')
INS_MODE = Keyword('INS.MODE', str, description='Instrument mode', comment='The observing mode codes of the DRLD instrument-modes table')
OCS_PXSCALE = Keyword('OCS.PXSCALE', float, unit='arcsec/pix', format='%.6f', description='Pixel scale')
