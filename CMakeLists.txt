# Copyright 2021 Silas Parker.
#
# This file is part of NB Dashcam Tools.
#
# NB Dashcam Tools is free software: you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option)
# any later version.
#
# NB Dashcam Tools is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License
# along with NB Dashcam Tools. If not, see <https://www.gnu.org/licenses/>.

cmake_minimum_required(VERSION 3.5)

project(nb-dashcam-tools LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check https://doc.qt.io/qt/deployment-android.html for more information.
# They need to be set before the find_package( ...) calls below.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

set(PROJECT_SOURCES
        src/clipmergewidget.cpp
        src/clipmergewidget.hpp
        src/clipmergewidget.ui
        src/gpsexport.cpp
        src/gpsexport.hpp
        src/gpsexportwidget.cpp
        src/gpsexportwidget.hpp
        src/gpsexportwidget.ui
        src/gpssampleparser.cpp
        src/gpssampleparser.hpp
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.hpp
        src/mp4file.cpp
        src/mp4file.hpp
        src/toollocator.cpp
        src/toollocator.hpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(nb-dashcam-tools
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(nb-dashcam-tools SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(nb-dashcam-tools
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_compile_definitions(nb-dashcam-tools PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
target_compile_options(nb-dashcam-tools PRIVATE -Werror -Wall -Wextra)
target_link_libraries(nb-dashcam-tools PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set_property(TARGET nb-dashcam-tools PROPERTY WIN32_EXECUTABLE true)
endif()
