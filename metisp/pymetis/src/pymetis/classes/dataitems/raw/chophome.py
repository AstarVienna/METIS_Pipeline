from pymetis.classes.dataitems.raw import Raw


class LmChophomeRaw(Raw):
    _name = r'LM_CHOPHOME_RAW'
    _title = "LM chop-home raw"
    _oca_keywords: set[str] = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                               'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}