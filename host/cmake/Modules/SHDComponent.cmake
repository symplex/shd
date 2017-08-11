#
# Copyright 2010-2011,2013,2015 Ettus Research LLC
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
SET(_shd_enabled_components "" CACHE INTERNAL "" FORCE)
SET(_shd_disabled_components "" CACHE INTERNAL "" FORCE)

########################################################################
# Register a component into the system
#  - name the component string name
#  - var the global enable variable
#  - enb the default enable setting
#  - deps a list of dependencies
#  - dis the default disable setting
#  - req fail if dependencies not met (unless specifically disabled)
########################################################################
MACRO(LIBSHD_REGISTER_COMPONENT name var enb deps dis req)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "Configuring ${name} support...")
    FOREACH(dep ${deps})
        MESSAGE(STATUS "  Dependency ${dep} = ${${dep}}")
    ENDFOREACH(dep)

    #if user specified option, store here
    IF("${${var}}" STREQUAL "OFF")
        SET(user_disabled TRUE)
    ELSE()
        SET(user_disabled FALSE)
    ENDIF("${${var}}" STREQUAL "OFF")

    #setup the dependent option for this component
    INCLUDE(CMakeDependentOption)
    CMAKE_DEPENDENT_OPTION(${var} "enable ${name} support" ${enb} "${deps}" ${dis})

    #if a required option's dependencies aren't met, fail unless user specifies otherwise
    IF(NOT ${var} AND ${req} AND NOT user_disabled)
        MESSAGE(FATAL_ERROR "Dependencies for required component ${name} not met.")
    ENDIF(NOT ${var} AND ${req} AND NOT user_disabled)

    #append the component into one of the lists
    IF(${var})
        MESSAGE(STATUS "  Enabling ${name} support.")
        LIST(APPEND _shd_enabled_components ${name})
    ELSE(${var})
        MESSAGE(STATUS "  Disabling ${name} support.")
        LIST(APPEND _shd_disabled_components ${name})
    ENDIF(${var})
    MESSAGE(STATUS "  Override with -D${var}=ON/OFF")

    #make components lists into global variables
    SET(_shd_enabled_components ${_shd_enabled_components} CACHE INTERNAL "" FORCE)
    SET(_shd_disabled_components ${_shd_disabled_components} CACHE INTERNAL "" FORCE)
ENDMACRO(LIBSHD_REGISTER_COMPONENT)

########################################################################
# Install only if appropriate for package and if component is enabled
########################################################################
FUNCTION(SHD_INSTALL)
    include(CMakeParseArgumentsCopy)
    CMAKE_PARSE_ARGUMENTS(SHD_INSTALL "" "DESTINATION;COMPONENT" "TARGETS;FILES;PROGRAMS" ${ARGN})

    IF(SHD_INSTALL_FILES)
        SET(TO_INSTALL "${SHD_INSTALL_FILES}")
    ELSEIF(SHD_INSTALL_PROGRAMS)
        SET(TO_INSTALL "${SHD_INSTALL_PROGRAMS}")
    ELSEIF(SHD_INSTALL_TARGETS)
        SET(TO_INSTALL "${SHD_INSTALL_TARGETS}")
    ENDIF(SHD_INSTALL_FILES)

    IF(SHD_INSTALL_COMPONENT STREQUAL "headers")
        IF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "devel")
        IF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "examples")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "tests")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "utilities")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "manual")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "doxygen")
        IF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT SHDHOST_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "manpages")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "images")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG AND NOT SHDHOST_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG AND NOT SHDHOST_PKG)
    ELSEIF(SHD_INSTALL_COMPONENT STREQUAL "readme")
        IF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG AND NOT SHDHOST_PKG)
            INSTALL(${ARGN})
        ENDIF(NOT LIBSHD_PKG AND NOT LIBSHDDEV_PKG AND NOT SHDHOST_PKG)
    ENDIF(SHD_INSTALL_COMPONENT STREQUAL "headers")
ENDFUNCTION(SHD_INSTALL)

########################################################################
# Print the registered component summary
########################################################################
FUNCTION(SHD_PRINT_COMPONENT_SUMMARY)
    MESSAGE(STATUS "")
    MESSAGE(STATUS "######################################################")
    MESSAGE(STATUS "# SHD enabled components                              ")
    MESSAGE(STATUS "######################################################")
    FOREACH(comp ${_shd_enabled_components})
        MESSAGE(STATUS "  * ${comp}")
    ENDFOREACH(comp)

    MESSAGE(STATUS "")
    MESSAGE(STATUS "######################################################")
    MESSAGE(STATUS "# SHD disabled components                             ")
    MESSAGE(STATUS "######################################################")
    FOREACH(comp ${_shd_disabled_components})
        MESSAGE(STATUS "  * ${comp}")
    ENDFOREACH(comp)

    MESSAGE(STATUS "")
ENDFUNCTION(SHD_PRINT_COMPONENT_SUMMARY)
