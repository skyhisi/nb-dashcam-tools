/* Copyright 2021 Silas Parker.
 *
 * This file is part of NB Dashcam Tools.
 *
 * NB Dashcam Tools is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * NB Dashcam Tools is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NB Dashcam Tools. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef GPSSAMPLEPARSER_HPP
#define GPSSAMPLEPARSER_HPP

#include <QIODevice>
#include <QDateTime>
#include <QRegularExpression>

struct GpsSample
{
    QDateTime datetime;
    bool gpsValid;
    float latitude; // + N / - S
    float longitude; // + E / - W
    float speed; // Meters/sec
    float bearing; // Degrees from true north
    float xAcc;
    float yAcc;
    float zAcc;
    int sats;
    float hdop;
    float altitude;
    float geoidheight;

    void reset();
};

class GpsSampleParser
{
public:
    static bool isCameraSupported(const QString& name);

    GpsSampleParser(QIODevice* device, const QString& name);
    bool isValid() const;

    bool nextSample(GpsSample* sample);

private:
    static int cameraFormat(const char* name);
    quint16 readUint16BE(bool* ok);
    qint16 readInt16LE(bool* ok);
    qint32 readInt32LE(bool* ok);

    bool parseSample322GW(GpsSample* sample, quint16 length);
    bool parseSample622GW(GpsSample* sample, quint16 length);

    bool parseGprmc(const QString& line, GpsSample* sample);
    bool parseGpgga(const QString& line, GpsSample* sample);
    bool parseCoord(const QString& part, float& output);

    QIODevice* mDevice;
    int mFormatCode;
    QRegularExpression mCoordRegEx;
};

#endif // GPSSAMPLEPARSER_HPP
