macro(eso_set_library_version current revision age)
    set(PACKAGE_LT_CURRENT ${current})
    set(PACKAGE_LT_REVISION ${revision})
    set(PACKAGE_LT_AGE ${age})
    set(PACKAGE_LIBRARY_VERSION ${current}.${revision}.${age})
endmacro()

# Package name and version
string(TOLOWER ${PROJECT_NAME} PACKAGE)

set(PACKAGE_MAJOR ${PROJECT_VERSION_MAJOR})
set(PACKAGE_MINOR ${PROJECT_VERSION_MINOR})
set(PACKAGE_PATCH ${PROJECT_VERSION_PATCH})

set(PACKAGE_NAME "${PACKAGE}")
set(PACKAGE_AUTHOR "European Southern Observatory")
set(PACKAGE_BUGREPORT "https://support.eso.org")

set(PACKAGE_VERSION "${PACKAGE_MAJOR}.${PACKAGE_MINOR}.${PACKAGE_PATCH}")

math(EXPR PACKAGE_BINARY_AGE "100 * ${PACKAGE_MINOR} + ${PACKAGE_PATCH}")
math(EXPR PACKAGE_BINARY_VERSION "10000 * ${PACKAGE_MAJOR} + ${PACKAGE_BINARY_AGE}")

# Library ABI version, using GNU libtool terminology
set(PACKAGE_LT_CURRENT ${PROJECT_VERSION_MAJOR})
set(PACKAGE_LT_REVISION ${PROJECT_VERSION_MINOR})
set(PACKAGE_LT_AGE ${PROJECT_VERSION_PATCH})
set(PACKAGE_LIBRARY_VERSION ${current}.${revision}.${age})
