import itertools

from cpl.core import Msg

from .dataitem import *
from .raw import *
from .background import *
from .distortion import *
from .ifu import *
from .linearity import *
from .rsrf import *


__all__ = [
    'DataItem', 'ImageDataItem', 'TableDataItem',
    'Raw', 'ImageRaw', 'LmImageStdRaw', 'LmImageSciRaw', 'NImageStdRaw', 'NImageSciRaw',
    'IfuSciRaw'
    'parametrize',
]

from ..mixins import Detector2rgMixin, DetectorGeoMixin, DetectorIfuMixin, BandSpecificMixin, BandLmMixin, BandNMixin, \
    BandIfuMixin, TargetStdMixin, TargetSciMixin, TargetSkyMixin


def dict_product(**kwargs):
    """
    Generate a Cartesian product of dicts:
    {'a': 1, 'b': 2} × {'c': 3, 'd': 4}
    yields
    {'a': 1, 'c': 3}, {'a': 1, 'd': 4}, {'b': 2, 'c': 3}, {'b': 2, 'd': 4}
    """
    keys = kwargs.keys()
    for instance in itertools.product(*kwargs.values()):
        yield dict(zip(keys, instance))


def parametrize(template: str, **params: list[str]):
    """
    Parametrize a `DataItem` class by prepending appropriate Mixins.

    template: str
        String template for forming the new class name (not evaluated, since 3.14 it should be a t-string
    params:
        List of possible values for each of the template fragments.
    """
    class_map = {
        'band': {
            'LM': BandLmMixin,
            'N': BandNMixin,
            'IFU': BandIfuMixin,
        },
        'target': {
            'STD': TargetStdMixin,
            'SCI': TargetSciMixin,
            'SKY': TargetSkyMixin,
        },
        'detector': {
            '2RG': Detector2rgMixin,
            'GEO': DetectorGeoMixin,
            'IFU': DetectorIfuMixin,
        },
    }

    def decorator(cls: type[DataItem]):
        assert issubclass(cls, DataItem), \
            f"Class {cls.__qualname__} is not a subclass of `DataItem`"

        for prod in dict_product(**params):
            bases = []
            typename = template.format(**{key: value.capitalize() for key, value in prod.items()})

            for param, name in prod.items():
                try:
                    if class_map[param]:
                        try:
                            bases += [class_map[param][name]]
                        except KeyError as e:
                            Msg.error(cls.__qualname__, f"Unknown {param} value {name}")
                except KeyError as e:
                    Msg.error(cls.__qualname__, f"Cannot parametrize DataItem by {param}: {e}")

            print(typename, tuple(bases + [cls]), {})
            print(type(typename, tuple(bases + [cls]), {}))

    return decorator


#@parametrize('{band}{target}Background', band=['LM', 'N'], target=['STD', 'SCI'])
#class Background(BandSpecificMixin, TargetSpecificMixin, DataItem):
#    _group = cpl.ui.Frame.FrameGroup.CALIB
#    _oca_keywords = {'PRO.CATG', 'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}  # maybe
#
#    @classmethod
#    def name(cls):
#        return rf"{cls.band()}_{cls.target()}_BKG_SUBTRACTED"
#
#    @classmethod
#    def title(cls):
#        return rf"{cls.band():s} {cls.get_target_string()} background-subtracted"
#
#    @classmethod
#    def description(cls):
#        return rf"Thermal background subtracted images of {cls.get_target_string():s} {cls.band():s} exposures."
#
#