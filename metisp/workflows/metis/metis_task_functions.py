from edps import JobParameters, get_parameter, Job


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

def instrument_to_linlimit(job : Job):
    linlimit = f'{job.command}.linlimit'
    subinstrument = job.input_files[0].get_keyword_value("dpr.tech", None)
    if "LM" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 20000 # this will also need to be adapted based on readoutmode, as that will change the saturation limit and gain
    elif "N" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 12000
    elif "IFU" in subinstrument:
        job.parameters.recipe_parameters[linlimit] = 21000 # need to verify this value with LMS properties (saturation limit and gain)
    else: 
        print("do not recognize instrument, using default value")
    
