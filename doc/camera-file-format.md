# Dashcam Video File Format
Notes on the video files produced by Nextbase 322GW dashcam.
This was worked out by examining the MP4 files, this is not official documentation.
These notes assume some knowledge of the [ISO BMFF](https://en.wikipedia.org/wiki/ISO/IEC_base_media_file_format) ([ISO/IEC 14496-12:2015](https://standards.iso.org/ittf/PubliclyAvailableStandards/c068960_ISO_IEC_14496-12_2015.zip)) standard, some atoms are documented in a similar way.

There are two main parts of the video file that are non-standard, the extra header atoms within the `moov/udat` atom and the use of the subtitles stream for storing the GPS and accelerometer data.

## Header Atoms
There are several extra atoms added into the `moov/udat` atom.

 * `AMBA` - Unknown data
 * `info` - Camera model and firmware version string, ASCII, not null terminated
 * `time` - Time stamp
   ```
   class NbTimeBox extends Box('time') {
     unsigned int(32) timestamp; // Unix timestamp
     unsigned int(16) unknown1;
     unsigned int(16) unknown2;
   }
   ```
 * `nbpl` - Unknown, appears to be 9 bytes of 0x20 (ASCII space?)
 * `maca` - Unknown data
 * `sern` - Unknown data
 * `infi` - Information string containing lines of camera information separated by CRLF, ASCII, not null terminated
 * `finm` - Filename string, ASCII, not null terminated
 * `nbid` - Unknown data

## GPS and Accelerometer Data
The subtitle track contains a samples containing the GPS and accelerometer data.
Samples appear to be fixed length, although they include a size prefix.
GPS data is in [NMEA 0183](https://en.wikipedia.org/wiki/NMEA_0183) line format, but the checksum is non-standard (doesn't validate).

### 322GW Sample Format
```
class Nb322GWSample {
  unsigned int(16)             length;          // Length of following data (0x0120 = 288)
  unsigned int(32)             unknown;
  unsigned int(8)[14]          datetime_string; // String of date time, format "%Y%m%d%H%M%S"
  little_endian signed int(32) y_acceleration;  // Y Acceleration, g_force = value / -1280.0
  little_endian signed int(32) x_acceleration;  // X Acceleration, g_force = value / 1280.0
  little_endian signed int(32) z_acceleration;  // Z Acceleration, g_force = value / 1280.0
  unsigned int(8)[128]         gprmc_string;    // NMEA 0183 GPRMC string, ASCII, null padded 
  unsigned int(8)[128]         gpgga_string;    // NMEA 0183 GPGGA string, ASCII, null padded
}
```

## Inspection Tools
Tools useful for inspecting the MP4 files:
 * [FFmpeg](https://www.ffmpeg.org/) - Useful for extracting the subtitle tracks out of MP4 files:
   ```
   ffmpeg -i input.mp4 -map 0:s -c:s copy -f data output.dat
   ```
 * [Bento4](https://www.bento4.com/) - Tools and a C++ library for manipulating MP4
 * [Okteta](https://apps.kde.org/en-gb/okteta/) - GUI Hex editor
 

