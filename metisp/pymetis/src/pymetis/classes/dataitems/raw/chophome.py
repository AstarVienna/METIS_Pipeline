from pymetis.classes.dataitems.raw import Raw


class LmChophomeRaw(Raw):
    _name: str = r'LM_CHOPHOME_RAW'
    _oca_keywords: set[str] = {'DPR.CATG', 'DPR.TECH', 'DPR.TYPE',
                               'INS.OPTI3.NAME', 'INS.OPTI9.NAME', 'INS.OPTI10.NAME', 'DRS.FILTER'}