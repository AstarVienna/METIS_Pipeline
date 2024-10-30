AUTOMAKE_OPTIONS = foreign

#Put here all the workflows and associate python files here
WORKFLOWFILES = __init__.py metis_lm_img_wkf.py

#Put here all the recipe parameters associated with the workflow
PARAMETERFILES = metis_parameters.yaml

#This installs the workflow and the OCA rules in reflexaux directory
workflow_DATA = $(WORKFLOWFILES) ${PARAMETERFILES}

EXTRA_DIST = $(WORKFLOWFILES) ${PARAMETERFILES}
