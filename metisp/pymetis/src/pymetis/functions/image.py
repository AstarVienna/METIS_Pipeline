from typing import Optional

import cpl
from cpl.core import (Image as CplImage,
                      Type as CplType)


def zeros_like(image: CplImage, new_type: Optional[CplType] = None):
    """
    Create a new CPL Image with the same size as image, optionally with a new underlying type

    # FixMe [Martin] This should ideally be added to CPL:
    # FixMe zeros_like that preserves size but allow you to override the type.
    """
    temp = cpl.core.Image.zeros_like(image)
    if new_type is None:
        return temp
    else:
        return temp.cast(new_type)
