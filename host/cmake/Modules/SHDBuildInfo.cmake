#
# Copyright 2015-2016 National Instruments Corp.
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

#
# We need this to be macro because GET_DIRECTORY_PROPERTY works with
# the current directory.
#
MACRO(SHD_LOAD_BUILD_INFO)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "Loading build info.")

    # Build date
    IF(IGNORE_BUILD_DATE)
        SET(SHD_BUILD_DATE "")
    ELSE()
        EXECUTE_PROCESS(COMMAND ${PYTHON_EXECUTABLE} -c
            "import time; print(time.strftime('%a, %d %b %Y %H:%M:%S', time.gmtime()))"
            OUTPUT_VARIABLE SHD_BUILD_DATE OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    ENDIF(IGNORE_BUILD_DATE)

    # Compiler name
    IF(MSVC)
        IF(MSVC10)
            SET(SHD_C_COMPILER "MSVC 2010")
            SET(SHD_CXX_COMPILER "MSVC 2010")
        ELSEIF(MSVC11)
            SET(SHD_C_COMPILER "MSVC 2012")
            SET(SHD_CXX_COMPILER "MSVC 2012")
        ELSEIF(MSVC12)
            SET(SHD_C_COMPILER "MSVC 2013")
            SET(SHD_CXX_COMPILER "MSVC 2013")
        ELSEIF(MSVC14)
            SET(SHD_C_COMPILER "MSVC 2015")
            SET(SHD_CXX_COMPILER "MSVC 2015")
        ELSE()
            # Go with the ugly string
            SET(SHD_C_COMPILER "MSVC ${CMAKE_C_COMPILER_VERSION}")
            SET(SHD_CXX_COMPILER "MSVC ${CMAKE_CXX_COMPILER_VERSION}")
        ENDIF(MSVC10)
    ELSE()
        SET(SHD_C_COMPILER "${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
        SET(SHD_CXX_COMPILER "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
    ENDIF(MSVC)

    # Compiler flags
    GET_DIRECTORY_PROPERTY(shd_flags COMPILE_DEFINITIONS)
    SET(SHD_C_FLAGS "${shd_flags}${CMAKE_C_FLAGS}") # CMAKE_C_FLAGS starts with a space
    SET(SHD_CXX_FLAGS "${shd_flags}${CMAKE_CXX_FLAGS}") # CMAKE_CXX_FLAGS starts with a space
ENDMACRO(SHD_LOAD_BUILD_INFO)
