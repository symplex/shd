#
# Copyright 2010-2015 Ettus Research LLC
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

########################################################################
# Helpful Macros
########################################################################
MACRO(LIBSHD_APPEND_SOURCES)
    LIST(APPEND libshd_sources ${ARGV})
ENDMACRO(LIBSHD_APPEND_SOURCES)

MACRO(LIBSHD_APPEND_LIBS)
    LIST(APPEND libshd_libs ${ARGV})
ENDMACRO(LIBSHD_APPEND_LIBS)

MACRO(LIBSHD_PYTHON_GEN_SOURCE pyfile outfile)
    #ensure that the directory exists for outfile
    GET_FILENAME_COMPONENT(outfile_dir ${outfile} PATH)
    FILE(MAKE_DIRECTORY ${outfile_dir})

    #make the outfile depend on the python script
    ADD_CUSTOM_COMMAND(
        OUTPUT ${outfile} DEPENDS ${pyfile} ${LIBSHD_PYTHON_GEN_SOURCE_DEPS}
        COMMAND ${PYTHON_EXECUTABLE} -B ${pyfile} ${outfile}
        COMMENT "Generating ${outfile}"
    )

    #make libshd depend on the outfile
    LIBSHD_APPEND_SOURCES(${outfile})
ENDMACRO(LIBSHD_PYTHON_GEN_SOURCE)

MACRO(INCLUDE_SUBDIRECTORY subdir)
    #insert the current directories on the front of the list
    LIST(INSERT _cmake_source_dirs 0 ${CMAKE_CURRENT_SOURCE_DIR})
    LIST(INSERT _cmake_binary_dirs 0 ${CMAKE_CURRENT_BINARY_DIR})

    #set the current directories to the names of the subdirs
    SET(CMAKE_CURRENT_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${subdir})
    SET(CMAKE_CURRENT_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/${subdir})

    #include the subdirectory CMakeLists to run it
    FILE(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
    INCLUDE(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt)

    #reset the value of the current directories
    LIST(GET _cmake_source_dirs 0 CMAKE_CURRENT_SOURCE_DIR)
    LIST(GET _cmake_binary_dirs 0 CMAKE_CURRENT_BINARY_DIR)

    #pop the subdir names of the front of the list
    LIST(REMOVE_AT _cmake_source_dirs 0)
    LIST(REMOVE_AT _cmake_binary_dirs 0)
ENDMACRO(INCLUDE_SUBDIRECTORY)

########################################################################
# Register lower level components
########################################################################
MESSAGE(STATUS "")
# Dependencies
FIND_PACKAGE(USB1)
FIND_PACKAGE(GPSD)
LIBSHD_REGISTER_COMPONENT("USB" ENABLE_USB ON "ENABLE_LIBSHD;LIBUSB_FOUND" OFF OFF)
LIBSHD_REGISTER_COMPONENT("GPSD" ENABLE_GPSD OFF "ENABLE_LIBSHD;ENABLE_GPSD;LIBGPS_FOUND" OFF OFF)
# Devices
LIBSHD_REGISTER_COMPONENT("B100" ENABLE_B100 ON "ENABLE_LIBSHD;ENABLE_USB" OFF OFF)
LIBSHD_REGISTER_COMPONENT("B200" ENABLE_B200 ON "ENABLE_LIBSHD;ENABLE_USB" OFF OFF)
LIBSHD_REGISTER_COMPONENT("E100" ENABLE_E100 OFF "ENABLE_LIBSHD;LINUX" OFF OFF)
LIBSHD_REGISTER_COMPONENT("E300" ENABLE_E300 OFF "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("SMINI1" ENABLE_SMINI1 ON "ENABLE_LIBSHD;ENABLE_USB" OFF OFF)
LIBSHD_REGISTER_COMPONENT("SMINI2" ENABLE_SMINI2 ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("X300" ENABLE_X300 ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("N230" ENABLE_N230 ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("OctoClock" ENABLE_OCTOCLOCK ON "ENABLE_LIBSHD" OFF OFF)

########################################################################
# Include subdirectories (different than add)
########################################################################
INCLUDE_SUBDIRECTORY(ic_reg_maps)
INCLUDE_SUBDIRECTORY(types)
INCLUDE_SUBDIRECTORY(convert)
INCLUDE_SUBDIRECTORY(rfnoc)
INCLUDE_SUBDIRECTORY(smini)
INCLUDE_SUBDIRECTORY(smini_clock)
INCLUDE_SUBDIRECTORY(utils)
INCLUDE_SUBDIRECTORY(experts)
INCLUDE_SUBDIRECTORY(transport)

########################################################################
# Build info
########################################################################
INCLUDE(SHDBuildInfo)
SHD_LOAD_BUILD_INFO()
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/build_info.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/build_info.cpp
@ONLY)

########################################################################
# Setup SHD_VERSION_STRING for version.cpp
########################################################################
CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
@ONLY)

########################################################################
# Append to the list of sources for lib shd
########################################################################
LIBSHD_APPEND_SOURCES(
    ${CMAKE_CURRENT_BINARY_DIR}/build_info.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/deprecated.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/device.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/device3.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/image_loader.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/exception.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/property_tree.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

IF(ENABLE_C_API)
    LIBSHD_APPEND_SOURCES(
        ${CMAKE_CURRENT_SOURCE_DIR}/error_c.cpp
    )
ENDIF(ENABLE_C_API)

########################################################################
# Add DLL resource file to Windows build
########################################################################
IF(MSVC)
    MATH(EXPR TRIMMED_VERSION_MAJOR_API "${TRIMMED_VERSION_MAJOR} * 1000 + ${TRIMMED_VERSION_API}")
    SET(RC_TRIMMED_VERSION_PATCH ${TRIMMED_VERSION_PATCH})
    IF(SHD_VERSION_DEVEL)
        SET(RC_TRIMMED_VERSION_PATCH "999")
    ENDIF(SHD_VERSION_DEVEL)

    # Allow a custom .rc template file to be used
    IF(CUSTOM_RC_FILE)
        IF(IS_ABSOLUTE "${CUSTOM_RC_FILE}")
            SET(SHD_RC_IN "${CUSTOM_RC_FILE}")
        ELSE()
            SET(SHD_RC_IN "${CMAKE_BINARY_DIR}/${CUSTOM_RC_FILE}")
        ENDIF(IS_ABSOLUTE "${CUSTOM_RC_FILE}")
        MESSAGE(STATUS "")
        MESSAGE(STATUS "Using custom RC template: ${SHD_RC_IN}")
        MESSAGE(STATUS "")
    ELSE()
        SET(SHD_RC_IN "${CMAKE_CURRENT_SOURCE_DIR}/shd.rc.in")
    ENDIF(CUSTOM_RC_FILE)
    SET(SHD_RC_IN ${SHD_RC_IN} CACHE STRING "shd.rc template filepath")

    CONFIGURE_FILE(
        ${SHD_RC_IN}
        ${CMAKE_CURRENT_BINARY_DIR}/shd.rc
    @ONLY)

    LIST(APPEND libshd_sources ${CMAKE_CURRENT_BINARY_DIR}/shd.rc)
ENDIF(MSVC)

########################################################################
# Setup libshd library
########################################################################
ADD_LIBRARY(shd SHARED ${libshd_sources})
TARGET_LINK_LIBRARIES(shd ${Boost_LIBRARIES} ${libshd_libs})
SET_TARGET_PROPERTIES(shd PROPERTIES DEFINE_SYMBOL "SHD_DLL_EXPORTS")
IF(NOT LIBSHDDEV_PKG)
    SET_TARGET_PROPERTIES(shd PROPERTIES SOVERSION "${SHD_VERSION_MAJOR}")
    SET_TARGET_PROPERTIES(shd PROPERTIES VERSION "${SHD_VERSION_MAJOR}.${SHD_VERSION_API}")
ENDIF(NOT LIBSHDDEV_PKG)
IF(DEFINED LIBSHD_OUTPUT_NAME)
    SET_TARGET_PROPERTIES(shd PROPERTIES OUTPUT_NAME ${LIBSHD_OUTPUT_NAME})
ENDIF(DEFINED LIBSHD_OUTPUT_NAME)

IF(NOT SHDHOST_PKG) #Syntax makes it unusable by SHD_INSTALL
    INSTALL(TARGETS shd
        LIBRARY DESTINATION ${LIBRARY_DIR} COMPONENT libraries # .so file
        ARCHIVE DESTINATION ${LIBRARY_DIR} COMPONENT libraries # .lib file
        RUNTIME DESTINATION ${RUNTIME_DIR} COMPONENT libraries # .dll file
    )
ENDIF(NOT SHDHOST_PKG)

#######################################################
# Setup libshd library (static)
#######################################################
IF(ENABLE_STATIC_LIBS)
    ADD_LIBRARY(shd_static STATIC ${libshd_sources})
    SET_TARGET_PROPERTIES(shd_static PROPERTIES OUTPUT_NAME shd)
    INSTALL(TARGETS shd_static
        ARCHIVE DESTINATION lib${LIB_SUFFIX} # .lib or .a file
    )
ENDIF(ENABLE_STATIC_LIBS)
