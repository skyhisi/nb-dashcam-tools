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

#include "gpssampleparser.hpp"

#include <QDebug>
#include <QTimeZone>


struct ParseFormat
{
    const char* name;
    int formatCode;
};

static const ParseFormat supportedCameras[] = {
    {"212G", -1},
    {"222G", -1},
    {"222GX", -1},
    {"312G", -1},
    {"312GW", -1},
    {"300W", -1},
    {"380GW", -1},
    {"380GWX", -1},
    {"412GW", -1},
    {"512GW", -1},
    {"612GW", -1},
    {"MIRGW", -1},
    {"DUOHD", -1},
    {"DUO", -1}, // Same as DUOHD
    {"402G", -1},
    {"RIDE", -1}, // Same as 402G
    {"512G", -1},
    {"322GW", 1},
    {"422GW", 1},
    {"522GW", 1},
    {"622GW", 2},
    {nullptr, 0}
};


void GpsSample::reset()
{
    datetime = QDateTime();
    gpsValid = false;
    latitude = longitude = speed = bearing = xAcc = yAcc = zAcc = hdop = altitude = geoidheight = qQNaN();
    sats = 0;
}


int GpsSampleParser::cameraFormat(const char* name)
{
    for (const ParseFormat* p = supportedCameras; p->name; ++p)
        if (qstrcmp(name, p->name) == 0)
            return p->formatCode;
    return 0;
}

bool GpsSampleParser::isCameraSupported(const QString& name)
{
    return cameraFormat(name.toLatin1().constData()) > 0;
}

GpsSampleParser::GpsSampleParser(QIODevice* device, const QString& name):
    mDevice(device),
    mFormatCode(cameraFormat(name.toLatin1().constData())),
    mCoordRegEx("^(\\d{0,3})(\\d\\d(?:\\.\\d*)?)$")
{
    Q_ASSERT(mCoordRegEx.isValid());
    mCoordRegEx.optimize();
}

bool GpsSampleParser::isValid() const
{
    return mDevice && mDevice->isOpen() && mDevice->isReadable() && (mFormatCode > 0);
}

bool GpsSampleParser::nextSample(GpsSample* sample)
{
    Q_ASSERT(sample != nullptr);
    sample->reset();

    if (mDevice->atEnd())
    {
        qDebug() << "At end, no more samples";
        return false;
    }

    bool ok;
    quint16 sampleLength = readUint16BE(&ok);
    if (!ok)
    {
        qDebug() << "Failed to read sample length";
        return false;
    }

    qint64 endpos = mDevice->pos() + sampleLength;
    if (endpos > mDevice->size())
    {
        qDebug() << "Not enough data remaining";
        return false;
    }
    if (sampleLength <= 4)
    {
        return mDevice->skip(sampleLength) == sampleLength;
    }

    if (mDevice->skip(4) != 4) // Not sure what this is, looks to be always zeros
    {
        qDebug() << "Failed to get to start of data";
        return false;
    }
    sampleLength -= 4;

    bool rc = false;
    switch (mFormatCode)
    {
    case 1:
        rc = parseSample322GW(sample, sampleLength);
        break;
    case 2:
        rc = parseSample622GW(sample, sampleLength);
        break;

    default:
        qDebug() << "Unknown format" << mFormatCode;
        break;
    }
    //qDebug() << mDevice->size() << mDevice->pos() << endpos << sampleLength;
    Q_ASSERT(mDevice->pos() == endpos);

    return rc;
}

quint16 GpsSampleParser::readUint16BE(bool* ok)
{
    quint16 rc = 0;
    unsigned char data[2];
    bool ok_ = (mDevice->read((char*)data, 2) == 2);
    if (ok)
        *ok = ok_;
    if (ok_)
    {
        rc = (data[0] << 8) + data[1];
    }
    return rc;
}

qint16 GpsSampleParser::readInt16LE(bool* ok)
{
    qint16 rc = 0;
    unsigned char data[2];
    bool ok_ = (mDevice->read((char*)data, 2) == 2);
    if (ok)
        *ok = ok_;
    if (ok_)
    {
        rc = (data[1] << 8) | data[0];
    }
    return rc;
}

qint32 GpsSampleParser::readInt32LE(bool* ok)
{
    qint32 rc = 0;
    unsigned char data[4];
    bool ok_ = (mDevice->read((char*)data, 4) == 4);
    if (ok)
        *ok = ok_;
    if (ok_)
    {
        rc = (data[3] << 24) | (data[2] << 16) | (data[1] << 8) | data[0];
    }
    return rc;
}


bool GpsSampleParser::parseSample322GW(GpsSample* sample, quint16 length)
{
    Q_ASSERT(length == 284);
    mDevice->skip(16);
    bool xOk, yOk, zOk;
    sample->yAcc = (readInt32LE(&yOk) / 1280.0f) * -1.0f;
    sample->xAcc = readInt32LE(&xOk) / 1280.0f;
    sample->zAcc = readInt32LE(&zOk) / 1280.0f;
    if (!(xOk && yOk && zOk))
    {
        qDebug() << "Failed to read acceleration data";
        return false;
    }

    char gprmc[129] = {0}; // 128 +1 for nul terminator
    char gpgga[129] = {0};
    if (mDevice->read(gprmc, 128) != 128)
    {
        qDebug() << "Failed to read GPRMC";
        return false;
    }
    if (mDevice->read(gpgga, 128) != 128)
    {
        qDebug() << "Failed to read GPGGA";
        return false;
    }

    QString gprmcStr(QLatin1String(gprmc).trimmed());
    QString gpggaStr(QLatin1String(gpgga).trimmed());

    if (!parseGprmc(gprmcStr, sample))
    {
        qDebug() << "Failed to parse GPRMC";
        return false;
    }
    if (!parseGpgga(gpggaStr, sample))
    {
        qDebug() << "Failed to parse GPGGA";
        return false;
    }

    return true;
}

// Untested
bool GpsSampleParser::parseSample622GW(GpsSample* sample, quint16 length)
{
    Q_ASSERT(length == 1042);
    mDevice->skip(24);
    bool xOk, yOk, zOk;
    sample->yAcc = (readInt16LE(&yOk) / 2048.0f) * -1.0f;
    sample->xAcc = readInt16LE(&xOk) / 2048.0f;
    sample->zAcc = readInt16LE(&zOk) / 2048.0f;
    if (!(xOk && yOk && zOk))
    {
        qDebug() << "Failed to read acceleration data";
        return false;
    }

    mDevice->skip(756);

    char gprmc[129] = {0}; // 128 +1 for nul terminator
    char gpgga[129] = {0};
    if (mDevice->read(gprmc, 128) != 128)
    {
        qDebug() << "Failed to read GPRMC";
        return false;
    }
    if (mDevice->read(gpgga, 128) != 128)
    {
        qDebug() << "Failed to read GPGGA";
        return false;
    }

    QString gprmcStr(QLatin1String(gprmc).trimmed());
    QString gpggaStr(QLatin1String(gpgga).trimmed());

    if (!parseGprmc(gprmcStr, sample))
    {
        qDebug() << "Failed to parse GPRMC";
        return false;
    }
    if (!parseGpgga(gpggaStr, sample))
    {
        qDebug() << "Failed to parse GPGGA";
        return false;
    }

    return true;
}


bool GpsSampleParser::parseGprmc(const QString& line, GpsSample* sample)
{
    //qDebug() << line;

    // Remove checksum, don't check it, never valid
    QString lineData = line.contains('*') ? line.section('*', 0, 0) : line;
    //qDebug() << lineData;

    QStringList parts = lineData.split(',');
    //qDebug() << parts;
    if (parts.size() != 13 || parts.at(0) != "$GPRMC")
    {
        qDebug() << "Failed to split GPRMC";
        return false;
    }

    if (parts.at(1).isEmpty() || parts.at(9).isEmpty())
        return true;

    QTime time(QTime::fromString(parts.at(1), "HHmmss.zzz"));
    if (!time.isValid())
    {
        qDebug() << "Time is invalid" << parts.at(1);
        return false;
    }

    QDate date(QDate::fromString(parts[9].insert(4, "20"), "ddMMyyyy"));
    if (!date.isValid())
    {
        qDebug() << "Date is invalid" << parts[9];
        return false;
    }

    sample->datetime = QDateTime(date, time, QTimeZone::utc());
    if (!sample->datetime.isValid())
    {
        qDebug() << "Date time is invalid";
        return false;
    }
    //qDebug() << sample->datetime;

    sample->gpsValid = (parts[2] == "A");

    if (sample->gpsValid)
    {
        if (!parseCoord(parts[3], sample->latitude))
        {
            qDebug() << "Failed to parse latitude";
            return false;
        }
        sample->latitude *= (parts[4] == "N") ? 1.0f : -1.0f;

        if (!parseCoord(parts[5], sample->longitude))
        {
            qDebug() << "Failed to parse longitude";
            return false;
        }
        sample->longitude *= (parts[6] == "E") ? 1.0f : -1.0f;


        sample->speed = parts[7].toFloat() * 0.514444f; // Knots to meters/sec
        sample->bearing = parts[8].toFloat();

        //qDebug() << sample->latitude << sample->longitude << sample->speed << sample->bearing;
    }

    return true;
}

bool GpsSampleParser::parseGpgga(const QString& line, GpsSample* sample)
{
    //qDebug() << line;

    // Remove checksum, don't check it, never valid
    QString lineData = line.contains('*') ? line.section('*', 0, 0) : line;
    //qDebug() << lineData;

    QStringList parts = lineData.split(',');
    //qDebug() << parts;
    if (parts.size() != 15 || parts.at(0) != "$GPGGA")
    {
        qDebug() << "Failed to split GPGGA";
        return false;
    }

    if (parts.at(1).isEmpty())
        return true;

    int fix = parts.at(6).toInt();
    if (fix > 0)
    {
        sample->sats = parts.at(7).toInt();
        sample->hdop = parts.at(8).toFloat();
        sample->altitude = parts.at(9).toFloat();
        sample->geoidheight = parts.at(11).toFloat();
        //qDebug() << "SATS" << sample->sats << "HDOP" << sample->hdop << "ALT" << sample->altitude;
    }
    return true;
}



bool GpsSampleParser::parseCoord(const QString& part, float& output)
{
    if (part.isEmpty())
        return false;

    QRegularExpressionMatch match = mCoordRegEx.match(part);
    bool dOk, mOk;
    int deg = match.captured(1).toInt(&dOk);
    float min = match.captured(2).toFloat(&mOk);
    //qDebug() << deg << min;

    if (!(dOk && mOk))
    {
       qDebug() << "Failed to parse coordinate" << part;
       return false;
    }

    output = deg + (min / 60.0f);
    return true;
}

