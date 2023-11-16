from edps import classification_rule

from . import aspresso_keywords as kwd
from . import aspresso_rules as rules

# Dictionaries containing the values of header keywords that define calibrations and science data
aspresso = {kwd.instrume: "ESPRESSO"}
calib_keywords = {**aspresso, kwd.dpr_catg: "CALIB"}
science_keywords = {**aspresso, kwd.dpr_catg: "SCIENCE"}

# Raw types
off_raw_class = classification_rule("DETMON_LAMP_OFF", {**aspresso, kwd.dpr_type: "BIAS,DETCHAR"})
on_raw_class = classification_rule("DETMON_LAMP_ON", {**aspresso, kwd.dpr_type: "LED,DETCHAR"})
bias_class = classification_rule("BIAS", {**calib_keywords, kwd.dpr_type: "BIAS"})
dark_class = classification_rule("DARK", {**calib_keywords, kwd.dpr_type: "DARK"})
led_ff_class = classification_rule("LED_FF", {**calib_keywords, kwd.dpr_type: "LED"})
orderdef_a_class = classification_rule("ORDERDEF_A", {**calib_keywords, kwd.dpr_type: "ORDERDEF,LAMP,OFF"})
orderdef_b_class = classification_rule("ORDERDEF_B", {**calib_keywords, kwd.dpr_type: "ORDERDEF,OFF,LAMP"})
flat_a_class = classification_rule("FLAT_A", {**calib_keywords, kwd.dpr_type: "FLAT,LAMP,OFF"})
flat_b_class = classification_rule("FLAT_B", {**calib_keywords, kwd.dpr_type: "FLAT,OFF,LAMP"})
contam_thar_class = classification_rule("RAW_CONTAM_THAR", {**calib_keywords, kwd.dpr_type: "CONTAM,OFF,THAR"})
contam_lfc_class = classification_rule("RAW_CONTAM_LFC", {**calib_keywords, kwd.dpr_type: "CONTAM,OFF,LFC"})
contam_fp_class = classification_rule("RAW_CONTAM_FP", {**calib_keywords, kwd.dpr_type: "CONTAM,OFF,FP"})

fp_fp_class = classification_rule("FP_FP", rules.is_fp_fp)
thar_fp_class = classification_rule("THAR_FP", rules.is_thar_fp)
fp_thar_class = classification_rule("FP_THAR", rules.is_fp_thar)
lfc_fp_class = classification_rule("LFC_FP", rules.is_lfc_fp)
fp_lfc_class = classification_rule("FP_LFC", rules.is_fp_lfc)

eff_ab_class = classification_rule("EFF_AB", {**calib_keywords, kwd.dpr_type: "EFF,SKY,SKY"})
std_flux_class = classification_rule("FLUX", {**calib_keywords, kwd.dpr_type: "FLUX,STD,SKY"})
science_sky_class = classification_rule("OBJ_SKY", {**science_keywords, kwd.dpr_type: "OBJECT,SKY"})
science_fp_class = classification_rule("OBJ_FP", {**science_keywords, kwd.dpr_type: "OBJECT,FP"})
science_thar_class = classification_rule("OBJ_THAR", {**science_keywords, kwd.dpr_type: "OBJECT,THAR"})
radial_velocity_class = classification_rule("OBJ_FP", {**science_keywords, kwd.dpr_type: "RVSTD,FP"})

# Static calibrations
ref_line_table_a_class = classification_rule("REF_LINE_TABLE_A", {**aspresso, kwd.pro_catg: "REF_LINE_TABLE_A"})
ref_line_table_b_class = classification_rule("REF_LINE_TABLE_B", {**aspresso, kwd.pro_catg: "REF_LINE_TABLE_B"})
static_dll_matrix_a_class = classification_rule("STATIC_DLL_MATRIX_A",
                                                {**aspresso, kwd.pro_catg: "STATIC_DLL_MATRIX_A"})
static_dll_matrix_b_class = classification_rule("STATIC_DLL_MATRIX_B",
                                                {**aspresso, kwd.pro_catg: "STATIC_DLL_MATRIX_B"})
static_wave_matrix_a_class = classification_rule("STATIC_WAVE_MATRIX_A",
                                                 {**aspresso, kwd.pro_catg: "STATIC_WAVE_MATRIX_A"})
static_wave_matrix_b_class = classification_rule("STATIC_WAVE_MATRIX_B",
                                                 {**aspresso, kwd.pro_catg: "STATIC_WAVE_MATRIX_B"})
wave_line_table_a_class = classification_rule("WAVE_LINE_TABLE_A", {**aspresso, kwd.pro_catg: "WAVE_LINE_TABLE_A"})
wave_line_table_b_class = classification_rule("WAVE_LINE_TABLE_B", {**aspresso, kwd.pro_catg: "WAVE_LINE_TABLE_B"})
ccd_geom_class = classification_rule("CCD_GEOM", {**aspresso, kwd.pro_catg: "CCD_GEOM"})
std_table_class = classification_rule("STD_TABLE", {**aspresso, kwd.pro_catg: "STD_TABLE"})
mask_lut_class = classification_rule("MASK_LUT", {**aspresso, kwd.pro_catg: "MASK_LUT"})
ext_table_class = classification_rule("EXT_TABLE", {**aspresso, kwd.pro_catg: "EXT_TABLE"})
flux_template_class = classification_rule("FLUX_TEMPLATE", {**aspresso, kwd.pro_catg: "FLUX_TEMPLATE"})
inst_config_class = classification_rule("INST_CONFIG", {**aspresso, kwd.pro_catg: "INST_CONFIG"})
led_ff_gain_windows_class = classification_rule("LED_FF_GAIN_WINDOWS",
                                                {**aspresso, kwd.pro_catg: "LED_FF_GAIN_WINDOWS"})
mask_table_class = classification_rule("MASK_TABLE", {**aspresso, kwd.pro_catg: "MASK_TABLE"})
cosmic_rays_mask_class = classification_rule("CRH_MAP", {**aspresso, kwd.pro_catg: "CRH_MAP"})

# Master calibrations
MASTER_BIAS_RES = classification_rule("MASTER_BIAS_RES", {**aspresso, kwd.pro_catg: "MASTER_BIAS_RES"})
HOT_PIXEL_MASK = classification_rule("HOT_PIXEL_MASK", {**aspresso, kwd.pro_catg: "HOT_PIXEL_MASK"})
BAD_PIXEL_MASK = classification_rule("BAD_PIXEL_MASK", {**aspresso, kwd.pro_catg: "BAD_PIXEL_MASK"})
ORDER_TABLE_A = classification_rule("ORDER_TABLE_A", {**aspresso, kwd.pro_catg: "ORDER_TABLE_A"})
ORDER_TABLE_B = classification_rule("ORDER_TABLE_B", {**aspresso, kwd.pro_catg: "ORDER_TABLE_B"})
ORDER_PROFILE_A = classification_rule("ORDER_PROFILE_A", {**aspresso, kwd.pro_catg: "ORDER_PROFILE_A"})
ORDER_PROFILE_B = classification_rule("ORDER_PROFILE_B", {**aspresso, kwd.pro_catg: "ORDER_PROFILE_B"})
FSPECTRUM_A = classification_rule("FSPECTRUM_A", {**aspresso, kwd.pro_catg: "FSPECTRUM_A"})
FSPECTRUM_B = classification_rule("FSPECTRUM_B", {**aspresso, kwd.pro_catg: "FSPECTRUM_B"})
BLAZE_A = classification_rule("BLAZE_A", {**aspresso, kwd.pro_catg: "BLAZE_A"})
BLAZE_B = classification_rule("BLAZE_B", {**aspresso, kwd.pro_catg: "BLAZE_B"})
FP_SEARCHED_LINE_TABLE_FP_FP_A = classification_rule("FP_SEARCHED_LINE_TABLE_FP_FP_A", {**aspresso,
                                                                                        kwd.pro_catg: "FP_SEARCHED_LINE_TABLE_FP_FP_A"})
FP_SEARCHED_LINE_TABLE_FP_FP_B = classification_rule("FP_SEARCHED_LINE_TABLE_FP_FP_B", {**aspresso,
                                                                                        kwd.pro_catg: "FP_SEARCHED_LINE_TABLE_FP_FP_B"})
S2D_BLAZE_FP_FP_A = classification_rule("S2D_BLAZE_FP_FP_A", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_FP_A"})
S2D_BLAZE_FP_FP_B = classification_rule("S2D_BLAZE_FP_FP_B", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_FP_B"})
WAVE_MATRIX_THAR_FP_A = classification_rule("WAVE_MATRIX_THAR_FP_A",
                                            {**aspresso, kwd.pro_catg: "WAVE_MATRIX_THAR_FP_A"})
DLL_MATRIX_THAR_FP_A = classification_rule("DLL_MATRIX_THAR_FP_A", {**aspresso, kwd.pro_catg: "DLL_MATRIX_THAR_FP_A"})
WAVE_MATRIX_FP_THAR_B = classification_rule("WAVE_MATRIX_FP_THAR_B",
                                            {**aspresso, kwd.pro_catg: "WAVE_MATRIX_FP_THAR_B"})
DLL_MATRIX_FP_THAR_B = classification_rule("DLL_MATRIX_FP_THAR_B", {**aspresso, kwd.pro_catg: "DLL_MATRIX_FP_THAR_B"})
S2D_BLAZE_FP_LFC_A = classification_rule("S2D_BLAZE_FP_LFC_A", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_LFC_A"})
S2D_BLAZE_FP_LFC_B = classification_rule("S2D_BLAZE_FP_LFC_B", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_LFC_B"})
DLL_MATRIX_LFC_FP_A = classification_rule("DLL_MATRIX_LFC_FP_A", {**aspresso, kwd.pro_catg: "DLL_MATRIX_LFC_FP_A"})
DLL_MATRIX_LFC_FP_B = classification_rule("DLL_MATRIX_LFC_FP_B", {**aspresso, kwd.pro_catg: "DLL_MATRIX_LFC_FP_B"})
S2D_BLAZE_THAR_FP_A = classification_rule("S2D_BLAZE_THAR_FP_A", {**aspresso, kwd.pro_catg: "S2D_BLAZE_THAR_FP_A"})
S2D_BLAZE_THAR_FP_B = classification_rule("S2D_BLAZE_THAR_FP_B", {**aspresso, kwd.pro_catg: "S2D_BLAZE_THAR_FP_B"})
S2D_BLAZE_FP_THAR_A = classification_rule("S2D_BLAZE_FP_THAR_A", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_THAR_A"})
S2D_BLAZE_FP_THAR_B = classification_rule("S2D_BLAZE_FP_THAR_B", {**aspresso, kwd.pro_catg: "S2D_BLAZE_FP_THAR_B"})
S2D_BLAZE_LFC_FP_A = classification_rule("S2D_BLAZE_LFC_FP_A", {**aspresso, kwd.pro_catg: "S2D_BLAZE_LFC_FP_A"})
S2D_BLAZE_LFC_FP_B = classification_rule("S2D_BLAZE_LFC_FP_B", {**aspresso, kwd.pro_catg: "S2D_BLAZE_LFC_FP_B"})
WAVE_MATRIX_LFC_FP_A = classification_rule("WAVE_MATRIX_LFC_FP_A", {**aspresso, kwd.pro_catg: "WAVE_MATRIX_LFC_FP_A"})
WAVE_MATRIX_FP_LFC_B = classification_rule("WAVE_MATRIX_FP_LFC_B", {**aspresso, kwd.pro_catg: "WAVE_MATRIX_FP_LFC_B"})
MASTER_CONTAM_FP = classification_rule("CONTAM_FP", {**aspresso, kwd.pro_catg: "CONTAM_FP"})
DLL_MATRIX_FP_LFC_B = classification_rule("DLL_MATRIX_FP_LFC_B", {**aspresso, kwd.pro_catg: "DLL_MATRIX_FP_LFC_B"})
MASTER_CONTAM_THAR = classification_rule("CONTAM_THAR", {**aspresso, kwd.pro_catg: "CONTAM_THAR"})
REL_EFF_B = classification_rule("REL_EFF_B", {**aspresso, kwd.pro_catg: "REL_EFF_B"})
ABS_EFF_A = classification_rule("ABS_EFF_A", {**aspresso, kwd.pro_catg: "ABS_EFF_A"})
S2D_FP_FP_A = classification_rule("S2D_FP_FP_A", {**aspresso, kwd.pro_catg: "S2D_FP_FP_A"})
S2D_FP_FP_B = classification_rule("S2D_FP_FP_B", {**aspresso, kwd.pro_catg: "S2D_FP_FP_B"})
