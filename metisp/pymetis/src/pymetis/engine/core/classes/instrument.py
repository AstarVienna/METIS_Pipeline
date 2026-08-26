import enum


class InstrumentDescription:
    class MaskFlags(enum.IntFlag):
        """
        This class provides meanings for the mask bits.

        Flags are stored in a `DataQuality` layer, whose backing store is a
        signed 32-bit integer, so bit 31 is the sign bit and must stay unused.
        Every flag must set exactly one of bits 0-`MAX_FLAG_BIT`, and no two
        flags may share a bit; both are enforced for all subclasses at
        definition time. Composite selections are built at the call site
        (``HOT | COLD``), never declared as members.
        """

        # FixMe: This should be also stored in the FITS file [Claude]
        # Document the bit→reason mapping in the FITS header.
        # Right now the meanings live only in Metis.MaskFlags in code.
        # Anyone reading the file with astropy sees an opaque integer plane.
        # Consider writing the flag definitions into the DQ HDU header
        # (e.g. HIERARCH ESO QC DQ BIT0 = 'BAD' cards, or a convention comment) so the file is self-describing.

        # Plain values, not enum members: Python forbids subclassing an enum
        # that has members, so the base class must stay member-free for the
        # instruments to be able to derive their flags from it at all.
        MAX_FLAG_BIT = enum.nonmember(30)
        ALL = enum.nonmember(0x7FFFFFFF)

        def __init_subclass__(cls, **kwargs):
            super().__init_subclass__(**kwargs)
            # `__members__` (unlike plain iteration) also yields aliases and
            # composite values, so nothing can slip past these checks.
            owners: dict[int, str] = {}
            for name, member in cls.__members__.items():
                if member.value.bit_count() != 1:
                    raise ValueError(
                        f"{cls.__qualname__}.{name} = 0x{member.value:08x} "
                        f"must set exactly one bit; combine flags at the call site "
                        f"instead of declaring composite members."
                    )
                if member.value.bit_length() > cls.MAX_FLAG_BIT + 1:
                    raise ValueError(
                        f"{cls.__qualname__}.{name} = 0x{member.value:08x} "
                        f"uses a bit above {cls.MAX_FLAG_BIT}; bit 31 is reserved "
                        f"as the sign bit of the signed int32 mask storage."
                    )
                if (owner := owners.setdefault(member.value, name)) != name:
                    raise ValueError(
                        f"{cls.__qualname__}.{name} shares bit 0x{member.value:08x} "
                        f"with {cls.__qualname__}.{owner}; every flag must own its bit."
                    )

