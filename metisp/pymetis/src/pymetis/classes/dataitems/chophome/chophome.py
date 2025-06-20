from pymetis.classes.dataitems.dataitem import DataItem
from pymetis.classes.mixins.band import BandLmMixin


class LmChophomeCombined(BandLmMixin, DataItem):
    _name = r'LM_CHOPHOME_COMBINED'
    _description = "Stacked LM band exposures."
    _oca_keywords = {'PRO.CATG', 'INS.OPTI20.NAME'}


class LmChophomeBackground(BandLmMixin, DataItem):
    _name = r'LM_CHOPHOME_BACKGROUND'
    _description = "Stacked WCU background images."
    _oca_keywords = {'PRO.CATG', 'INS.OPTI19.NAME', 'INS.OPTI20.NAME'}