# ***** BEGIN LICENSE BLOCK *****
# This file is part of Natron <https://natrongithub.github.io/>,
# (C) 2018-2023 The Natron developers
# (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
#
# Natron is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# Natron is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
# ***** END LICENSE BLOCK *****

set(Natron_SOURCES NatronApp_main.cpp)
if(WIN32)
    list(APPEND Natron_SOURCES ../Natron.rc)
endif()
add_executable(Natron ${Natron_SOURCES})
if(APPLE)
    set_target_properties(Natron PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "${PROJECT_NAME}"
        MACOSX_BUNDLE_INFO_STRING "${PROJECT_NAME} Copyright (c) 2023 The Natron Developers"
        MACOSX_BUNDLE_ICON_FILE Icon.icns
        MACOSX_BUNDLE_GUI_IDENTIFIER "${PROJECT_NAME}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${PROJECT_VERSION}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright (c) 2023 The Natron Developers"
    )
endif()
target_link_libraries(Natron
    PRIVATE
        NatronGui
        Python3::Python
)
target_include_directories(Natron
    PRIVATE
        ..
        ../Gui
        ../Engine
        ../Global
)

if(NATRON_BUNDLE_RUNTIME)
    include(ExternalProject)

    # Natron loads bundled plug-ins from <bindir>/../Plugins/OFX/Natron (Engine/OfxHost.cpp) and
    # the OpenColorIO configs from <bindir>/../Resources (../share on Linux, Engine/Settings.cpp).
    if(APPLE)
        set(NATRON_RUNTIME_ROOT "${CMAKE_CURRENT_BINARY_DIR}/Natron.app/Contents")
        set(NATRON_OCIO_CONFIGS_DIR "${NATRON_RUNTIME_ROOT}/Resources/OpenColorIO-Configs")
    else()
        set(NATRON_RUNTIME_ROOT "${CMAKE_BINARY_DIR}")
        if(WIN32)
            set(NATRON_OCIO_CONFIGS_DIR "${NATRON_RUNTIME_ROOT}/Resources/OpenColorIO-Configs")
        else()
            set(NATRON_OCIO_CONFIGS_DIR "${NATRON_RUNTIME_ROOT}/share/OpenColorIO-Configs")
        endif()
    endif()
    set(NATRON_PLUGINS_DIR "${NATRON_RUNTIME_ROOT}/Plugins/OFX/Natron")

    # CMAKE_PREFIX_PATH is a list: pass it through with another separator
    string(REPLACE ";" "|" _prefix_path "${CMAKE_PREFIX_PATH}")

    set(_runtime_targets)
    foreach(_plugin ${NATRON_OFX_PLUGINS})
        string(TOUPPER "${_plugin}" _var)
        string(REPLACE "-" "_" _var "${_var}")
        set(NATRON_${_var}_SOURCE_DIR "" CACHE PATH
            "Local checkout of ${_plugin} to build instead of cloning it from NATRON_RUNTIME_GIT_URL")
        if(NATRON_${_var}_SOURCE_DIR)
            # rebuild on every build so that local changes are picked up (ninja keeps it incremental)
            set(_source SOURCE_DIR "${NATRON_${_var}_SOURCE_DIR}" BUILD_ALWAYS ON)
        else()
            set(_source
                GIT_REPOSITORY "${NATRON_RUNTIME_GIT_URL}/${_plugin}.git"
                GIT_TAG master
                GIT_SHALLOW ON
                GIT_SUBMODULES_RECURSE ON
                # don't fetch on every build; build the <plugin>-update target to update
                UPDATE_DISCONNECTED ON)
        endif()
        ExternalProject_Add(${_plugin}
            ${_source}
            LIST_SEPARATOR |
            CMAKE_ARGS
                "-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}"
                "-DCMAKE_PREFIX_PATH=${_prefix_path}"
                "-DCMAKE_INSTALL_PREFIX=${NATRON_PLUGINS_DIR}"
                "-DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
            USES_TERMINAL_BUILD ON)
        list(APPEND _runtime_targets ${_plugin})
    endforeach()

    ExternalProject_Add(OpenColorIO-Configs
        URL "${NATRON_RUNTIME_GIT_URL}/OpenColorIO-Configs/archive/Natron-v${NATRON_OCIO_CONFIGS_VERSION}.tar.gz"
        TLS_VERIFY ON
        DOWNLOAD_EXTRACT_TIMESTAMP ON
        CONFIGURE_COMMAND ""
        BUILD_COMMAND ""
        INSTALL_COMMAND "${CMAKE_COMMAND}" -E copy_directory <SOURCE_DIR> "${NATRON_OCIO_CONFIGS_DIR}")
    list(APPEND _runtime_targets OpenColorIO-Configs)

    add_custom_target(natron-runtime DEPENDS ${_runtime_targets})
    add_dependencies(Natron natron-runtime)
endif()

install(TARGETS Natron DESTINATION "${CMAKE_INSTALL_BINDIR}")
install(FILES ../Gui/Resources/Applications/fr.natron.Natron.desktop
    DESTINATION "${CMAKE_INSTALL_DATADIR}/applications")
install(FILES ../Gui/Resources/Metainfo/fr.natron.Natron.appdata.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/metainfo")
install(FILES ../Gui/Resources/Mime/x-natron.xml
    DESTINATION "${CMAKE_INSTALL_DATADIR}/mime")
install(FILES ../Gui/Resources/Images/natronIcon256_linux.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")
install(FILES ../Gui/Resources/Images/natronProjectIcon_linux.png
    DESTINATION "${CMAKE_INSTALL_DATADIR}/pixmaps")
