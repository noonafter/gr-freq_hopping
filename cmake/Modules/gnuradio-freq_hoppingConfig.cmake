find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_FREQ_HOPPING gnuradio-freq_hopping)

FIND_PATH(
    GR_FREQ_HOPPING_INCLUDE_DIRS
    NAMES gnuradio/freq_hopping/api.h
    HINTS $ENV{FREQ_HOPPING_DIR}/include
        ${PC_FREQ_HOPPING_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_FREQ_HOPPING_LIBRARIES
    NAMES gnuradio-freq_hopping
    HINTS $ENV{FREQ_HOPPING_DIR}/lib
        ${PC_FREQ_HOPPING_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-freq_hoppingTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_FREQ_HOPPING DEFAULT_MSG GR_FREQ_HOPPING_LIBRARIES GR_FREQ_HOPPING_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_FREQ_HOPPING_LIBRARIES GR_FREQ_HOPPING_INCLUDE_DIRS)
