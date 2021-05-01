# NB Dashcam Tools
Tools for merging clips and extracting GPS data from Nextbase dash cam video
files.

This product is not developed by or associated with Nextbase or
Portable Multimedia Limited.

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
422GW  | Yes       | No
522GW  | Yes       | No
622GW  | Maybe     | No


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
