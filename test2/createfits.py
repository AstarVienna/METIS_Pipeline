from astropy.io import fits
import numpy

SIZE_X, SIZE_Y = 6, 4

hdulist = fits.HDUList([
    fits.PrimaryHDU(
        header=fits.Header({
            "INSTRUME": "METIS",
            "MJD-OBS": 50000.5,
            "HIERARCH ESO DPR CATG": "SCIENCE",
            "HIERARCH ESO DPR TECH": "IMAGE,N",
            "HIERARCH ESO DPR TYPE": "OBJECT",
        })
    ),
    fits.ImageHDU(
        data=numpy.random.rand(SIZE_X, SIZE_Y),
    )
])
hdulist.writeto("N_IMAGE_SCI_RAW_1.fits", overwrite=True)

hdulist = fits.HDUList([
    fits.PrimaryHDU(
        header=fits.Header({
            "INSTRUME": "METIS",
            "MJD-OBS": 50000.5,
            "HIERARCH ESO PRO CATG": "MASTER_DARK_GEO",
        })
    ),
    fits.ImageHDU(
        data=numpy.random.rand(SIZE_X, SIZE_Y),
    )
])
hdulist.writeto("MASTER_DARK_GEO_1.fits", overwrite=True)

hdulist = fits.HDUList([
    fits.PrimaryHDU(
        header=fits.Header({
            "INSTRUME": "METIS",
            "MJD-OBS": 50000.5,
            "HIERARCH ESO PRO CATG": "MASTER_FLAT_GEO",
        })
    ),
    fits.ImageHDU(
        data=numpy.random.rand(SIZE_X, SIZE_Y),
    )
])
hdulist.writeto("MASTER_FLAT_GEO_1.fits", overwrite=True)

with open("test.sof", 'w', encoding="utf-8") as f:
    f.write("""
N_IMAGE_SCI_RAW_1.fits  OBJECT
MASTER_DARK_GEO_1.fits  MASTER_BIAS
MASTER_FLAT_GEO_1.fits  MASTER_FLAT
""".lstrip())

