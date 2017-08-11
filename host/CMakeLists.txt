#
# Copyright 2010-2016 Ettus Research LLC
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
#IF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
#    MESSAGE(FATAL_ERROR "Prevented in-tree built. This is bad practice.")
#ENDIF(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})

########################################################################
# Project setup
########################################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(SHD CXX C)
ENABLE_TESTING()

#make sure our local CMake Modules path comes first
list(INSERT CMAKE_MODULE_PATH 0 ${CMAKE_SOURCE_DIR}/cmake/Modules)

########################################################################
# Check Compiler Version
########################################################################
# Full C++11 came with GCC 4.7.
SET(GCC_MIN_VERSION "4.8.0")
# for c++0x or c++11 support, require:
#   Apple Clang >= 500
# or
#   Clang >= 3.3.0
SET(CLANG_MIN_VERSION "3.3.0")
SET(APPLECLANG_MIN_VERSION "500")

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    IF(DEFINED CMAKE_CXX_COMPILER_VERSION)
        IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_LESS ${GCC_MIN_VERSION})
            MESSAGE(WARNING "\nThe compiler selected to build SHD (GCC version ${CMAKE_CXX_COMPILER_VERSION} : ${CMAKE_CXX_COMPILER}) is older than that officially supported (${GCC_MIN_VERSION} minimum). This build may or not work. We highly recommend using a more recent GCC version.")
         ENDIF()
    ELSE()
        MESSAGE(WARNING "\nCannot determine the version of the compiler selected to build SHD (GCC : ${CMAKE_CXX_COMPILER}). This build may or not work. We highly recommend using GCC version ${GCC_MIN_VERSION} or more recent.")
    ENDIF()
    SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")
ELSEIF(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    EXECUTE_PROCESS(COMMAND
        ${CMAKE_CXX_COMPILER} -v
        RESULT_VARIABLE res ERROR_VARIABLE err
        ERROR_STRIP_TRAILING_WHITESPACE)
    IF(${res} STREQUAL "0")
        # output is in error stream
        STRING(REGEX MATCH "^Apple.*" IS_APPLE ${err})
        IF("${IS_APPLE}" STREQUAL "")
            SET(MIN_VERSION ${CLANG_MIN_VERSION})
            SET(APPLE_STR "")
            # retrieve the compiler's version from it
            STRING(REGEX MATCH "clang version [0-9.]+" CLANG_OTHER_VERSION ${err})
            STRING(REGEX MATCH "[0-9.]+" CLANG_VERSION ${CLANG_OTHER_VERSION})
        ELSE()
            SET(MIN_VERSION ${APPLECLANG_MIN_VERSION})
            SET(APPLE_STR "Apple ")
            # retrieve the compiler's version from it
            STRING(REGEX MATCH "(clang-[0-9.]+)" CLANG_APPLE_VERSION ${err})
            STRING(REGEX MATCH "[0-9.]+" CLANG_VERSION ${CLANG_APPLE_VERSION})
        ENDIF()
        IF(${CLANG_VERSION} VERSION_LESS "${MIN_VERSION}")
            MESSAGE(WARNING "\nThe compiler selected to build SHD (${APPLE_STR}Clang version ${CLANG_VERSION} : ${CMAKE_CXX_COMPILER}) is older than that officially supported (${MIN_VERSION} minimum). This build may or not work. We highly recommend using Apple Clang version ${APPLECLANG_MIN_VERSION} or more recent, or Clang version ${CLANG_MIN_VERSION} or more recent.")
        ENDIF()
    ELSE()
        MESSAGE(WARNING "\nCannot determine the version of the compiler selected to build SHD (${APPLE_STR}Clang : ${CMAKE_CXX_COMPILER}). This build may or not work. We highly recommend using Apple Clang version ${APPLECLANG_MIN_VERSION} or more recent, or Clang version ${CLANG_MIN_VERSION} or more recent.")
    ENDIF()
ENDIF()

IF(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    SET(CMAKE_EXE_LINKER_FLAGS "-lthr ${CMAKE_EXE_LINKER_FLAGS}")
ENDIF()
########################################################################
# Packaging Variables
########################################################################

SET(LIBSHD_PKG ${LIBSHD_PKG} CACHE BOOL "Build Debian libshd003 package")
SET(LIBSHDDEV_PKG ${LIBSHDDEV_PKG} CACHE BOOL "Build Debian libshd-dev package")
SET(SHDHOST_PKG ${SHDPOST_PKG} CACHE BOOL "Build Debian shd-host package")

INCLUDE(SHDComponent) #enable components
INCLUDE(SHDPackage)   #setup cpack

########################################################################
# Install Dirs
########################################################################
#when the library suffix should be 64 (applies to redhat linux family)
IF(NOT DEFINED LIB_SUFFIX AND REDHAT AND CMAKE_SYSTEM_PROCESSOR MATCHES "64$")
    SET(LIB_SUFFIX 64)
ENDIF()
IF(CMAKE_INSTALL_LIBDIR MATCHES lib64)
    SET(LIB_SUFFIX 64)
ENDIF()

SET(LIB_SUFFIX ${LIB_SUFFIX} CACHE STRING "lib directory suffix")
SET(RUNTIME_DIR bin)
SET(LIBRARY_DIR lib${LIB_SUFFIX})
SET(INCLUDE_DIR include)
SET(PKG_DATA_DIR share/shd)
IF(NOT DEFINED PKG_LIB_DIR)
    SET(PKG_LIB_DIR ${LIBRARY_DIR}/shd)
ENDIF()
SET(PKG_DOC_DIR share/doc/shd)
SET(PKG_MAN_DIR share/man/man1)

########################################################################
# SHD Image Directories
########################################################################
IF(NOT DEFINED SHD_IMAGES_DIR)
    IF(DEFINED FPGA_IMAGES_DIR)
        SET(SHD_IMAGES_DIR ${FPGA_IMAGES_DIR})
    ELSE(DEFINED FPGA_IMAGES_DIR)
        FILE(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/share/shd/images using_images_dir)
        SET(SHD_IMAGES_DIR ${using_images_dir})
    ENDIF(DEFINED FPGA_IMAGES_DIR)
ENDIF(NOT DEFINED SHD_IMAGES_DIR)

OPTION(SHD_IMAGES_DIR "Path to installed SHD image binaries.")
MESSAGE( STATUS "Using SHD Images Directory: ${SHD_IMAGES_DIR}" )
ADD_DEFINITIONS(-DSHD_IMAGES_DIR=${SHD_IMAGES_DIR})

IF(DEFINED SHD_IMAGES_DIR_WINREG_KEY)
    ADD_DEFINITIONS(-DSHD_IMAGES_DIR_WINREG_KEY=${SHD_IMAGES_DIR_WINREG_KEY})
ENDIF(DEFINED SHD_IMAGES_DIR_WINREG_KEY)

########################################################################
# Local Include Dir
########################################################################
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)

########################################################################
# Static Lib Configuration
########################################################################
OPTION(ENABLE_STATIC_LIBS "Enable building of static libraries" OFF)
IF(ENABLE_STATIC_LIBS)
    MESSAGE(STATUS "Building Static Libraries: ${ENABLE_STATIC_LIBS}")
ENDIF(ENABLE_STATIC_LIBS)

########################################################################
# On Apple only, set install name and use rpath correctly, if not already set
########################################################################
IF(APPLE)
    IF(NOT CMAKE_INSTALL_NAME_DIR)
        SET(CMAKE_INSTALL_NAME_DIR
            ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR} CACHE
            PATH "Library Install Name Destination Directory" FORCE)
    ENDIF(NOT CMAKE_INSTALL_NAME_DIR)
    IF(NOT CMAKE_INSTALL_RPATH)
        SET(CMAKE_INSTALL_RPATH
            ${CMAKE_INSTALL_PREFIX}/${LIBRARY_DIR} CACHE
            PATH "Library Install RPath" FORCE)
    ENDIF(NOT CMAKE_INSTALL_RPATH)
    IF(NOT CMAKE_BUILD_WITH_INSTALL_RPATH)
        SET(CMAKE_BUILD_WITH_INSTALL_RPATH ON CACHE
            BOOL "Do Build Using Library Install RPath" FORCE)
    ENDIF(NOT CMAKE_BUILD_WITH_INSTALL_RPATH)
ENDIF(APPLE)

########################################################################
# Optional Compiler Flags
########################################################################
INCLUDE(CheckCXXCompilerFlag)
MACRO(SHD_ADD_OPTIONAL_CXX_COMPILER_FLAG flag have)
    CHECK_CXX_COMPILER_FLAG(${flag} ${have})
    IF(${have})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    ENDIF(${have})
ENDMACRO(SHD_ADD_OPTIONAL_CXX_COMPILER_FLAG)

#select the release build type by default to get optimization flags
IF(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE "Release")
   MESSAGE(STATUS "Build type not specified: defaulting to release.")
ENDIF(NOT CMAKE_BUILD_TYPE)
SET(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING "")

#force SHD_RELEASE_MODE to be a string for cmake-gui
SET(SHD_RELEASE_MODE "${SHD_RELEASE_MODE}" CACHE STRING "SHD Release Mode")

IF(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" OR
    ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang")
    IF(STRIP_BINARIES)
        IF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s")
        ENDIF(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    ENDIF(STRIP_BINARIES)
    ADD_DEFINITIONS(-Wall)
    ADD_DEFINITIONS(-Wextra)
    ADD_DEFINITIONS(-Wsign-compare)
    #ADD_DEFINITIONS(-Wconversion)
    #ADD_DEFINITIONS(-pedantic)
    #ADD_DEFINITIONS(-ansi)
    IF(NOT WIN32)
        #only export symbols that are declared to be part of the shd api (non dll platforms)
        SHD_ADD_OPTIONAL_CXX_COMPILER_FLAG(-fvisibility=hidden HAVE_VISIBILITY_HIDDEN)
        SHD_ADD_OPTIONAL_CXX_COMPILER_FLAG(-fvisibility-inlines-hidden HAVE_VISIBILITY_INLINES_HIDDEN)
    ENDIF(NOT WIN32)
ENDIF()

IF(MSVC)
    INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/cmake/msvc)
    ADD_DEFINITIONS( #stop all kinds of compatibility warnings
        -D_SCL_SECURE_NO_WARNINGS
        -D_SCL_SECURE_NO_DEPRECATE
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_SECURE_NO_DEPRECATE
        -D_CRT_NONSTDC_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_WINSOCK_DEPRECATED_NO_WARNINGS
    )
    # multi-threaded build and increases the number of addressable sections in an .obj file.
    SET (COMPILE_FLAGS ${COMPILE_FLAGS} /MP /bigobj)
ENDIF(MSVC)

IF(CYGWIN)
    ADD_DEFINITIONS(-D__USE_W32_SOCKETS) #boost asio says we need this
ENDIF(CYGWIN)

IF(WIN32)
    ADD_DEFINITIONS(-D_WIN32_WINNT=0x0501) #minimum version required is windows xp
    ADD_DEFINITIONS(-DNOMINMAX) #disables stupidity and enables std::min and std::max
ENDIF(WIN32)

########################################################################
# Setup Boost
########################################################################
MESSAGE(STATUS "")
MESSAGE(STATUS "Configuring Boost C++ Libraries...")
SET(BOOST_REQUIRED_COMPONENTS
    chrono
    date_time
    filesystem
    program_options
    regex
    system
    unit_test_framework
    serialization
)
IF(MINGW)
    LIST(APPEND BOOST_REQUIRED_COMPONENTS thread_win32)
ELSE()
    LIST(APPEND BOOST_REQUIRED_COMPONENTS thread)
ENDIF()

IF(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64")
    LIST(APPEND BOOST_LIBRARYDIR "/usr/lib64") #fedora 64-bit fix
ENDIF(UNIX AND NOT BOOST_ROOT AND EXISTS "/usr/lib64")

IF(MSVC)
    SET(BOOST_ALL_DYN_LINK "${BOOST_ALL_DYN_LINK}" CACHE BOOL "boost enable dynamic linking")
    IF(BOOST_ALL_DYN_LINK)
        ADD_DEFINITIONS(-DBOOST_ALL_DYN_LINK) #setup boost auto-linking in msvc
    ELSE(BOOST_ALL_DYN_LINK)
        SET(BOOST_REQUIRED_COMPONENTS) #empty components list for static link
    ENDIF(BOOST_ALL_DYN_LINK)
ENDIF(MSVC)

SET(Boost_ADDITIONAL_VERSIONS
    "1.46.0" "1.46" "1.47.0" "1.47" "1.48.0" "1.48" "1.48.0" "1.49" "1.50.0" "1.50"
    "1.51.0" "1.51" "1.52.0" "1.52" "1.53.0" "1.53" "1.54.0" "1.54" "1.55.0" "1.55"
    "1.56.0" "1.56" "1.57" "1.57" "1.58" "1.59" "1.60" "1.61"
)
FIND_PACKAGE(Boost 1.53 COMPONENTS ${BOOST_REQUIRED_COMPONENTS})

INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
LINK_DIRECTORIES(${Boost_LIBRARY_DIRS})

MESSAGE(STATUS "Boost include directories: ${Boost_INCLUDE_DIRS}")
MESSAGE(STATUS "Boost library directories: ${Boost_LIBRARY_DIRS}")
MESSAGE(STATUS "Boost libraries: ${Boost_LIBRARIES}")

########################################################################
# Additional settings for build environment
########################################################################
# Note: RFNoC never gets fully disabled, but the public APIs do
SET(ENABLE_RFNOC OFF CACHE BOOL "Export RFNoC includes and symbols")
INCLUDE(SHDGlobalDefs)

########################################################################
# Check Python Modules
########################################################################
INCLUDE(SHDPython)

PYTHON_CHECK_MODULE(
    "Python version 2.7 or greater"
    "platform" "platform.python_version() >= '2.7'"
    HAVE_PYTHON_PLAT_MIN_VERSION
)

PYTHON_CHECK_MODULE(
    "Mako templates 0.4.2 or greater"
    "mako" "mako.__version__ >= '0.4.2'"
    HAVE_PYTHON_MODULE_MAKO
)

PYTHON_CHECK_MODULE(
    "requests 2.0 or greater"
    "requests" "requests.__version__ >= '2.0'"
    HAVE_PYTHON_MODULE_REQUESTS
)

########################################################################
# Create Uninstall Target
########################################################################
CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/cmake/cmake_uninstall.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
@ONLY)

ADD_CUSTOM_TARGET(uninstall
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake
)

########################################################################
# Install Package Docs
########################################################################
SHD_INSTALL(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/README.md
    ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
    DESTINATION ${PKG_DOC_DIR}
    COMPONENT readme
)

########################################################################
# Images download directory for utils/shd_images_downloader.py
########################################################################
#{{{IMG_SECTION
# This section is written automatically by /images/create_imgs_package.py
# Any manual changes in here will be overwritten.
SET(SHD_IMAGES_MD5SUM "8e6e5edab697fa7e3c63c612ba6f79f5")
SET(SHD_IMAGES_DOWNLOAD_SRC "shd-images_003.010.002.000-release.zip")
#}}}

########################################################################
# Register top level components
########################################################################
LIBSHD_REGISTER_COMPONENT("LibSHD" ENABLE_LIBSHD ON "Boost_FOUND;HAVE_PYTHON_PLAT_MIN_VERSION;HAVE_PYTHON_MODULE_MAKO" OFF ON)
LIBSHD_REGISTER_COMPONENT("LibSHD - C API" ENABLE_C_API ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("Examples" ENABLE_EXAMPLES ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("Utils" ENABLE_UTILS ON "ENABLE_LIBSHD" OFF OFF)
LIBSHD_REGISTER_COMPONENT("Tests" ENABLE_TESTS ON "ENABLE_LIBSHD" OFF OFF)

########################################################################
# Check for fpga-src submodule
########################################################################
SET(HAS_FPGA_SUBMODULE FALSE)
EXECUTE_PROCESS(
    COMMAND ${PYTHON_EXECUTABLE} -c "import os; print(os.path.abspath(os.path.join('${CMAKE_SOURCE_DIR}', '..', 'fpga-src')))"
    OUTPUT_VARIABLE FPGA_SUBMODULE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
IF(EXISTS "${FPGA_SUBMODULE_DIR}/docs/fpga.md")
    SET(HAS_FPGA_SUBMODULE TRUE)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "Found FPGA submodule: ${FPGA_SUBMODULE_DIR}")
ENDIF(EXISTS "${FPGA_SUBMODULE_DIR}/docs/fpga.md")

########################################################################
# Add the subdirectories
########################################################################
IF(ENABLE_LIBSHD)
    ADD_SUBDIRECTORY(lib)
ENDIF(ENABLE_LIBSHD)

ADD_SUBDIRECTORY(include)

IF(ENABLE_EXAMPLES)
    ADD_SUBDIRECTORY(examples)
ENDIF(ENABLE_EXAMPLES)

IF(ENABLE_TESTS)
    ADD_SUBDIRECTORY(tests)
ENDIF(ENABLE_TESTS)

IF(ENABLE_UTILS)
    ADD_SUBDIRECTORY(utils)
ENDIF(ENABLE_UTILS)

ADD_SUBDIRECTORY(docs)

########################################################################
# Create Pkg Config File
########################################################################
FOREACH(inc ${Boost_INCLUDE_DIRS})
    LIST(APPEND SHD_PC_CFLAGS "-I${inc}")
ENDFOREACH(inc)

FOREACH(lib ${Boost_LIBRARY_DIRS})
    LIST(APPEND SHD_PC_LIBS "-L${lib}")
ENDFOREACH(lib)

#use space-separation format for the pc file
STRING(REPLACE ";" " " SHD_PC_REQUIRES "${SHD_PC_REQUIRES}")
STRING(REPLACE ";" " " SHD_PC_CFLAGS "${SHD_PC_CFLAGS}")
STRING(REPLACE ";" " " SHD_PC_LIBS "${SHD_PC_LIBS}")

#unset these vars to avoid hard-coded paths to cross environment
IF(CMAKE_CROSSCOMPILING)
    SET(SHD_PC_CFLAGS)
    SET(SHD_PC_LIBS)
ENDIF(CMAKE_CROSSCOMPILING)

CONFIGURE_FILE(
    ${CMAKE_CURRENT_SOURCE_DIR}/shd.pc.in
    ${CMAKE_CURRENT_BINARY_DIR}/shd.pc
@ONLY)

SHD_INSTALL(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/shd.pc
    DESTINATION ${LIBRARY_DIR}/pkgconfig
    COMPONENT "devel"
)

########################################################################
# Create and Install SHD cmake files
########################################################################
IF(NOT CMAKE_MODULES_DIR)
   SET(CMAKE_MODULES_DIR lib${LIB_SUFFIX}/cmake)
ENDIF(NOT CMAKE_MODULES_DIR)

# SHDConfig.cmake needs SHD_LINK_LIST_STATIC set:
LIST(APPEND SHD_LINK_LIST_STATIC "dl")
LIST(APPEND SHD_LINK_LIST_STATIC "pthread")
FOREACH(Boost_Comp ${BOOST_REQUIRED_COMPONENTS})
    IF(NOT ${Boost_Comp} STREQUAL "unit_test_framework")
        LIST(APPEND SHD_LINK_LIST_STATIC "boost_${Boost_Comp}")
    ENDIF(NOT ${Boost_Comp} STREQUAL "unit_test_framework")
ENDFOREACH(Boost_Comp)
IF(ENABLE_USB)
    LIST(APPEND SHD_LINK_LIST_STATIC "usb-1.0")
ENDIF(ENABLE_USB)
# SHDConfig.cmake also needs SHD_RFNOC_FOUND
IF(ENABLE_RFNOC)
    SET(SHD_RFNOC_FOUND "TRUE")
ELSE()
    SET(SHD_RFNOC_FOUND "FALSE")
ENDIF(ENABLE_RFNOC)

CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/cmake/Modules/SHDConfigVersion.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/Modules/SHDConfigVersion.cmake
    @ONLY
)
CONFIGURE_FILE(
    ${CMAKE_SOURCE_DIR}/cmake/Modules/SHDConfig.cmake.in
    ${CMAKE_BINARY_DIR}/cmake/Modules/SHDConfig.cmake
    @ONLY
)

SET(cmake_configs
    ${CMAKE_BINARY_DIR}/cmake/Modules/SHDConfig.cmake
    ${CMAKE_BINARY_DIR}/cmake/Modules/SHDConfigVersion.cmake
)

SHD_INSTALL(
    FILES ${cmake_configs}
    DESTINATION ${CMAKE_MODULES_DIR}/shd
    COMPONENT "devel"
)

########################################################################
# Handle pre-built SHD Images for packaging
########################################################################
IF(DEFINED SHD_IMAGES_SRC_DIR AND EXISTS "${SHD_IMAGES_SRC_DIR}")
    MESSAGE(STATUS "Reading images from directory `${SHD_IMAGES_SRC_DIR}'")
    FILE(GLOB_RECURSE _image_files "${SHD_IMAGES_SRC_DIR}/*")
    MESSAGE(STATUS "These images files will be installed/packaged:")
    FOREACH(_img ${_image_files})
        MESSAGE(STATUS "  ${_img}")
    ENDFOREACH(_img)
    SHD_INSTALL(FILES ${_image_files} DESTINATION ${PKG_DATA_DIR}/images COMPONENT images)
ENDIF(DEFINED SHD_IMAGES_SRC_DIR AND EXISTS "${SHD_IMAGES_SRC_DIR}")

########################################################################
# Print Summary
########################################################################
IF(LIBSHD_PKG)
    MESSAGE(STATUS "")
    SET(PRINT_APPEND " (Debian libshd003 package configuration)")
ELSEIF(LIBSHDDEV_PKG)
    MESSAGE(STATUS "")
    SET(PRINT_APPEND " (Debian libshd-dev package configuration)")
ELSEIF(SHDHOST_PKG)
    MESSAGE(STATUS "")
    SET(PRINT_APPEND " (Debian shd-host package configuration)")
ENDIF(LIBSHD_PKG)
SHD_PRINT_COMPONENT_SUMMARY()
IF(SHD_VERSION_DEVEL AND NOT SHD_GIT_BRANCH STREQUAL "maint")
    MESSAGE(STATUS "******************************************************")
    IF(SHD_GIT_BRANCH STREQUAL "master")
        MESSAGE(STATUS "* You are building the SHD development master branch.")
        MESSAGE(STATUS "* For production code, we recommend our stable,")
        MESSAGE(STATUS "* releases or using the release branch (maint).")
    ELSE()
        MESSAGE(STATUS "* You are building a development branch of SHD.")
        MESSAGE(STATUS "* These branches are designed to provide early access")
        MESSAGE(STATUS "* to SHD and SMINI features, but should be considered")
        MESSAGE(STATUS "* unstable and/or experimental!")
    ENDIF(SHD_GIT_BRANCH STREQUAL "master")
    MESSAGE(STATUS "******************************************************")
ENDIF(SHD_VERSION_DEVEL AND NOT SHD_GIT_BRANCH STREQUAL "maint")
MESSAGE(STATUS "Building version: ${SHD_VERSION}${PRINT_APPEND}")
MESSAGE(STATUS "Using install prefix: ${CMAKE_INSTALL_PREFIX}")
