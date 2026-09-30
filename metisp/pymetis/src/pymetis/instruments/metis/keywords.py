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

The FITS keywords of the METIS interface: the vocabulary every `_oca_keywords`,
`_matched_keywords` and header read draws from, and what the DRLD's FITS-keyword appendix is
generated from. One declaration per keyword, with what its card says (type, unit, format,
default, range, description), transcribed from the DRLD (App02_FITS_keywords.tex) with the
defects noted inline. The ESO-generic keywords come from `pymetis.engine.keywords.eso`.

Every keyword here is used somewhere (an item's OCA keywords, a recipe's matched keywords, an
alias, a header read, the engine, a workflow) or listed in `DECLARED_ONLY`: the DRLD promises
them on raws or products but no code touches them yet. The tests check both directions.
"""
from pymetis.engine.keywords import Keyword, Alias
from pymetis.engine.keywords.eso import (      # noqa: F401  (re-exported: one namespace for recipe authors)
    DPR_CATG, DPR_TECH, DPR_TYPE, PRO_CATG, PRO_TECH, DO_CATG,
    DET_ID, DET_DIT, DET_NDIT, DET_READOUT,
    INSTRUME, MJD_OBS, EXPTIME, ARCFILE, TPL_START, TPL_EXPNO, TPL_NEXP, OBS_TPLNO, OBS_ID, OBS_RA, OBS_DEC,
    INS_MODE, OCS_PXSCALE,
)

# --- the instrument: optical elements (INS.OPTIn is the ICS naming of the wheels and mechanisms) ---
INS_OPTI1_NAME = Keyword('INS.OPTI1.NAME', str, description='FO-PP1 mask wheel', comment='Coronagraphic pupil-plane mask of the fore-optics')
INS_OPTI3_NAME = Keyword('INS.OPTI3.NAME', str, description='LSS slit name', range='A_19 B_29 C_38 D_57 E_114',
                         comment='Name of the LSS slit')
INS_OPTI5_NAME = Keyword('INS.OPTI5.NAME', str, description='LMS PP1 mask wheel')
INS_OPTI6_NAME = Keyword('INS.OPTI6.NAME', str, description='LMS spectral IFU mechanism')
INS_OPTI9_NAME = Keyword('INS.OPTI9.NAME', str, description='LM-LSS mask / grism name', range='open GRISM-L GRISM-M')
INS_OPTI10_NAME = Keyword('INS.OPTI10.NAME', str, description='LM filter name', comment='LM imager and LM-LSS filter wheel')
INS_OPTI11_NAME = Keyword('INS.OPTI11.NAME', str, description='LM neutral-density filter name', range='OPEN ND1 ND2 ND3 ND4 ND5')
INS_OPTI12_NAME = Keyword('INS.OPTI12.NAME', str, description='N-LSS mask / grism name', range='open GRISM-N')
INS_OPTI13_NAME = Keyword('INS.OPTI13.NAME', str, description='N filter name', range='full_N', comment='N imager and N-LSS filter wheel')
INS_OPTI14_NAME = Keyword('INS.OPTI14.NAME', str, description='N neutral-density filter name', range='OPEN ND1 ND2 ND3 ND4')
INS_OPTI15_NAME = Keyword('INS.OPTI15.NAME', str, description='IMG-LM pupil imaging lens')
INS_OPTI16_NAME = Keyword('INS.OPTI16.NAME', str, description='IMG-N pupil imaging lens')
INS_OPTI19_NAME = Keyword('INS.OPTI19.NAME', str, description='WCU black-body exit aperture masks')
INS_OPTI20_NAME = Keyword('INS.OPTI20.NAME', str, description='WCU FP2.1 mask wheel', range='LM-pinhole')
# Read by metis_ifu_distortion (the named position 'open' is the open mask); no App02 card.
INS_OPTI20_POSNAME = Keyword('INS.OPTI20.POSNAME', str, description='Named position of the WCU FP2.1 mask wheel', range='open, LM-pinhole, ...')
INS_DROT = Keyword('INS.DROT', float, unit='degree', default=0.0, description='Derotator angle')
INS_READMODE = Keyword('INS.READMODE', str, default='CDS', range='CDS TLI RRR', description='Readout mode of the detector')
# OCA keyword of the LSS response items; no App02 card (mentioned starred only).
INS_SPEC_SETUP = Keyword('INS.SPEC.SETUP', str, description='Spectroscopic setup (slit, grism and filter combination)')
# Read by metis_ifu_wavecal; no App02 card.
INS_WLEN_CEN = Keyword('INS.WLEN.CEN', float, unit='um', format='%.4f', description='Central wavelength of the IFU grating setting')

# --- the warm calibration unit ---
# App02 has one card "SEQ WCU LASERn" (a prefix): the simulator writes the name, wavecal reads the wavelength.
SEQ_WCU_LASER_NAME = Keyword('SEQ.WCU.LASER{n}.NAME', str, index=range(1, 5), description='WCU laser source n',
                             labels=('seq.wcu.lasern',))
SEQ_WCU_LASER_WLEN = Keyword('SEQ.WCU.LASER{n}.WLEN', float, index=range(1, 5), unit='um', format='%.4f',
                             description='Wavelength of WCU laser source n')
INS_WCU_LASER_WLEN = Keyword('INS.WCU.LASER{n}.WLEN', float, index=range(1, 5), unit='um', format='%.4f',
                             description='Wavelength of WCU laser source n (instrument-side keyword)',
                             comment='metis_ifu_wavecal reads SEQ.WCU.LASERn.WLEN, then this; which one the ICS writes is TBC')

# --- product and WCS keywords the DRLD promises (declared only until a recipe writes them) ---
EXTNAME = Keyword('EXTNAME', str, description='Extension name')
SCIDATA = Keyword('SCIDATA', str, description='Extension name of the science data belonging to this error or quality extension')
ERRDATA = Keyword('ERRDATA', str, description='Extension name of the error data belonging to this science extension')
QUALDATA = Keyword('QUALDATA', str, description='Extension name of the quality data belonging to this science extension')
HDUCLAS1 = Keyword('HDUCLAS1', str, range='IMAGE', description='HDU class: the kind of HDU')
HDUCLAS2 = Keyword('HDUCLAS2', str, range='DATA, ERROR, QUALITY', description='HDU class: the role of the extension')
HDUCLAS3 = Keyword('HDUCLAS3', str, range='RMSE (error), FLAG32BIT (quality)', description='HDU class: the representation')
CDELT = Keyword('CDELT{n}', float, format='%.8f', index=range(1, 4), description='Increment of the coordinate specified by CTYPEn at the reference pixel')
CRPIX = Keyword('CRPIX{n}', float, format='%.1f', index=range(1, 4), description='Pixel position of the reference point in axis n')
CRVAL = Keyword('CRVAL{n}', float, format='%.5f', index=range(1, 4), description='Coordinate value as specified by CTYPEn at the reference pixel')
CTYPE = Keyword('CTYPE{n}', str, format='%s', index=range(1, 4), description='Name of the coordinate represented by axis n')
CUNIT = Keyword('CUNIT{n}', str, format='%s', index=range(1, 4), description='Unit of the coordinate of axis n')
CD_MATRIX = Keyword('CD{n}_{m}', float, format='%f', index=range(1, 4), description='Translation from array axis n to coordinate axis m',
                    labels=('cdn_ms',))
PV_MATRIX = Keyword('PV{n}_{m}', float, format='%f', index=range(1, 4), description='Projection parameter m for axis n',
                    labels=('pvn_ks',))
EXTINCT = Keyword('EXTINCT', float, unit='mag', default=0.0, description='Extinction of the observation')
GAIN = Keyword('GAIN', float, unit='e/adu', default=1.0, description='Gain of the detector')
ZEROPNT = Keyword('ZEROPNT', float, unit='mag', default=0.0, description='Zeropoint of the observation')
ICCOEF = Keyword('ICCOEF{n}', float, default=0.0, index=range(0, 100), description='Illumination correction coefficient n',
                 labels=('iccoefi',))

# --- aliases: what a recipe matches on, resolved to the wheels of the band in question (App02 cards) ---
DRS_FILTER = Alias('DRS.FILTER', resolves_to=(INS_OPTI10_NAME, INS_OPTI13_NAME),
                   description='Keyword alias for filter settings',
                   comment='Alias for the combination of keywords used to identify the filter settings: '
                           'the value of INS.OPTI10.NAME (LM) or INS.OPTI13.NAME (N)')
# App02 says OPTI11 or OPTI14; the chapter-5 alias table says OPTI13, which is the N filter wheel: taken as the typo.
DRS_NDFILTER = Alias('DRS.NDFILTER', resolves_to=(INS_OPTI11_NAME, INS_OPTI14_NAME),
                     description='Keyword alias for neutral-density filter settings',
                     comment='The value of INS.OPTI11.NAME (LM) or INS.OPTI14.NAME (N)')
DRS_SLIT = Alias('DRS.SLIT', resolves_to=(INS_OPTI3_NAME, INS_OPTI9_NAME, INS_OPTI12_NAME), combine='all',
                 description='Keyword alias for slit settings',
                 comment='The combination of the slit (INS.OPTI3.NAME) with the grism of the band (INS.OPTI9.NAME or INS.OPTI12.NAME)')
# App02 describes DRS.IFU as "ND filter settings": a copy-paste of the card above it.
DRS_IFU = Alias('DRS.IFU', resolves_to=(INS_OPTI6_NAME,),
                description='Keyword alias for IFU settings', comment='The value of INS.OPTI6.NAME')
DRS_MASK = Alias('DRS.MASK', resolves_to=(INS_OPTI1_NAME, INS_OPTI3_NAME, INS_OPTI5_NAME, INS_OPTI9_NAME, INS_OPTI12_NAME), combine='all',
                 description='Keyword alias for coronagraph settings',
                 comment='The combination of the pupil-plane and focal-plane masks in the beam')
DRS_PUPIL = Alias('DRS.PUPIL', resolves_to=(INS_OPTI15_NAME, INS_OPTI16_NAME),
                  description='Keyword alias for pupil settings', comment='The value of INS.OPTI15.NAME (LM) or INS.OPTI16.NAME (N)')

# The DRLD promises these on raws (the "mandatory" list) or products; no recipe or item refers to them yet.
DECLARED_ONLY: frozenset[Keyword] = frozenset({
    DET_READOUT, INS_DROT, INS_READMODE, PRO_TECH, DO_CATG, EXPTIME,
    TPL_EXPNO, OBS_TPLNO, OBS_RA, OBS_DEC, OCS_PXSCALE,
    EXTNAME, SCIDATA, ERRDATA, QUALDATA, HDUCLAS1, HDUCLAS2, HDUCLAS3,
    CDELT, CRPIX, CRVAL, CTYPE, CUNIT, CD_MATRIX, PV_MATRIX, EXTINCT, GAIN, ZEROPNT, ICCOEF,
    SEQ_WCU_LASER_NAME,
    DRS_NDFILTER,                                   # the DRLD defines the alias; no recipe matches on it
})
