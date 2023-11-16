from edps import match

from . import aspresso_keywords as kwd


def is_aspresso(f):
    return f[kwd.instrume] == "ESPRESSO"


def is_calib(f):
    return is_aspresso(f) and f[kwd.dpr_catg] == "CALIB"


def is_science(f):
    return is_aspresso(f) and f[kwd.dpr_catg] == "SCIENCE"


def is_fp_fp(f):
    return is_calib(f) and f[kwd.dpr_type] == "WAVE,FP,FP" and f[kwd.tpl_nexp] == 1 and f[kwd.obs_id] != -1


def is_thar_fp(f):
    return is_calib(f) and f[kwd.dpr_type] == "WAVE,THAR,FP" and f[kwd.tpl_nexp] == 1


def is_fp_thar(f):
    return is_calib(f) and f[kwd.dpr_type] == "WAVE,FP,THAR" and f[kwd.tpl_nexp] == 1


def is_lfc_fp(f):
    return is_calib(f) and f[kwd.dpr_type] == "WAVE,LFC,FP" and f[kwd.tpl_nexp] == 1


def is_fp_lfc(f):
    return is_calib(f) and f[kwd.dpr_type] == "WAVE,FP,LFC" and f[kwd.tpl_nexp] == 1


# ASSOCIATION RULES f=file to associate (e.g. calibration) ref=trigger (e.g. science)
# All the associations but this one are specified in the workflow, using .with_match_keywords()

# This function forces unconditional association
def is_associated(ref, f):
    return True


def assoc_setup_and_telescope(ref, f):
    return match(ref, f, [kwd.det_binx, kwd.det_biny, kwd.ins_mode, kwd.instrume]) and \
           (match(ref, f, [kwd.ins5_modsel_id]) or match(ref, f, [kwd.ocs_enabled_fe]))
