# SPDX-License-Identifier: BSD-2-Clause

# Copyright (c) 2022 Maarten L. Hekkelman

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 3.15)

# set the project name
project(salt VERSION 4.0.0 LANGUAGES C CXX)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(GNUInstallDirs)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CMakePackageConfigHelpers)
include(Dart)
include(FindFilesystem)
include(FindPkgConfig)
include(GenerateExportHeader)

set(CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    if(WIN32)
		set(pinch_compile_flags "/clang:-fcoroutines-ts")
	else()
		set(pinch_compile_flags "-fcoroutines-ts")
	endif()
elseif(MSVC)
    check_include_file_cxx("coroutine" has_coroutine "/std:c++latest")
    if(NOT has_coroutine)
        message(STATUS "Try <experimental/coroutine> (Coroutines TS) instead of <coroutine> ...")
        check_include_file_cxx("experimental/coroutine" has_coroutine_ts "/std:c++17")
    endif()

	# has coroutine headers?
	if(has_coroutine)
		set(pinch_compile_flags "/std:c++latest")
	elseif(has_coroutine_ts)
		set(pinch_compile_flags "/std:c++17")
	else()
        message(FATAL_ERROR "There are no headers for C++ Coroutines")
    endif()
elseif(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    #
    # expect GCC 10 or later
    #
    check_include_file_cxx("coroutine" has_coroutine "-fcoroutines")
	if(has_coroutine)
		set(pinch_compile_flags "-fcoroutines")
	endif()
endif()

# support compiler options for coroutine?
check_cxx_compiler_flag("${pinch_compile_flags}" support_coroutines)
if(NOT support_coroutines)
    message(FATAL_ERROR "The compiler doesn't support C++ Coroutines")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${pinch_compile_flags}")
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers -Wno-multichar")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

if(MSVC)
    # make msvc standards compliant...
    add_compile_options(/permissive-)

	macro(get_WIN32_WINNT version)
		if (WIN32 AND CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REPLACE "." "" ver ${ver})
			string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

			set(${version} "0x${ver}")
		endif()
	endmacro()

	get_WIN32_WINNT(ver)
	add_definitions(-D_WIN32_WINNT=${ver})

	# On Windows, do not install in the system location
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT AND NOT ($ENV{LOCALAPPDATA} STREQUAL ""))
		message(STATUS "The executable will be installed in $ENV{LOCALAPPDATA}")
		set(CMAKE_INSTALL_PREFIX "$ENV{LOCALAPPDATA}" CACHE PATH "..." FORCE)
	endif()
endif()

if(UNIX AND NOT APPLE)
	# On Linux, install in the $HOME/.local folder by default
	if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
		message(STATUS "The library and auxiliary files will be installed in $ENV{HOME}/.local")
		set(CMAKE_INSTALL_PREFIX "$ENV{HOME}/.local" CACHE PATH "..." FORCE)
	endif()
endif()

# Create a revision file, containing the current git version info

include(GetGitRevisionDescription)
option(GENERATE_CUSTOM_VERSION "Generate a custom version string")

if(NOT GENERATE_CUSTOM_VERSION OR GIT-NOTFOUND OR HEAD-HASH-NOTFOUND)
	SET(BUILD_VERSION_STRING ${PROJECT_VERSION})
else()
	get_git_head_revision(REFSPEC COMMITHASH)

	# Generate our own version string
	git_describe_working_tree(BUILD_VERSION_STRING --match=build --dirty)
endif()

# generate version.h
string(TIMESTAMP BUILD_DATE_TIME "%Y-%m-%dT%H:%M:%SZ" UTC)
configure_file("${CMAKE_SOURCE_DIR}/src/revision.hpp.in" "${CMAKE_BINARY_DIR}/revision.hpp" @ONLY)

# Optionally use mrc to create resources
find_package(Mrc)

if(MRC_FOUND)
	option(USE_RSRC "Use mrc to create resources" ON)
else()
	message(STATUS "Not using resources since mrc was not found")
endif()

if(USE_RSRC)
	set(USE_RSRC 1)

	message("Using resources compiled with ${MRC_EXECUTABLE}")
	add_compile_definitions(USE_RSRC)

	mrc_write_header(${CMAKE_BINARY_DIR}/mrsrc.hpp)
endif()

set(CMAKE_THREAD_PREFER_PTHREAD)
set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

find_package(Filesystem REQUIRED)

find_package(pinch)
find_package(zeep 5.1.6 REQUIRED)

find_library(DL dl)
find_library(UTIL util)

# set(Boost_DETAILED_FAILURE_MSG ON)

# find_package(Boost COMPONENTS coroutines)

pkg_check_modules(GTK3 libcanberra-gtk3 REQUIRED)
# pkg_check_modules(PQXX libpqxx>=7.2 REQUIRED)
# pkg_check_modules(MAILIO mailio REQUIRED)
# pkg_check_modules(SSL libssl)
# pkg_check_modules(CRYPTO libcrypto)
# pkg_check_modules(JPEG libjpeg REQUIRED)
# pkg_check_modules(PNG libpng REQUIRED)
# pkg_check_modules(TIDY tidy REQUIRED)
# pkg_check_modules(ZLIB zlib)

# The salt executable

add_executable(salt
	${CMAKE_SOURCE_DIR}/lib/bsd-closefrom.c

	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkAlerts.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkAnimationImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkCanvasImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkClipboardImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkControlsImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkDeviceImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkDocClosedNotifier.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkError.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkFile.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkLib.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkList.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkList.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkMenu.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkSound.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkUtils.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkWidgetMixin.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkWindowImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MPrimary.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MPrimary.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkCanvasImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkControlsImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkDeviceImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkDialogImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkWidgetMixin.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkApplicationImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkApplicationImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkMain.cpp
	${CMAKE_SOURCE_DIR}/lib/Gtk/MGtkWindowImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/MAcceleratorTable.cpp
	${CMAKE_SOURCE_DIR}/lib/MAcceleratorTable.hpp
	${CMAKE_SOURCE_DIR}/lib/MAnimation.cpp
	${CMAKE_SOURCE_DIR}/lib/MAnimation.hpp
	${CMAKE_SOURCE_DIR}/lib/MTypes.hpp
	${CMAKE_SOURCE_DIR}/lib/MApplication.hpp
	${CMAKE_SOURCE_DIR}/lib/MApplicationImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MMenu.cpp
	${CMAKE_SOURCE_DIR}/lib/mrsrc.hpp
	${CMAKE_SOURCE_DIR}/lib/MCanvas.cpp
	${CMAKE_SOURCE_DIR}/lib/MCanvas.hpp
	${CMAKE_SOURCE_DIR}/lib/MCanvasImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MClipboard.cpp
	${CMAKE_SOURCE_DIR}/lib/MClipboard.hpp
	${CMAKE_SOURCE_DIR}/lib/MClipboardImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MColorPicker.cpp
	${CMAKE_SOURCE_DIR}/lib/MColorPicker.hpp
	${CMAKE_SOURCE_DIR}/lib/MCommands.hpp
	${CMAKE_SOURCE_DIR}/lib/MFile.cpp
	${CMAKE_SOURCE_DIR}/lib/MUnicode.hpp
	${CMAKE_SOURCE_DIR}/lib/MController.hpp
	${CMAKE_SOURCE_DIR}/lib/MView.hpp
	${CMAKE_SOURCE_DIR}/lib/MDevice.cpp
	${CMAKE_SOURCE_DIR}/lib/MDevice.hpp
	${CMAKE_SOURCE_DIR}/lib/MDialog.cpp
	${CMAKE_SOURCE_DIR}/lib/MDialog.hpp
	${CMAKE_SOURCE_DIR}/lib/MDocApplication.cpp
	${CMAKE_SOURCE_DIR}/lib/MDocApplication.hpp
	${CMAKE_SOURCE_DIR}/lib/MDocClosedNotifier.cpp
	${CMAKE_SOURCE_DIR}/lib/MDocClosedNotifier.hpp
	${CMAKE_SOURCE_DIR}/lib/MDocWindow.hpp
	${CMAKE_SOURCE_DIR}/lib/MWindow.cpp
	${CMAKE_SOURCE_DIR}/lib/MError.cpp
	${CMAKE_SOURCE_DIR}/lib/MError.hpp
	${CMAKE_SOURCE_DIR}/lib/MExploreBrowserView.hpp
	${CMAKE_SOURCE_DIR}/lib/MFile.hpp
	${CMAKE_SOURCE_DIR}/lib/MFileImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MGfxDevice.cpp
	${CMAKE_SOURCE_DIR}/lib/MGfxDevice.hpp
	${CMAKE_SOURCE_DIR}/lib/MApplication.cpp
	${CMAKE_SOURCE_DIR}/lib/MLib.cpp
	${CMAKE_SOURCE_DIR}/lib/MLib.hpp
	${CMAKE_SOURCE_DIR}/lib/MWindowImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/MListViewImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MMenu.hpp
	${CMAKE_SOURCE_DIR}/lib/MP2PEvents.hpp
	${CMAKE_SOURCE_DIR}/lib/MPreferences.cpp
	${CMAKE_SOURCE_DIR}/lib/MPreferences.hpp
	${CMAKE_SOURCE_DIR}/lib/MPrinter.cpp
	${CMAKE_SOURCE_DIR}/lib/MPrinter.hpp
	${CMAKE_SOURCE_DIR}/lib/MSound.hpp
	${CMAKE_SOURCE_DIR}/lib/MStrings.cpp
	${CMAKE_SOURCE_DIR}/lib/MStrings.hpp
	${CMAKE_SOURCE_DIR}/lib/MTypes.cpp
	${CMAKE_SOURCE_DIR}/lib/MUnicode.cpp
	${CMAKE_SOURCE_DIR}/lib/MUnicodeTables.hpp
	${CMAKE_SOURCE_DIR}/lib/MUtils.cpp
	${CMAKE_SOURCE_DIR}/lib/MUtils.hpp
	${CMAKE_SOURCE_DIR}/lib/MView.cpp
	${CMAKE_SOURCE_DIR}/lib/MWindow.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinApplicationImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinCanvasImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinExploreBrowserView.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinExploreBrowserView.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinFile.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinLib.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinListViewImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinListViewImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinMenuImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinPreferences.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinSound.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinUtils.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinWindowImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinAlerts.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinAnimationImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinApplicationImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinCanvasImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinClipboardImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinControlsImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinControlsImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinDeviceImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinDialogImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinLib.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinMain.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinMenuImpl.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinProcMixin.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinResources.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinUtils.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinWindowImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/comptr.hpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinDeviceImpl.cpp
	# ${CMAKE_SOURCE_DIR}/lib/Windows/MWinProcMixin.cpp
	# ${CMAKE_SOURCE_DIR}/lib/MListView.hpp
	# ${CMAKE_SOURCE_DIR}/lib/MListView.cpp
	${CMAKE_SOURCE_DIR}/lib/MHandler.cpp
	${CMAKE_SOURCE_DIR}/lib/MHandler.hpp
	${CMAKE_SOURCE_DIR}/lib/MAlerts.cpp
	${CMAKE_SOURCE_DIR}/lib/MAlerts.hpp
	${CMAKE_SOURCE_DIR}/lib/MAnimationImpl.cpp
	${CMAKE_SOURCE_DIR}/lib/MAnimationImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MColor.cpp
	${CMAKE_SOURCE_DIR}/lib/MColor.hpp
	${CMAKE_SOURCE_DIR}/lib/MController.cpp
	${CMAKE_SOURCE_DIR}/lib/MControls.cpp
	${CMAKE_SOURCE_DIR}/lib/MControls.hpp
	${CMAKE_SOURCE_DIR}/lib/MControlsImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MDeviceImpl.hpp
	${CMAKE_SOURCE_DIR}/lib/MDocWindow.cpp
	${CMAKE_SOURCE_DIR}/lib/MDocument.cpp
	${CMAKE_SOURCE_DIR}/lib/MDocument.hpp
	${CMAKE_SOURCE_DIR}/lib/MMenuImpl.hpp

	${CMAKE_SOURCE_DIR}/src/MAddTOTPHashDialog.cpp
	${CMAKE_SOURCE_DIR}/src/MAddTOTPHashDialog.hpp
	${CMAKE_SOURCE_DIR}/src/MAuthDialog.cpp
	${CMAKE_SOURCE_DIR}/src/MAuthDialog.hpp
	${CMAKE_SOURCE_DIR}/src/MCSICommands.hpp
	${CMAKE_SOURCE_DIR}/src/MConnectDialog.cpp
	${CMAKE_SOURCE_DIR}/src/MConnectDialog.hpp
	# ${CMAKE_SOURCE_DIR}/src/MExploreBrowserWindow.cpp
	# ${CMAKE_SOURCE_DIR}/src/MExploreBrowserWindow.hpp
	${CMAKE_SOURCE_DIR}/src/MHTTPProxy.hpp
	# ${CMAKE_SOURCE_DIR}/src/MLibTestApp.cpp
	# ${CMAKE_SOURCE_DIR}/src/MLibTestApp.hpp
	${CMAKE_SOURCE_DIR}/src/MPortForwardingDialog.cpp
	${CMAKE_SOURCE_DIR}/src/MPortForwardingDialog.hpp
	${CMAKE_SOURCE_DIR}/src/MPreferencesDialog.hpp
	${CMAKE_SOURCE_DIR}/src/MSalt.hpp
	${CMAKE_SOURCE_DIR}/src/MSaltVersion.h.tmpl
	${CMAKE_SOURCE_DIR}/src/MSearchPanel.hpp
	${CMAKE_SOURCE_DIR}/src/MTerminalBuffer.hpp
	${CMAKE_SOURCE_DIR}/src/MTerminalChannel.hpp
	${CMAKE_SOURCE_DIR}/src/MTerminalColours.cpp
	${CMAKE_SOURCE_DIR}/src/MTerminalColours.hpp
	${CMAKE_SOURCE_DIR}/src/MTerminalView.hpp
	${CMAKE_SOURCE_DIR}/src/MVT220CharSets.hpp
	${CMAKE_SOURCE_DIR}/src/MPtyTerminalChannel.hpp
	${CMAKE_SOURCE_DIR}/src/MSearchPanel.cpp
	${CMAKE_SOURCE_DIR}/src/MTerminalBuffer.cpp
	${CMAKE_SOURCE_DIR}/src/MSaltApp.hpp
	${CMAKE_SOURCE_DIR}/src/MTerminalWindow.hpp
	${CMAKE_SOURCE_DIR}/src/MHTTPProxy.cpp
	${CMAKE_SOURCE_DIR}/src/MPtyTerminalChannel.cpp
	${CMAKE_SOURCE_DIR}/src/MTerminalChannel.cpp
	${CMAKE_SOURCE_DIR}/src/MTerminalView.cpp
	${CMAKE_SOURCE_DIR}/src/MTerminalWindow.cpp
	${CMAKE_SOURCE_DIR}/src/MPreferencesDialog.cpp
	${CMAKE_SOURCE_DIR}/src/MSaltApp.cpp)

set(RESOURCES
	${CMAKE_SOURCE_DIR}/rsrc/Menus
	${CMAKE_SOURCE_DIR}/rsrc/Alerts
	${CMAKE_SOURCE_DIR}/rsrc/close.png
	${CMAKE_SOURCE_DIR}/rsrc/version.rc
	${CMAKE_SOURCE_DIR}/rsrc/templates
	${CMAKE_SOURCE_DIR}/rsrc/css
	${CMAKE_SOURCE_DIR}/rsrc/salt.desktop
	${CMAKE_SOURCE_DIR}/rsrc/Dialogs
	${CMAKE_SOURCE_DIR}/rsrc/strings.xml.nl
	${CMAKE_SOURCE_DIR}/rsrc/MakeRes.pl
	${CMAKE_SOURCE_DIR}/rsrc/Icons)

target_include_directories(salt PRIVATE ${CMAKE_BINARY_DIR} ${GTK3_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/lib)
target_link_libraries(salt zeep::zeep pinch::pinch Threads::Threads ${GTK3_LIBRARIES} ${DL} ${UTIL})

if(USE_RSRC)
	mrc_target_resources(salt ${RESOURCES})
endif()

install(TARGETS salt
    RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)
