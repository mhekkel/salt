# SPDX-License-Identifier: BSD-2-Clause

# Copyright (c) 2022 Maarten L. Hekkelman

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:

# 1. Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer
# 2. Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following disclaimer in the documentation
# and/or other materials provided with the distribution.

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

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(USE_BOOST_ASIO "Use the asio library from Boost instead of the non-boost version" ON)

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
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-multichar")
elseif(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
endif()

if(MSVC)
	# make msvc standards compliant...
	add_compile_options(/permissive-)

	macro(get_WIN32_WINNT version)
		if(CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REGEX MATCH "^([0-9]+).([0-9])" ver ${ver})
			string(REGEX MATCH "^([0-9]+)" verMajor ${ver})

			# Check for Windows 10, b/c we'll need to convert to hex 'A'.
			if("${verMajor}" MATCHES "10")
				set(verMajor "A")
				string(REGEX REPLACE "^([0-9]+)" ${verMajor} ver ${ver})
			endif()

			# Remove all remaining '.' characters.
			string(REPLACE "." "" ver ${ver})

			# Prepend each digit with a zero.
			string(REGEX REPLACE "([0-9A-Z])" "0\\1" ver ${ver})
			set(${version} "0x${ver}")
		endif()
	endmacro()

	get_WIN32_WINNT(ver)
	add_definitions(-D_WIN32_WINNT=${ver})
endif()

add_compile_definitions(BOOST_ASIO_ENABLE_HANDLER_TRACKING)

if(USE_BOOST_ASIO)
	add_compile_definitions(USE_BOOST_ASIO)
endif()

# Create a revision file, containing the current git version info
include(VersionString)
write_version_header(${PROJECT_SOURCE_DIR}/src/)

# Optionally use mrc to create resources
find_package(Mrc QUIET)

if(MRC_FOUND)
	option(USE_RSRC "Use mrc to create resources" ON)
else()
	message(FATAL_ERROR "Cannot continue since mrc was not found")
endif()

set(CMAKE_THREAD_PREFER_PTHREAD)
set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

find_package(Filesystem REQUIRED)

# find_package(pinch 1.1 REQUIRED)
add_subdirectory(libpinch EXCLUDE_FROM_ALL)
add_subdirectory(libmcfp EXCLUDE_FROM_ALL)
add_subdirectory(libmgui EXCLUDE_FROM_ALL)

find_package(zeep 5.1.6 REQUIRED)

find_library(DL dl REQUIRED)
find_library(UTIL util REQUIRED)

# find_package(Boost COMPONENTS coroutines)
pkg_check_modules(GTK3 libcanberra-gtk3 REQUIRED IMPORTED_TARGET)

# The salt executable
add_compile_definitions(GDK_DISABLE_DEPRECATED GTK_DISABLE_DEPRECATED)

add_executable(salt
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

target_include_directories(salt PRIVATE ${CMAKE_BINARY_DIR} ${PROJECT_SOURCE_DIR}/lib)
target_link_libraries(salt zeep::zeep pinch::pinch libmcfp::libmcfp Threads::Threads mgui::mgui dl util)

mrc_target_resources(salt ${RESOURCES})

install(TARGETS salt
	RUNTIME DESTINATION ${BIN_INSTALL_DIR}
)
