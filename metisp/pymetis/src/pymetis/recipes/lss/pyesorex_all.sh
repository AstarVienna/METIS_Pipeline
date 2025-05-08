echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++  LM ADC SLITLOSS +++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_adc_slitloss $SOF_DIR/metis_LM_adc_slitloss.sof --log-level debug
mv -v pyesorex.log pyesorex_adc_slitloss.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM DET DARK +++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_det_dark $SOF_DIR/metis_det_dark.lm.sof --log-level debug
mv -v pyesorex.log pyesorex_dark.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM LSS RSRF +++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_rsrf $SOF_DIR/metis_LM_lss_rsrf.sof --log-level debug
mv -v pyesorex.log pyesorex_rsrf.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM LSS TRACE ++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_trace $SOF_DIR/metis_LM_lss_trace.sof --log-level debug
mv -v pyesorex.log pyesorex_trace.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM LSS WAVE +++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_wave $SOF_DIR/metis_LM_lss_wave.sof --log-level debug
mv -v pyesorex.log pyesorex_wave.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM LSS STD ++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_std $SOF_DIR/metis_LM_lss_std.sof --log-level debug
mv -v pyesorex.log pyesorex_std.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  LM LSS SCI ++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_sci $SOF_DIR/metis_LM_lss_sci.sof --log-level debug
mv -v pyesorex.log pyesorex_sci.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "++++++++++++++++++++  MF MODEL +++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_mf_model $SOF_DIR/metis_LM_lss_mf_model.sof --log-level debug
mv -v pyesorex.log pyesorex_mf_model.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "++++++++++++++++++  MF CALCTRANS +++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_mf_calctrans $SOF_DIR/metis_LM_lss_mf_calctrans.sof --log-level debug
mv -v pyesorex.log pyesorex_mf_model.log

echo " "
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
echo "+++++++++++++++++++  MF CORRECT ++++++++++++++++++++"
echo "++++++++++++++++++++++++++++++++++++++++++++++++++++"
pyesorex metis_lm_lss_mf_correct $SOF_DIR/metis_LM_lss_mf_correct.sof --log-level debug
mv -v pyesorex.log pyesorex_mf_correct.log
