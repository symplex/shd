#
# Copyright 2010-2014,2016 Ettus Research LLC
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
INCLUDE(SHDPython) #requires python for parsing
FIND_PACKAGE(Git QUIET)

########################################################################
# Setup Version Numbers
#  - Increment major on large-scale library changes
#  - Increment API on API changes
#  - Increment ABI on ABI changes
#  - Increment patch for bugfixes and docs
#  - set SHD_VERSION_DEVEL to true for master and development branches
########################################################################
SET(SHD_VERSION_MAJOR 003)
SET(SHD_VERSION_API   010)
SET(SHD_VERSION_ABI   002)
SET(SHD_VERSION_PATCH 000)
SET(SHD_VERSION_DEVEL FALSE)

########################################################################
# If we're on a development branch, we skip the patch version
########################################################################
IF(DEFINED SHD_VERSION_PATCH_OVERRIDE)
    SET(SHD_VERSION_DEVEL FALSE)
    SET(SHD_VERSION_PATCH ${SHD_VERSION_PATCH_OVERRIDE})
ENDIF(DEFINED SHD_VERSION_PATCH_OVERRIDE)
IF(NOT DEFINED SHD_VERSION_DEVEL)
    SET(SHD_VERSION_DEVEL FALSE)
ENDIF(NOT DEFINED SHD_VERSION_DEVEL)
SET(SHD_GIT_BRANCH "")
IF(GIT_FOUND)
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${GIT_EXECUTABLE} rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE _git_branch OUTPUT_STRIP_TRAILING_WHITESPACE
        RESULT_VARIABLE _git_branch_result
    )
    IF(_git_branch_result EQUAL 0)
        SET(SHD_GIT_BRANCH ${_git_branch})
        IF(SHD_GIT_BRANCH STREQUAL "maint")
            MESSAGE(STATUS "Operating on maint branch (stable).")
	    SET(SHD_VERSION_DEVEL FALSE)
        ELSEIF(SHD_GIT_BRANCH STREQUAL "master")
            MESSAGE(STATUS "Operating on master branch.")
            SET(SHD_VERSION_DEVEL TRUE)
        ELSE()
            MESSAGE(STATUS "Working off of feature or development branch. Updating version number.")
            EXECUTE_PROCESS(
                COMMAND ${PYTHON_EXECUTABLE} -c "print('${_git_branch}'.replace('/', '-'))"
                OUTPUT_VARIABLE _git_safe_branch OUTPUT_STRIP_TRAILING_WHITESPACE
            )
            SET(SHD_VERSION_PATCH ${_git_safe_branch})
            SET(SHD_VERSION_DEVEL TRUE)
        ENDIF()
    ELSE()
        MESSAGE(STATUS "Could not determine git branch. Probably building from tarball.")
    ENDIF()
ENDIF(GIT_FOUND)

########################################################################
# Set up trimmed version numbers for DLL resource files and packages
########################################################################
FUNCTION(DEPAD_NUM input_num output_num)
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} -c "print(\"${input_num}\".lstrip(\"0\") or 0)"
        OUTPUT_VARIABLE depadded_num OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    SET(${output_num} ${depadded_num} PARENT_SCOPE)
ENDFUNCTION(DEPAD_NUM)

DEPAD_NUM(${SHD_VERSION_MAJOR} TRIMMED_VERSION_MAJOR)
DEPAD_NUM(${SHD_VERSION_API}   TRIMMED_VERSION_API)
DEPAD_NUM(${SHD_VERSION_ABI}   TRIMMED_VERSION_ABI)
IF(SHD_VERSION_DEVEL)
    SET(TRIMMED_VERSION_PATCH ${SHD_VERSION_PATCH})
ELSE(SHD_VERSION_DEVEL)
    DEPAD_NUM(${SHD_VERSION_PATCH} TRIMMED_VERSION_PATCH)
ENDIF(SHD_VERSION_DEVEL)
SET(TRIMMED_SHD_VERSION "${TRIMMED_VERSION_MAJOR}.${TRIMMED_VERSION_API}.${TRIMMED_VERSION_ABI}.${TRIMMED_VERSION_PATCH}")

########################################################################
# Version information discovery through git log
########################################################################

#grab the git ref id for the current head
EXECUTE_PROCESS(
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMAND ${GIT_EXECUTABLE} describe --always --abbrev=8 --long
    OUTPUT_VARIABLE _git_describe OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE _git_describe_result
)

#only set the build info on success
IF(_git_describe_result EQUAL 0)
    IF(NOT SHD_GIT_COUNT)
        EXECUTE_PROCESS(
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${PYTHON_EXECUTABLE} -c "
try:
    print('${_git_describe}'.split('-')[-2])
except IndexError:
    print('0')
"
            OUTPUT_VARIABLE SHD_GIT_COUNT OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ENDIF()
    IF(NOT SHD_GIT_HASH)
        EXECUTE_PROCESS(
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            COMMAND ${PYTHON_EXECUTABLE} -c "
try:
    print('${_git_describe}'.split('-')[-1])
except IndexError:
    print('unknown')
"
             OUTPUT_VARIABLE SHD_GIT_HASH OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ENDIF()
ENDIF()

## Set default values if all fails. Make sure they're identical to the ones above.
IF(NOT SHD_GIT_COUNT)
    SET(SHD_GIT_COUNT "0")
ENDIF()

IF(NOT SHD_GIT_HASH)
    SET(SHD_GIT_HASH "unknown")
ENDIF()

IF(SHD_RELEASE_MODE)
    SET(SHD_GIT_HASH ${SHD_RELEASE_MODE})

    #Ignore SHD_GIT_COUNT in SHD_VERSION if the string 'release' is in SHD_RELEASE_MODE
    EXECUTE_PROCESS(
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${PYTHON_EXECUTABLE} -c "print ('release' in '${SHD_RELEASE_MODE}') or ('rc' in '${SHD_RELEASE_MODE}')"
        OUTPUT_VARIABLE TRIM_SHD_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE
    )
ENDIF()


########################################################################
IF(TRIM_SHD_VERSION STREQUAL "True")
    SET(SHD_VERSION "${SHD_VERSION_MAJOR}.${SHD_VERSION_API}.${SHD_VERSION_ABI}.${SHD_VERSION_PATCH}-${SHD_GIT_HASH}")
ELSE()
    SET(SHD_VERSION "${SHD_VERSION_MAJOR}.${SHD_VERSION_API}.${SHD_VERSION_ABI}.${SHD_VERSION_PATCH}-${SHD_GIT_COUNT}-${SHD_GIT_HASH}")
ENDIF()
