echo "Copying METIS EDPS files"
rsync -av /daten/ELT/METIS/devel/edps/workflows/metis/*.py .
rsync -av /daten/ELT/METIS/devel/edps/workflows/metis/*.sh .
rsync -av /daten/ELT/METIS/devel/edps/workflows/metis/*.png .
rm edps_activate.sh

git add *.py *.sh README.md *.png
git commit -m "Minor updates"
git push
