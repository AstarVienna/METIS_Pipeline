echo "Copying METIS EDPS files"
cp -v /daten/ELT/METIS/devel/edps/workflows/metis/*.py .
cp -v /daten/ELT/METIS/devel/edps/workflows/metis/*.sh .
cp -v /daten/ELT/METIS/devel/edps/workflows/metis/*.png .
rm edps_activate.sh

git add *.py *.sh README.md *.png
git commit -m "Updates (see Changelog)"
git push
