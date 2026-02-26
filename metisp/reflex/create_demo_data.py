# -*- coding: utf-8 -*-
import sys
import numpy as np
from astropy.io import fits as pyfits
import contextlib
import os

"""
create random demo data
if called with no arguments it creates two extended source files with
science and calibration DPR keys and packs them into a tarball
"""


@contextlib.contextmanager
def chdir(dirname=None):
    curdir = os.getcwd()
    try:
        if dirname is not None:
          os.chdir(dirname)
        yield
    finally:
        os.chdir(curdir)


@contextlib.contextmanager
def tempdir():
    import tempfile, shutil
    tmpdir = tempfile.mkdtemp()
    yield tmpdir
    shutil.rmtree(tmpdir)


def make_extended(imsize, powerlaw=2.0, window=np.hanning):
    """ create extended source from power law
    window: 1d windowing function default np.hanning
    from https://github.com/keflavich/image_registration
    """
    if isinstance(imsize, int):
        imsize, imsize2 = imsize, imsize
    else:
        imsize, imsize2 = imsize

    yy,xx = np.indices((imsize2, imsize))
    xcen = imsize // 2 - (1-imsize % 2)
    ycen = imsize2 // 2 - (1-imsize2 % 2)
    yy -= ycen
    xx -= xcen
    rr = (xx**2 + yy**2) ** 0.5 

    with np.errstate(divide='ignore', invalid='ignore'):
        powermap = (np.random.randn(imsize2, imsize) * rr**(-powerlaw) +
            np.random.randn(imsize2, imsize) * rr**(-powerlaw) * 1j) 
    powermap[powermap != powermap] = 0 

    newmap = np.abs(np.fft.fftshift(np.fft.fft2(powermap)))
    if window:
        w2d = np.sqrt(np.outer(window(newmap.shape[0]),
                               window(newmap.shape[1])))
        newmap *= w2d 
    return newmap


def triple_to_card(header):
    """ convert a python triple quote string to a header card
        removes newlines, pads to 80 chars and pads it to fitsblock
        load with pyfits.Header.fromstring
    """
    header = ''.join(h.ljust(80, ' ') for h in header.splitlines())
    header += ' ' * ((2880 - (len(header) % 2880)) % 2880)
    return header


baseheader = triple_to_card("""\
SIMPLE  =                    T          / Standard FITS
ORIGIN  = 'ESO     '                    / European Southern Observatory         
DATE    = '2012-08-02T09:12:16.1813'    / Date the file was written             
TELESCOP= 'ESO-VLT-U3'                  / ESO Telescope Name                    
INSTRUME= 'VISIR   '                    / Instrument used.                      
OBJECT  = 'HD-224630'                   / Original target.                      
RA      =           359.865183          / 23:59:27.6 RA (J2000) pointing        
DEC     =            -29.48510          / -29:29:06.3 DEC (J2000) pointing      
EQUINOX =               2000.0          / Standard FK5                          
RADECSYS= 'FK5     '                    / Coordinate reference frame            
EXPTIME =            0.0080000          / Integration time                      
MJD-OBS =       56141.38312605          / 2012-08-02T09:11:42.0907              
DATE-OBS= '2012-08-02T09:11:42.0907'    / Observing date                        
UTC     =            33099.000          / 09:11:39.000 UTC at start             
LST     =             4531.265          / 01:15:31.265 LST at start             
CTYPE1  = 'RA---TAN'                    / Pixel coordinate system               
CTYPE2  = 'DEC--TAN'                    / Pixel coordinate system               
CRPIX1  =           128.000000          / Reference pixel in <axis direction>   
CRPIX2  =           128.000000          / Reference pixel in <axis direction>   
CRVAL1  =           359.865183          / Coordinate value of reference pixel   
CRVAL2  =           -29.485096          / Coordinate value of reference pixel   
CD1_1   = -1.258333333333333E-05 / Horizontal binning factor                    
CD2_1   =                 -0.0          / Coordinate translation matrix.        
CD1_2   =                 -0.0          / Coordinate translation matrix.        
CD2_2   = 1.258333333333333E-05 / Vertical binning factor                       
CUNIT1  = 'deg     '                    / Coordinate system units.              
CUNIT2  = 'deg     '                    / Coordinate system units.              
HIERARCH ESO DET DIT         =    0.0200000 / Integration Time
HIERARCH ESO INS MODE        =    'ins mode'
""")


def write_file(fn, obs=True, setup='setup1'):
    prim = pyfits.PrimaryHDU()
    prim.header = pyfits.Header.fromstring(baseheader)
    if obs:
        prim.header.set('HIERARCH ESO DPR CATG', 'SCIENCE')
        prim.header.set('HIERARCH ESO DPR TECH', 'IMAGE,CHOPNOD,JITTER')
        prim.header.set('HIERARCH ESO DPR TYPE', 'OBJECT')
    else:
        prim.header.set('HIERARCH ESO DPR CATG', 'CALIB')
        prim.header.set('HIERARCH ESO DPR TECH', 'IMAGE,CHOPNOD,JITTER')
        prim.header.set('HIERARCH ESO DPR TYPE', 'STD')
    prim.header.set('HIERARCH ESO METIS SETUP', setup)
    prim.data = make_extended((256, 256))
    prim.writeto(fn, checksum=True)

if __name__ == "__main__":
    if len(sys.argv) < 2:
        import tarfile
        import time
        tfn = "metis-demo-reflex-0.6.tar.gz"
        prefix = 'metis/metis-demo-reflex-0.6'
        # I ❤ context managers
        with tarfile.open(tfn, "w:gz") as tar, tempdir() as tmpdir, \
             chdir(tmpdir):
            write_file('obsdata_setup1.fits', obs=True, setup='setup1')
            write_file('obsdata_setup2.fits', obs=True, setup='setup2')
            write_file('caldata.fits', obs=False)
            with open('README', 'w') as r:
                r.write('Created by metisp create_demo_data.py at %s\n'
                        % time.ctime())
            tar.add(r.name, arcname=os.path.join(prefix, r.name))
            tar.add('obsdata_setup1.fits', arcname=prefix + '/obsdata_setup1.fits')
            tar.add('obsdata_setup2.fits', arcname=prefix + '/obsdata_setup2.fits')
            tar.add('caldata.fits', arcname=prefix + '/calib/caldata.fits')
        sys.exit(0)

    prim = pyfits.PrimaryHDU()
    prim.header = pyfits.Header.fromstring(baseheader)
    if len(sys.argv) == 3:
        write_file(sys.argv[1], obs=False)
    else:
        write_file(sys.argv[1], obs=True)

