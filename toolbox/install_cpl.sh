#!/bin/bash


wget https://ftp.eso.org/pub/dfs/pipelines/libraries/cpl/cpl-7.4.tar.gz
#wget https://ftp.eso.org/pub/dfs/pipelines/libraries/fftw/fftw-3.3.10.tar.gz
#wget https://ftp.eso.org/pub/dfs/pipelines/libraries/cfitsio/cfitsio-4.6.3.tar.gz
#wget https://ftp.eso.org/pub/dfs/pipelines/libraries/wcslib/wcslib-8.5.tar.bz2
#wget https://ftp.eso.org/pub/dfs/pipelines/libraries/cext/cext-1.3.2.tar.gz
 

#tar -xvf fftw-3.3.10.tar.gz
#tar -xvf cfitsio-4.6.3.tar.gz
#tar -xvf wcslib-8.5.tar.bz2
#tar -xvf cext-1.3.2.tar.gz
tar -xvf cpl-7.4.tar.gz

cwd=$(pwd)


#export CEXT_INCLUDE_DIR=$cwd/cpl/include
export HDRL_INCLUDE_DIR=$cwd/cpl/include

mkdir cpl

#cd cfitsio-4.6.3
#./configure --enable-reentrant --prefix=$cwd/cpl
#make
#make install
#cd ..

export LD_LIBRARY_PATH=$cwd/cpl/lib:$LD_LIBRARY_PATH

#cd cext-1.3.2
#./configure --prefix=$cwd/cpl
#make
#make install

#cd ..
cd cpl-7.4
./configure --prefix=$cwd/cpl
make
make install

cd ..
export HDRLDIR=$cwd/cpl
export CPLDIR=$cwd/cpl

wget https://ftp.eso.org/pub/dfs/pipelines/libraries/hdrl/hdrl-1.6.0.tar.gz
tar -xvf hdrl-1.6.0.tar.gz
cd hdrl-1.6.0
./configure --prefix=$cwd/cpl --enable-standalone --with-cpl=$CPLDIR --with-hdrl=$cwd/cpl
make
make install

#sudo ldconfig
