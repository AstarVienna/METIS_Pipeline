# METIS LSS LM BAND EDPS workflow
#
# Auhor: W. Kausch / University of Innsbruck
#
# Version: see Changelog
#

from edps import JobParameters, get_parameter, List, ClassifiedFitsFile


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

def which_detector(files: List[ClassifiedFitsFile]):
    # Check whether the observation type is EXTENDED or POINT-LIKE
    ins_mode = files[0].get_keyword_value("ins.mode", None)
    if "LM" in ins_mode:
        detector = "LM"
    if "N" in ins_mode:
        detector = "N"
    if "IFU" in ins_mode:
        detector = "IFU"

    return detector

def is_LM(params: JobParameters) -> bool:
    return get_parameter(params, "which_detector") == "LM"
def is_N(params: JobParameters) -> bool:
    return get_parameter(params, "which_detector") == "N"
def is_IFU(params: JobParameters) -> bool:
    return get_parameter(params, "which_detector") == "IFU"