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

cmake_minimum_required(VERSION 3.15)
project(nb-dashcam-tools LANGUAGES CXX)

set(APP_VER_MAJOR 0)
set(APP_VER_MINOR 1)
set(APP_VER_PATCH 3)
set(APP_VER_STR "${APP_VER_MAJOR}.${APP_VER_MINOR}.${APP_VER_PATCH}")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Tools REQUIRED)
endif()

configure_file("${CMAKE_SOURCE_DIR}/src/main.cpp" "${CMAKE_BINARY_DIR}/main.cpp" @ONLY)

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
  "${CMAKE_BINARY_DIR}/main.cpp"
  src/mainwindow.cpp
  src/mainwindow.hpp
  src/mp4file.cpp
  src/mp4file.hpp
  src/toollocator.cpp
  src/toollocator.hpp
)

if (WIN32)
  configure_file("${CMAKE_SOURCE_DIR}/src/version.rc.in" "${CMAKE_BINARY_DIR}/version.rc" @ONLY)
  list(APPEND PROJECT_SOURCES "${CMAKE_BINARY_DIR}/version.rc")
endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(nb-dashcam-tools
    ${PROJECT_SOURCES}
  )
else()
  add_executable(nb-dashcam-tools
    ${PROJECT_SOURCES}
  )
endif()

target_include_directories(nb-dashcam-tools PRIVATE src)
target_compile_definitions(nb-dashcam-tools PRIVATE QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_definitions(nb-dashcam-tools PRIVATE DEBUG)
endif()
set_source_files_properties(src/main.cpp PROPERTIES COMPILE_DEFINITIONS "APP_VER_STR=\"${APP_VER_STR}\"")

if(CMAKE_COMPILER_IS_GNUCXX)
  target_compile_options(nb-dashcam-tools PRIVATE -Werror -Wall -Wextra)
endif()

target_link_libraries(nb-dashcam-tools PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set_property(TARGET nb-dashcam-tools PROPERTY WIN32_EXECUTABLE true)
endif()

if (WIN32)
  install(TARGETS nb-dashcam-tools RUNTIME DESTINATION ".")
  
  if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    get_target_property(WINDEPLOYQT_BIN Qt${QT_VERSION_MAJOR}::windeployqt IMPORTED_LOCATION)
    install(CODE "execute_process(COMMAND \"${WINDEPLOYQT_BIN}\" \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_FILE_NAME:nb-dashcam-tools>\")")
  endif()
  
  set(WIN_INSTALL_FFMPEG OFF CACHE BOOL "Install ffmpeg on Windows")
  if (WIN_INSTALL_FFMPEG)
    find_program(7Z_EXE "7z")
    file(DOWNLOAD "https://www.gyan.dev/ffmpeg/builds/release-version" "${CMAKE_BINARY_DIR}/ffmpeg-version.txt")
    file(READ "${CMAKE_BINARY_DIR}/ffmpeg-version.txt" FFMPEG_VERSION)
    set(FFMPEG_URL "https://github.com/GyanD/codexffmpeg/releases/download/${FFMPEG_VERSION}/ffmpeg-${FFMPEG_VERSION}-essentials_build.7z")
    message(STATUS "FFMPEG_URL=${FFMPEG_URL}")
    set(FFMPEG_7Z_FILE "${CMAKE_BINARY_DIR}/ffmpeg.7z")
    install(CODE "if(NOT EXISTS \"${FFMPEG_7Z_FILE}\")\n    file(DOWNLOAD \"${FFMPEG_URL}\" \"${FFMPEG_7Z_FILE}\" SHOW_PROGRESS)\n  endif()")
    install(CODE "execute_process(COMMAND \"${7Z_EXE}\" e -r -y \"-o\${CMAKE_INSTALL_PREFIX}\" \"${CMAKE_BINARY_DIR}/ffmpeg.7z\" ffmpeg.exe)")
  endif()
  
  set(WIN_CREATE_SETUP OFF CACHE BOOL "Create setup program")
  if (WIN_CREATE_SETUP)
    find_program(ISCC_EXE "ISCC" PATHS "$ENV{ProgramFiles\(x86\)}/Inno Setup 6")
    install(CODE "execute_process(COMMAND \"${ISCC_EXE}\" /DMyAppVersion=${APP_VER_STR} nb-dashcam-tools.iss)")
  endif()

else()
  install(TARGETS nb-dashcam-tools)
endif()

