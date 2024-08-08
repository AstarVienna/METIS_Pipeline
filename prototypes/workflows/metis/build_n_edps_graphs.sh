# Script for creating METIS LSS LM/N-band workflow plots from EDPS

echo "Building graph for METIS LSS N-band workflow "
edps -shutdown  # EDPS server needs to be restarted after changes in the workflows

# (Useless) first run of edps because of the following error message:
#     "Error: <stdin>: syntax error in line 1 near 'Local'"
# However this disappears after the second call wondrously :-)
edps -w metis.metis_n_lss_wkf -g2 | dot -T png -Grankdir=TB > metis_n_lss_wkf_g2_vert.png

# LSS N-band plots in vertical layout
edps -w metis.metis_n_lss_wkf -g2 > tmp.dot
awk '{gsub("Workflow METIS","METIS EDPS LSS N-Band Workflow",$0); print $0}' tmp.dot > tmp1.dot
dot -T png -Grankdir=TB tmp1.dot > metis_n_lss_wkf_g2_vert.png
rm tmp.dot tmp1.dot

edps -w metis.metis_n_lss_wkf -g > tmp.dot
awk '{gsub("Workflow METIS","METIS EDPS LSS N-Band Workflow",$0); print $0}' tmp.dot > tmp1.dot
dot -T png -Grankdir=LR tmp1.dot > metis_n_lss_wkf_g1_vert.png
rm tmp.dot tmp1.dot
