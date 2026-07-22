import enum

from pymetis.engine.core.classes.mask import Mask


class InstrumentDescription:
    class MaskFlags(enum.IntFlag):
        """
        This class provides meanings for the mask bits.

        Flags are stored in a `Mask`, whose backing store is a signed 32-bit
        integer, so bit 31 is the sign bit and must stay unused. Every flag
        must therefore fit within bits 0-`Mask.MAX_FLAG_BIT`; this is enforced
        for all subclasses at definition time.
        """

        # FixMe: This should be also stored in the FITS file [Claude]
        # Document the bit→reason mapping in the FITS header.
        # Right now the meanings live only in Metis.MaskFlags in code.
        # Anyone reading the file with astropy sees an opaque integer plane.
        # Consider writing the flag definitions into the DQ HDU header
        # (e.g. HIERARCH ESO QC DQ BIT0 = 'BAD' cards, or a convention comment) so the file is self-describing.

        def __init_subclass__(cls, **kwargs):
            super().__init_subclass__(**kwargs)
            for member in cls:
                if member.value.bit_length() > Mask.MAX_FLAG_BIT + 1:
                    raise ValueError(
                        f"{cls.__qualname__}.{member.name} = 0x{member.value:08x} "
                        f"uses a bit above {Mask.MAX_FLAG_BIT}; bit 31 is reserved "
                        f"as the sign bit of the signed int32 mask storage."
                    )

