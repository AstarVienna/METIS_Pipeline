#!/bin/sh

if [ $# -ne 1 ]; then
    echo "Usage: $0 <rbs_rb_working_directory>" >&2
    exit 1
fi

rbs_rb_working_directory=$1
#product_directory=$2
#product_name_prefix=$3
#olas_directory=$4

if [ ! -d ${rbs_rb_working_directory} ]; then
    echo "ERROR: $0: rbs_rb_working_directory=${rbs_rb_working_directory} does not exist or is no directory" >&2
    exit 1
fi
#
#if [ ! -d ${product_directory} ]; then
#    echo "ERROR: $0: product_directory=${product_directory} does not exist or is no directory" >&2
#    exit 1
#fi



result=0

####################
# write QC logfile #
####################

instrument=METIS
vlt_dictionaries=PRO,ASM,DPR,GEN,IRD,METIS_OS,METIS_CFG,METIS_ICS,OBS,PRIMARY-FITS,TCS,TPL
paf_files=`find ${rbs_rb_working_directory} -name "qc*.paf" | xargs -n 1 basename | sort`

for file in ${paf_files}; do
  paf_file=`find ${rbs_rb_working_directory} -name "${file}"`

  ( cd `dirname ${paf_file}`; \
    QC1LogWriter -s ${instrument} \
                 -I ${INS_ROOT}/SYSTEM/Dictionary \
                 -D ${vlt_dictionaries} \
                 -p `basename ${paf_file}` \
  ) >&2
  
  if [ $? -ne 0 ]; then
    echo "ERROR: $0: QC1LogWriter failed on ${paf_file}" >&2
    result=1
    break
  fi
  
done


exit ${result}
