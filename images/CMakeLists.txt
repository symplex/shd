#
# Copyright 2010-2012 Ettus Research LLC
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
# Setup Project
########################################################################
#OPTION(SHD_PATCH_OVERRIDE "Use this string to force a patch level version string." OFF)
MESSAGE(STATUS ${SHD_PATCH_OVERRIDE})
IF(DEFINED SHD_PATCH_OVERRIDE)
    SET(SHD_VERSION_PATCH_OVERRIDE ${SHD_PATCH_OVERRIDE})
ENDIF(DEFINED SHD_PATCH_OVERRIDE)
CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(SHD-images NONE)
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/../host/cmake/Modules)
INCLUDE(SHDVersion) #sets version variables (used below)
EXECUTE_PROCESS(COMMAND "date" OUTPUT_VARIABLE DATETIME_NOW OUTPUT_STRIP_TRAILING_WHITESPACE) #sets the date

########################################################################
# Setup CPack
########################################################################
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Ettus Research - SMINI Hardware Driver Images")
SET(CPACK_PACKAGE_VENDOR              "Ettus Research LLC")
SET(CPACK_PACKAGE_CONTACT             "support@ettus.com")
SET(CPACK_PACKAGE_VERSION ${SHD_VERSION})
SET(CPACK_PACKAGE_FILE_NAME "shd-images_${SHD_VERSION}")
SET(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "all")
INCLUDE(CPack) #include after setting vars
MESSAGE(STATUS "Version: ${SHD_VERSION}")

########################################################################
# Install Images
########################################################################

#tag the images with a version number (something identifiable)
FILE(WRITE ${CMAKE_SOURCE_DIR}/images/${SHD_VERSION_MAJOR}.${SHD_VERSION_API}.${SHD_VERSION_ABI}.${SHD_VERSION_PATCH}.tag "${SHD_VERSION}\n${DATETIME_NOW}\n")
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/images DESTINATION share/shd)
INSTALL(FILES ${CMAKE_SOURCE_DIR}/../host/LICENSE DESTINATION share/shd/images)
