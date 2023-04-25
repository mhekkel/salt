# SPDX-License-Identifier: BSD-2-Clause

# Copyright (c) 2023 Maarten L. Hekkelman

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
project(libmgui VERSION 1.0.0 LANGUAGES C CXX)

include(GNUInstallDirs)
include(CheckFunctionExists)
include(CheckIncludeFiles)
include(CheckLibraryExists)
include(CMakePackageConfigHelpers)
include(GenerateExportHeader)

set(CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
		if (WIN32 AND CMAKE_SYSTEM_VERSION)
			set(ver ${CMAKE_SYSTEM_VERSION})
			string(REPLACE "." "" ver ${ver})
			string(REGEX REPLACE "([0-9])" "0\\1" ver ${ver})

			set(${version} "0x${ver}")
		endif()
	endmacro()

	get_WIN32_WINNT(ver)
	add_definitions(-D_WIN32_WINNT=${ver})
endif()

find_package(Mrc REQUIRED)
message("Using resources compiled with ${MRC_EXECUTABLE}")
mrc_write_header(${PROJECT_SOURCE_DIR}/include/mrsrc.hpp)

set(CMAKE_THREAD_PREFER_PTHREAD)
set(THREADS_PREFER_PTHREAD_FLAG)
find_package(Threads REQUIRED)

find_package(Filesystem REQUIRED)
find_package(zeep 5.1.6 REQUIRED)

# find_library(DL dl REQUIRED)
# find_library(UTIL util REQUIRED)

pkg_check_modules(GTK3 libcanberra-gtk3 REQUIRED IMPORTED_TARGET)

# The salt executable

list(APPEND headers
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkCanvasImpl.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkControlsImpl.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkDeviceImpl.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkLib.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkList.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkWidgetMixin.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkWindowImpl.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MPrimary.hpp
	${PROJECT_SOURCE_DIR}/include/Gtk/MGtkApplicationImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MAcceleratorTable.hpp
	${PROJECT_SOURCE_DIR}/include/MAnimation.hpp
	${PROJECT_SOURCE_DIR}/include/MTypes.hpp
	${PROJECT_SOURCE_DIR}/include/MApplication.hpp
	${PROJECT_SOURCE_DIR}/include/MApplicationImpl.hpp
	${PROJECT_SOURCE_DIR}/include/mrsrc.hpp
	${PROJECT_SOURCE_DIR}/include/MCanvas.hpp
	${PROJECT_SOURCE_DIR}/include/MCanvasImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MClipboard.hpp
	${PROJECT_SOURCE_DIR}/include/MClipboardImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MColorPicker.hpp
	${PROJECT_SOURCE_DIR}/include/MCommands.hpp
	${PROJECT_SOURCE_DIR}/include/MUnicode.hpp
	${PROJECT_SOURCE_DIR}/include/MController.hpp
	${PROJECT_SOURCE_DIR}/include/MView.hpp
	${PROJECT_SOURCE_DIR}/include/MDevice.hpp
	${PROJECT_SOURCE_DIR}/include/MDialog.hpp
	${PROJECT_SOURCE_DIR}/include/MDocApplication.hpp
	${PROJECT_SOURCE_DIR}/include/MDocClosedNotifier.hpp
	${PROJECT_SOURCE_DIR}/include/MDocWindow.hpp
	${PROJECT_SOURCE_DIR}/include/MError.hpp
	${PROJECT_SOURCE_DIR}/include/MExploreBrowserView.hpp
	${PROJECT_SOURCE_DIR}/include/MFile.hpp
	${PROJECT_SOURCE_DIR}/include/MFileImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MGfxDevice.hpp
	${PROJECT_SOURCE_DIR}/include/MLib.hpp
	${PROJECT_SOURCE_DIR}/include/MWindowImpl.hpp
	# ${PROJECT_SOURCE_DIR}/include/MListViewImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MMenu.hpp
	${PROJECT_SOURCE_DIR}/include/MP2PEvents.hpp
	${PROJECT_SOURCE_DIR}/include/MPreferences.hpp
	${PROJECT_SOURCE_DIR}/include/MPrinter.hpp
	${PROJECT_SOURCE_DIR}/include/MSound.hpp
	${PROJECT_SOURCE_DIR}/include/MStrings.hpp
	${PROJECT_SOURCE_DIR}/include/MUnicodeTables.hpp
	${PROJECT_SOURCE_DIR}/include/MUtils.hpp
	${PROJECT_SOURCE_DIR}/include/MWindow.hpp

	${PROJECT_SOURCE_DIR}/include/MHandler.hpp
	${PROJECT_SOURCE_DIR}/include/MAlerts.hpp
	${PROJECT_SOURCE_DIR}/include/MAnimationImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MColor.hpp
	${PROJECT_SOURCE_DIR}/include/MControls.hpp
	${PROJECT_SOURCE_DIR}/include/MControlsImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MDeviceImpl.hpp
	${PROJECT_SOURCE_DIR}/include/MDocument.hpp
	${PROJECT_SOURCE_DIR}/include/MMenuImpl.hpp)


list(APPEND sources
	${PROJECT_SOURCE_DIR}/src/bsd-closefrom.c

	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkAlerts.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkAnimationImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkClipboardImpl.cpp
	# ${PROJECT_SOURCE_DIR}/src/Gtk/MGtkDocClosedNotifier.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkError.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkFile.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkList.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkMenu.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkSound.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkUtils.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MPrimary.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkCanvasImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkControlsImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkDeviceImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkDialogImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkWidgetMixin.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkApplicationImpl.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkMain.cpp
	${PROJECT_SOURCE_DIR}/src/Gtk/MGtkWindowImpl.cpp
	${PROJECT_SOURCE_DIR}/src/MAcceleratorTable.cpp
	${PROJECT_SOURCE_DIR}/src/MAnimation.cpp
	${PROJECT_SOURCE_DIR}/src/MMenu.cpp
	${PROJECT_SOURCE_DIR}/src/MCanvas.cpp
	${PROJECT_SOURCE_DIR}/src/MClipboard.cpp
	${PROJECT_SOURCE_DIR}/src/MColorPicker.cpp
	${PROJECT_SOURCE_DIR}/src/MFile.cpp
	${PROJECT_SOURCE_DIR}/src/MDevice.cpp
	${PROJECT_SOURCE_DIR}/src/MDialog.cpp
	${PROJECT_SOURCE_DIR}/src/MDocApplication.cpp
	${PROJECT_SOURCE_DIR}/src/MDocClosedNotifier.cpp
	${PROJECT_SOURCE_DIR}/src/MWindow.cpp
	${PROJECT_SOURCE_DIR}/src/MError.cpp
	${PROJECT_SOURCE_DIR}/src/MGfxDevice.cpp
	${PROJECT_SOURCE_DIR}/src/MApplication.cpp
	${PROJECT_SOURCE_DIR}/src/MLib.cpp
	${PROJECT_SOURCE_DIR}/src/MPreferences.cpp
	${PROJECT_SOURCE_DIR}/src/MPrinter.cpp
	${PROJECT_SOURCE_DIR}/src/MStrings.cpp
	${PROJECT_SOURCE_DIR}/src/MTypes.cpp
	${PROJECT_SOURCE_DIR}/src/MUnicode.cpp
	${PROJECT_SOURCE_DIR}/src/MUtils.cpp
	${PROJECT_SOURCE_DIR}/src/MView.cpp

	${PROJECT_SOURCE_DIR}/src/MHandler.cpp
	${PROJECT_SOURCE_DIR}/src/MAlerts.cpp
	${PROJECT_SOURCE_DIR}/src/MAnimationImpl.cpp
	${PROJECT_SOURCE_DIR}/src/MColor.cpp
	${PROJECT_SOURCE_DIR}/src/MController.cpp
	${PROJECT_SOURCE_DIR}/src/MControls.cpp
	${PROJECT_SOURCE_DIR}/src/MDocWindow.cpp
	${PROJECT_SOURCE_DIR}/src/MDocument.cpp)

add_library(mgui ${sources} ${headers})
add_library(mgui::mgui ALIAS mgui)

target_include_directories(mgui PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(mgui libmcfp::libmcfp Threads::Threads PkgConfig::GTK3 zeep::zeep)

# install(TARGETS salt
#     RUNTIME DESTINATION ${BIN_INSTALL_DIR}
# )
