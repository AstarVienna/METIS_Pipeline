#!/bin/bash
echo "=== EDPS task directories ==="
ls -d /tmp/EDPS_data/METIS/*/ 2>/dev/null | while read d; do
    task=$(basename "$d")
    echo "$task"
done
