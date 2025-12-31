# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT freq_hopping_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT freq_hopping_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################
list(APPEND freq_hopping_python_files
    slot_frame_python.cc
    bb_pskmod_python.cc
    hop_interp_python.cc
    hop_mod_python.cc
    hop_demod_python.cc
    symbol_recover_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(freq_hopping
   ../../..
   gr::freq_hopping
   "${freq_hopping_python_files}")

# copy in bindings .so file for use in QA test module
add_custom_target(
  copy_bindings_for_tests ALL
  COMMAND
    ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/*.so"
    ${CMAKE_BINARY_DIR}/test_modules/gnuradio/freq_hopping/
  DEPENDS freq_hopping_python)

install(TARGETS freq_hopping_python DESTINATION ${GR_PYTHON_DIR}/gnuradio/freq_hopping COMPONENT pythonapi)
