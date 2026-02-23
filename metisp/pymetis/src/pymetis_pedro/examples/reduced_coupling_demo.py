from __future__ import annotations

from pymetis.core.common import Parametrizable, partial_format


class DemoItem(Parametrizable):
    _tag_parameters = {"detector": "2RG"}
    _name_template = "MASTER_DARK_{detector}"

    @classmethod
    def name(cls) -> str:
        return partial_format(cls._name_template, **cls.tag_parameters())


class DemoMixin(Parametrizable):
    _tag_parameters = {"band": "LM"}


class DemoDataItem(DemoMixin, DemoItem):
    pass


if __name__ == "__main__":
    print(DemoDataItem.name())
