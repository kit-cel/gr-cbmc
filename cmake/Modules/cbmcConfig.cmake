INCLUDE(FindPkgConfig)
PKG_CHECK_MODULES(PC_CBMC cbmc)

FIND_PATH(
    CBMC_INCLUDE_DIRS
    NAMES cbmc/api.h
    HINTS $ENV{CBMC_DIR}/include
        ${PC_CBMC_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    CBMC_LIBRARIES
    NAMES gnuradio-cbmc
    HINTS $ENV{CBMC_DIR}/lib
        ${PC_CBMC_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CBMC DEFAULT_MSG CBMC_LIBRARIES CBMC_INCLUDE_DIRS)
MARK_AS_ADVANCED(CBMC_LIBRARIES CBMC_INCLUDE_DIRS)

