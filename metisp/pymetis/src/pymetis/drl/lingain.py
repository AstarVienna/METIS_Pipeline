from cpl.core import Image as CplImage, ImageList as CplImageList, Msg
from hdrl.core import Image as HdrlImage, ImageList as HdrlImageList
import hdrl
import copy

from typing import Literal, Optional

def correct_nonlinearity_image_hdrl(image: HdrlImage, linearity_map: HdrlImageList) -> HdrlImage:
    """
    Correct a single raw image for nonlinearity

    Parameters
    ----------
    image : HdrlImage
        raw image to correct [ADU]
    linearity_map : HdrlImageList
        polynomial linearity coefficients and their errors

    Returns
    -------
    corrected_image : HdrlImage
        linearity corrected image [ADU]
    """
    
    # satlimit=20000.0 # grab this from headers or from user input. user input sounds better but should it be consistent across the full reduction chain?
    
    # any values that exceed the linearity cutoff value need to be truncated.
    # negative values we keep the same. they might have received a small correction ~1 outside of the fitted regime, but that is ok. at least we will not get a discontinuity in flux values.

    #thresh_cpl=raw_org_hdrl.image # grab the cpl image part of the raw hdrl
    #thresh_mask=cpl.core.Mask(2048,2048) # define empty mask
    #thresh_mask.threshold_image(thresh_cpl,lo_cut=-9999,hi_cut=satlimit,inval=0) # set the mask with the saturated values being "True"
    # TODO to combine with any previously defined mask there needs to be an | (OR) operation.
    #thresh_cpl.threshold(hi_cut=satlimit,assign_hi_cut=satlimit,lo_cut=-9999,assign_lo_cut=-9999) # threshold these values in the cpl image. keep negative values the same
    
    #raw_org_hdrl.insert_into(image=thresh_cpl,error=None,ypos=0,xpos=0) # put the cpl image back into the hdrl image
    #raw_org_hdrl.reject_from_mask(thresh_mask) # put the mask back into the hdrl image
    
    for n, i in enumerate(range(len(linearity_map)-1, -1, -1)): # i follows the index from last to first array index, while n follows the power from low to high (python polyfit puts the constant component last)
        print(n,i)
        if n == 0: # only use constant term for power n=0: a[i]*flux**0
            correction_hdrl = copy.deepcopy(linearity_map[i]) # a[i] for n=0
        else: # for powers n>0 add: a[i]*flux**n
                    
            image_pow = image.pow_scalar_create(exponent=(n,0)) # flux**n (without error on the exponent)
            
            image_pow.mul_image(linearity_map[i]) # a[i]*flux**n
            correction_hdrl.add_image(image_pow) # add a[i]*flux**n to previous sum

    corrected_image = image.div_image_create(correction_hdrl) # maybe the whole problem, including the linearity characterization needs to be inverted so we do not do divisions but multiplications in this step. To avoid divide by zero. But maybe this just moves the divide by zero to another part of the code.
            
    return corrected_image

def correct_gain(images: HdrlImageList, gain: (float,float)
                 ) -> HdrlImageList:
    """
    Correct raw image list for gain.

    Parameters
    ----------
    images : HdrlImageList
        List of linearized images [ADU] to correct for gain
    gain : tuple of floats
        Gain and error [e/ADU]

    Returns
    -------
    corrected_images : HdrlImageList
        List of gain-corrected images [e]
    """
    
    if isinstance(images, HdrlImageList):
        Msg.info("correct_gain",f"Correct linearized images for gain")
        images.mul_scalar(gain)
    else:
        raise ValueError(f"Unknown input type {type(images)}")
    return images

def correct_nonlinearity(images: HdrlImageList | HdrlImage,
        linearity_map: HdrlImageList,
    ) -> HdrlImageList:
    """
    Correct raw image list for non-linearity.

    Parameters
    ----------
    images : HdrlImageList | HdrlImage
        List of raw images or single raw image [ADU] to correct for nonlinearity
    linearity_map : HdrlImageList
        Map of polynomial linearity coefficients and their errors
    
    Returns
    -------
    corrected_images : HdrlImageList
        List of raw images [ADU], now corrected for non-linearity
    """
        
    if isinstance(images, HdrlImageList):
        Msg.info("correct_nonlinearity",f"Correcting raw images for non-linearity")
        for i, image in enumerate(images):
            images[i]=correct_nonlinearity_image_hdrl(image,linearity_map)
    elif isinstance(images, HdrlImage):
        Msg.info("correct_nonlinearity",f"Correcting raw image for non-linearity")
        images=correct_nonlinearity_image_hdrl(images,linearity_map)
    else:
        raise ValueError(f"Unknown input type {type(images)}")
    
        
    return images
