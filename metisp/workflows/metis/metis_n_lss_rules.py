# METIS LSS N BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from . import metis_n_lss_keywords as kwd

# Check for instrument
def is_metis(f):
    return f[kwd.instrume] == "METIS"

# Check for LSS N band mode
def is_lss_n(f):
    return f[kwd.dpr_tech] == "LSS,N"

