#!/bin/bash
for d in imgN ifu lssLM lssN Calib hciAppLM hciRavcIfu hciRavcLM; do
    count=$(ls /mnt/d/simData_202602/output/${d}/*.fits 2>/dev/null | wc -l)
    echo "${d}: ${count} files"
done
