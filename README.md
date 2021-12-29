# NB Dashcam Tools

[![Build](https://github.com/skyhisi/nb-dashcam-tools/actions/workflows/build.yml/badge.svg)](https://github.com/skyhisi/nb-dashcam-tools/actions/workflows/build.yml)
![GitHub](https://img.shields.io/github/license/skyhisi/nb-dashcam-tools?color=green)
![Lines of code](https://img.shields.io/tokei/lines/github/skyhisi/nb-dashcam-tools)


Tools for merging clips and extracting GPS data from Nextbase dash cam video
files.

This product is not developed by or associated with Nextbase or
Portable Multimedia Limited.

![Screenshot](doc/images/screenshot-merge.png?raw=true)


## Features

 * Very fast merging of files in video copy mode - avoids re-encoding video
   while merging
 * Can merge and re-encode video files with a customisable compression level
 * Re-encoding can use an NVidia graphics card for fast re-encode
 * Extracting GPS data to a standard GPX file
 * Extracting GPS and accelerometer data to a CSV file


## Camera Compatibility

Let me know if you would like support for other cameras, or if you can help
with testing with other cameras.
To add support some sample clips may needed for testing.

Camera | Supported | Tested
-------|-----------|-------
322GW  | Yes       | Yes
422GW  | Maybe     | No
522GW  | Maybe     | No
622GW  | Maybe     | No


## Downloads
 * [Windows Installer](https://github.com/skyhisi/nb-dashcam-tools/releases/latest/download/nb-dashcam-tools-setup.exe)
 * [Linux Packages and Source](https://github.com/skyhisi/nb-dashcam-tools/releases/latest)

## Building
### Requirements
 * Qt 5 or Qt 6
 * CMake
 * C++ Compiler Tool chain
 * FFmpeg Executable (ffmpeg is needed only at runtime)

### General Steps for Linux

```sh
git clone https://github.com/skyhisi/nb-dashcam-tools.git
cd nb-dashcam-tools
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build .
./nb-dashcam-tools
```

## Documents
 * [Dashcam Video File Format](doc/camera-file-format.md)

