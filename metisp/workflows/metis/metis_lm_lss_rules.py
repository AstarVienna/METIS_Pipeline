# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from . import metis_lm_lss_keywords as kwd

# Check for instrument
def is_metis(f):
    return f[kwd.instrume] == "METIS"

# Check for LSS LM band mode
def is_lss_lm(f):
    return f[kwd.dpr_tech] == "LSS,LM"

