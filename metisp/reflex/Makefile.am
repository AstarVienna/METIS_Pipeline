
AUTOMAKE_OPTIONS = 1.8 foreign

#Put here all the workflows. They must end with extension .in. They should
#be like a normal workflow but using @prefix@ for paths 
#to the OCA rules (grep for OCA in the xml file) or Python scripts
#
#WARNING!!! Make sure that the last character of the following line is not 
#           a space.
WORKFLOWS = metis.xml

#Put here the OCA rules used by the workflow
OCAWKF =  metis_wkf.oca

#Put Python scripts used by the workflows
PYTHONWKF = interactive_window.py

#This installs the workflow and the OCA rules in reflexaux directory
wkfextra_DATA = $(WORKFLOWS) $(OCAWKF) $(PYTHONWKF)

#Pack in the distribution the original workflow
EXTRA_DIST = $(WORKFLOWS).in $(OCAWKF) $(PYTHONWKF)

#This also installs the workflow in the common reflex directory
wkfcopy_DATA = $(WORKFLOWS)

