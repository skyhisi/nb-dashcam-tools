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

#include "mp4file.hpp"

#include <QDebug>

QString Mp4File::cameraModel(const QString& infoString)
{
    if (infoString.contains("222")) return "222";
    if (infoString.contains("122HD")) return "BASIC";
    if (infoString.contains("322GW")) return "322GW";
    if (infoString.contains("422GW")) return "422GW";
    if (infoString.contains("522GW")) return "522GW";
    if (infoString.contains("522GW")) return "522GW";
    if (infoString.contains("222GX")) return "222GX";
    if (infoString.contains("NBDVR222G-")) return "222G";
    if (infoString.contains("212")) return "BASIC";
    if (infoString.contains("222")) return "BASIC";
    if (infoString == "NBDVR312GW") return "312GW";
    if (infoString.contains("312G")) return "312G";
    if (infoString == "NBDVR412GW") return "412GW";
    if (infoString == "NBDVR512GW") return "512GW";
    if (infoString.contains("612")) return "612GW";
    if (infoString == "NBDVRDHDGW") return "DUOHD";
    if (infoString.contains("NBDVR300")) return "300W";
    if (infoString.contains("NBDVR380GWX")) return "380GWX";
    if (infoString == "NBDVR380GW") return "380GW";
    return QString();
}


Mp4File::Mp4File(const QString& filename) :
    mFile(filename)
{}

Mp4File::~Mp4File()
{
    if (mFile.isOpen())
        mFile.close();
}

bool Mp4File::open(QIODevice::OpenMode mode)
{
    return mFile.open(mode);
}

void Mp4File::close()
{
    mFile.flush();
    mFile.close();
}

quint32 Mp4File::convertUint32(const quint8* data)
{
  return
      (quint32(data[0]) << 24) |
      (quint32(data[1]) << 16) |
      (quint32(data[2]) << 8) |
      quint32(data[3]);
}

quint64 Mp4File::convertUint64(const quint8* data)
{
  return
      (quint64(data[0]) << 56) |
      (quint64(data[1]) << 48) |
      (quint64(data[2]) << 40) |
      (quint64(data[3]) << 32) |
      (quint64(data[4]) << 24) |
      (quint64(data[5]) << 16) |
      (quint64(data[6]) << 8) |
      quint64(data[7]);
}

bool Mp4File::readHeader(AtomHeader* hdr)
{
    Q_ASSERT(hdr != nullptr);
    unsigned char atomHdr[8];
    if (mFile.read((char*)atomHdr, 8) != 8)
    {
        return false;
    }
    hdr->length = convertUint32(atomHdr);
    if (hdr->length < 8)
    {
        return false;
    }
    memcpy(hdr->type, atomHdr + 4, 4);
    return true;
}



QByteArray Mp4File::readUdta(QString* errMsg)
{
    AtomHeader hdr;
    mFile.seek(0);
    qint64 endpos = -1;
    while (!mFile.atEnd() && ((endpos == -1) || (mFile.pos() < endpos)))
    {
        if (!readHeader(&hdr))
        {
            if (errMsg)
                *errMsg = QObject::tr("Failed to locate camera data in file");
            return QByteArray();
        }
        if (hdr == "moov")
        {
            endpos = mFile.pos() + hdr.length - 8;
            continue;
        }
        else if (hdr == "udta")
        {
            QByteArray data = mFile.read(hdr.length - 8);
            if (data.size() != int(hdr.length - 8))
            {
                if (errMsg)
                    *errMsg = QObject::tr("Failed to read camera data in file");
                return QByteArray();
            }
            return data;
        }
        mFile.skip(hdr.length - 8);
    }
    if (errMsg)
        *errMsg = QObject::tr("Failed to locate camera data in file");
    return QByteArray();
}

bool Mp4File::appendUdta(const QByteArray& data, QString* errMsg)
{
    AtomHeader hdr;
    mFile.seek(0);
    qint64 endpos = -1;
    qint64 moovPos = -1;
    quint32 moovSize = 0;
    while (!mFile.atEnd() && ((endpos == -1) || (mFile.pos() < endpos)))
    {
        qint64 hdrPos = mFile.pos();
        if (!readHeader(&hdr))
        {
            if (errMsg)
                *errMsg = QObject::tr("Failed to locate camera data in file");
            return false;
        }
        if (hdr == "moov")
        {
            moovPos = hdrPos;
            moovSize = hdr.length;
            endpos = mFile.pos() + hdr.length - 8;
            continue;
        }
        else if (hdr == "udta")
        {
            // Fast update only works if udta is at end of file
            if ((hdrPos + hdr.length) == mFile.size())
            {
                qDebug() << "POS:" << mFile.pos() << "LEN:" << hdr.length << "FILE SIZE:" << mFile.size();
                quint32 newMoovSize = moovSize + data.size();
                quint32 newUdtaSize = hdr.length + data.size();
                qDebug() << "NEW ATOM SIZE:" << newMoovSize << newUdtaSize;

                unsigned char newMoovSizeBuf[4];
                newMoovSizeBuf[0] = (newMoovSize >> 24) & 0xff;
                newMoovSizeBuf[1] = (newMoovSize >> 16) & 0xff;
                newMoovSizeBuf[2] = (newMoovSize >> 8) & 0xff;
                newMoovSizeBuf[3] = (newMoovSize >> 0) & 0xff;

                unsigned char newUdtaSizeBuf[4];
                newUdtaSizeBuf[0] = (newUdtaSize >> 24) & 0xff;
                newUdtaSizeBuf[1] = (newUdtaSize >> 16) & 0xff;
                newUdtaSizeBuf[2] = (newUdtaSize >> 8) & 0xff;
                newUdtaSizeBuf[3] = (newUdtaSize >> 0) & 0xff;

                if ((mFile.seek(moovPos)) &&
                    (mFile.write((char*)newMoovSizeBuf, 4) == 4) &&
                    (mFile.seek(hdrPos)) &&
                    (mFile.write((char*)newUdtaSizeBuf, 4) == 4) &&
                    (mFile.seek(hdrPos + hdr.length)) &&
                    (mFile.write(data) == data.size()))
                {
                    qDebug() << "NEW FILE SIZE:" << mFile.size() << "NEW POS:" << mFile.pos();
                    return true;
                }
                else
                {
                    if (errMsg)
                        *errMsg = QObject::tr("Failed to update file");
                    return false;
                }
            }
            else
            {
                if (errMsg)
                    *errMsg = QObject::tr("File not in expected format");
                return false;
            }
        }
        mFile.skip(hdr.length - 8);
    }
    return false;
}

QString Mp4File::readInfoString(QString* errMsg)
{
    AtomHeader hdr;
    mFile.seek(0);
    qint64 endpos = -1;
    while (!mFile.atEnd() && ((endpos == -1) || (mFile.pos() < endpos)))
    {
        if (!readHeader(&hdr))
        {
            if (errMsg)
                *errMsg = QObject::tr("Failed to locate camera data in file");
            return QString();
        }
        if (hdr == "moov")
        {
            endpos = mFile.pos() + hdr.length - 8;
            continue;
        }
        else if (hdr == "udta")
        {
            endpos = mFile.pos() + hdr.length - 8;
            continue;
        }
        else if (hdr == "info")
        {
            QByteArray data = mFile.read(hdr.length - 8);
            if (data.size() != int(hdr.length - 8))
            {
                if (errMsg)
                    *errMsg = QObject::tr("Failed to read camera data in file");
                return QString();
            }
            return QString::fromLatin1(data);
        }
        mFile.skip(hdr.length - 8);
    }
    if (errMsg)
        *errMsg = QObject::tr("Failed to locate camera data in file");
    return QString();
}


double Mp4File::readDuration(QString* errMsg)
{
    AtomHeader hdr;
    mFile.seek(0);
    qint64 endpos = -1;
    while (!mFile.atEnd() && ((endpos == -1) || (mFile.pos() < endpos)))
    {
        if (!readHeader(&hdr))
        {
            if (errMsg)
                *errMsg = QObject::tr("Failed to read duration of file");
            return qQNaN();
        }
        if (hdr == "moov")
        {
            endpos = mFile.pos() + hdr.length - 8;
            continue;
        }
        else if (hdr == "mvhd")
        {
            QByteArray data = mFile.read(32);
            if (data.size() != 32)
            {
                if (errMsg)
                    *errMsg = QObject::tr("Failed to read duration of file");
                return qQNaN();
            }
            // Full box version
            const quint8* p = (const quint8*)data.data();
            quint32 version = convertUint32(p + 0);
            quint32 timescale;
            quint64 duration;
            switch (version)
            {
            case 0:
              timescale = convertUint32(p + 12);
              duration = convertUint32(p + 16);
              break;
            case 1:
              timescale = convertUint32(p + 20);
              duration = convertUint64(p + 24);
              break;
            default:
              if (errMsg)
                  *errMsg = QObject::tr("Failed to read duration of file");
              return qQNaN();
            }
            return double(duration) / double(timescale);
        }
        mFile.skip(hdr.length - 8);
    }
    if (errMsg)
        *errMsg = QObject::tr("Failed to locate duration of file");
    return qQNaN();
}
