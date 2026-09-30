# HEADER KEYWORDS USED FOR CLASSIFICATION, GROUPING, AND ASSOCIATION.
instrume = "instrume"
pro_catg = "pro.catg"
dpr_type = "dpr.type"
dpr_catg = "dpr.catg"
dpr_tech = "dpr.tech"
tpl_nexp = "tpl.nexp"
obs_id = "obs.id"
tpl_start = "tpl.start"
date = "date"
filt_id = "filt.id"
airmass = "tel.airm.start"
targ_name = "obs.targ.name"
det_id = "det.id"
det_binx = "det.binx"
det_biny = "det.biny"
det_ndit = "det.ndit"
ins_mode = "ins.mode"
unique = "arcfile"
mjd_obs = "mjd-obs"
telescop = "telescop"
ocs_enabled_fe = "ocs.enabled.fe"
ins5_modsel_id = "ins5.modsel.id"

# DRS.FILTER is an alias of the DRLD, not a header card: the filter wheel of the band in question.
# EDPS cannot resolve an alias, so the task functions read these instead (kept equal to the pipeline's
# `keywords.DRS_FILTER.edps_alternatives` by a test in pymetis).
drs_filter_alternatives = ("ins.opti10.name", "ins.opti13.name")
