# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import JobParameters, get_parameter


########################################################################################################################
###         Functions that define conditions depending on the values of the the workflow parameters                  ###
########################################################################################################################

#def use_telluric_standard(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'false'


#def use_molecfit(params: JobParameters):
#    return get_parameter(params, 'molecfit') != 'false'


#def molecfit_on_standard(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'standard'


#def molecfit_on_science(params: JobParameters):
#    return get_parameter(params, 'molecfit') == 'science'

def on_science (params : JobParameters) -> bool:
    return get_parameter(params, "molecfit") == "science"

def on_standard (params: JobParameters) -> bool:
    return get_parameter(params, "molecfit") == "standard"