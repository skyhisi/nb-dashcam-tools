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

#ifndef MP4READER_HPP
#define MP4READER_HPP

#include <QString>
#include <QFile>

class Mp4File
{
public:
    static QString cameraModel(const QString& infoString);

    Mp4File(const QString& filename);
    ~Mp4File();
    bool open(QIODevice::OpenMode mode);
    void close();

    QByteArray readUdta(QString* errMsg = nullptr);
    bool appendUdta(const QByteArray& data, QString* errMsg = nullptr);
    QString readInfoString(QString* errMsg = nullptr);
    double readDuration(QString* errMsg);

private:
    struct AtomHeader
    {
        quint64 length;
        quint32 hdrSize;
        char    type[4];

        bool operator==(const char* t) const
        {return strncmp(type, t, 4) == 0;}

        quint64 lengthAfterHdr() const
        {return length - hdrSize;}
    };
    static quint32 convertUint32(const quint8* data);
    static quint64 convertUint64(const quint8* data);
    bool readHeader(AtomHeader* hdr);
    QFile mFile;
};

#endif // MP4READER_HPP
