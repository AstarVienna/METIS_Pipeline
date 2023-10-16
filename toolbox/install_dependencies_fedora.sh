dnf install -y dnf-plugins-core

dnf config-manager --add-repo=https://ftp.eso.org/pub/dfs/pipelines/repositories/stable/fedora/esorepo.repo

# libcurl-devel must go before libcurl, because otherwise dnf might install
# the 32-bit version of libcurl, which will not work with the ESO m4 macros.
dnf install -y \
    wget gcc gcc-g++ automake autogen libtool gsl gsl-devel fftw fftw-devel \
    curl bzip2 less svn git which dnf-plugins-core cppcheck lcov valgrind \
    zlib zlib-devel \
    erfa erfa-devel \
    libcurl-devel libcurl \
    tmux ripgrep file \
    cfitsio cfitsio-devel \
    wcslib wcslib-devel \
    cpl cpl-devel \
    java \
    python3-astropy python3-matplotlib python3-numpy \
    esorex esoreflex
