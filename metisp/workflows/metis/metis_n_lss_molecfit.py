from edps import subworkflow, task, List
from .metis_datasources import *

# This sub-workflow computes the atmospheric transmission by running molecfit on telluric standard star.
@subworkflow("telluric_on_standard", "")
def telluric_on_standard(reference, target):
    # Create an atmospheric model using the telluric standard star as reference spectrum
    model = (task('model_on_standard')
             .with_recipe('metis_mf_model')
             .build())

    # Computing the transmission curve using the information from the science target
    # and the atmospheric model computed om the telluric standard stare.
    transmission = (task('transmission_on_standard')
                    .with_recipe('metis_mf_calctrans')
                    .build())

    return transmission


# This sub-workflow computes the atmospheric transmission by running molecfit directly on science data.
@subworkflow("telluric_on_science", "")
def telluric_on_science(target):
    # This runs the extraction recipe metis_extrac_spec on each individual file separately and collects all the results.

    # ------------------------------------------------------------------------------------------------------------------

    # Create an atmospheric model using the selected science spectrum as reference.
    model = (task('model_on_science')
             .with_recipe('metis_mf_model')
             .build())

    # Computing the transmission curve using the information from the science target
    # and the atmospheric model computed om the telluric standard star.
    transmission = (task('transmission_on_science')
                    .with_recipe('metis_mf_calctrans')
                    .build())

    return transmission
