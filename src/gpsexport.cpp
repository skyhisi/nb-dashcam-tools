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

#include "gpsexport.hpp"

#include <QCoreApplication>

GpsExport* GpsExport::createExporter(GpsExportFormat format, QIODevice* output)
{
    switch (format)
    {
    case GpsExportFormat::Invalid:
        return nullptr;
    case GpsExportFormat::GPX:
        return new GpsExportGpx(output);
    case GpsExportFormat::CSV:
        return new GpsExportCsv(output);
    }
    return nullptr;
}


GpsExport::GpsExport(QIODevice* output) :
    mOutput(output)
{}

GpsExport::~GpsExport()
{}

bool GpsExport::isValid() const
{
    return mOutput && mOutput->isOpen() && mOutput->isWritable();
}

///////////////////////////////////////////////////////////////////////////////

GpsExportGpx::GpsExportGpx(QIODevice* output) :
    GpsExport(output),
    mWriter(mOutput)
{
    mWriter.setAutoFormatting(true);
}

bool GpsExportGpx::start()
{
    mWriter.writeStartDocument();
    mWriter.writeStartElement("gpx");
    mWriter.writeDefaultNamespace("http://www.topografix.com/GPX/1/1");
    mWriter.writeNamespace("https://osmand.net", "osmand");
    mWriter.writeAttribute("version", "1.1");
    mWriter.writeAttribute("creator", QCoreApplication::instance()->applicationName());

    mWriter.writeStartElement("trk");

    mWriter.writeStartElement("trkseg");

    return (!mWriter.hasError());
}

bool GpsExportGpx::finish()
{
    mWriter.writeEndDocument();
    return (!mWriter.hasError());
}

bool GpsExportGpx::addSample(const GpsSample* sample)
{
    Q_ASSERT(sample);
    if (!(sample->datetime.isValid() && sample->gpsValid))
        return true;

    mWriter.writeStartElement("trkpt");
    mWriter.writeAttribute("lat", QString::number(sample->latitude, 'f', 6)); // 6 decimal places ~ 10cm precision
    mWriter.writeAttribute("lon", QString::number(sample->longitude, 'f', 6));
    mWriter.writeTextElement("time", sample->datetime.toString(Qt::ISODateWithMs));

    if (!qIsNaN(sample->altitude))
        mWriter.writeTextElement("ele", QString::number(sample->altitude, 'f', 1));
    if (!qIsNaN(sample->geoidheight))
        mWriter.writeTextElement("geoidheight", QString::number(sample->geoidheight, 'f', 1));
    mWriter.writeTextElement("sat", QString::number(sample->sats));
    if (!qIsNaN(sample->hdop))
        mWriter.writeTextElement("hdop", QString::number(sample->hdop, 'f', 2));

    mWriter.writeStartElement("extensions");
        if (!qIsNaN(sample->speed))
            mWriter.writeTextElement("osmand:speed", QString::number(sample->speed, 'f', 1));
        if (!qIsNaN(sample->bearing))
            mWriter.writeTextElement("osmand:heading", QString::number(int(sample->bearing)));
    mWriter.writeEndElement();

    mWriter.writeEndElement();
    return (!mWriter.hasError());
}

///////////////////////////////////////////////////////////////////////////////

GpsExportCsv::GpsExportCsv(QIODevice* output) :
    GpsExport(output),
    mStream(mOutput)
{
    mStream.setRealNumberNotation(QTextStream::FixedNotation);
}

bool GpsExportCsv::start()
{
    mStream << "Date,Time,Latitude,Longitude,Elevation,GeoidHeight,Satellites,HDOP,Speed,Bearing,Xacc,Yacc,Zacc\n";
    return true;
}

bool GpsExportCsv::finish()
{
    mStream.flush();
    return true;
}

bool GpsExportCsv::addSample(const GpsSample* sample)
{
    if (!sample->datetime.isValid())
        return true;

    mStream << sample->datetime.date().toString(Qt::ISODate) << "," << sample->datetime.time().toString(Qt::ISODateWithMs) << ",";;

    mStream.setRealNumberPrecision(6);
    if (sample->gpsValid && !(qIsNaN(sample->latitude) || qIsNaN(sample->longitude)))
        mStream << sample->latitude << "," << sample->longitude << ",";
    else
        mStream << ",,";

    mStream.setRealNumberPrecision(1);

    if (sample->gpsValid && !(qIsNaN(sample->speed) || qIsNaN(sample->bearing)))
        mStream << sample->speed << "," << sample->bearing << ",";
    else
        mStream << ",,";

    if (sample->gpsValid && !(qIsNaN(sample->altitude) || qIsNaN(sample->geoidheight)))
        mStream << sample->altitude << "," << sample->geoidheight << ",";
    else
        mStream << ",,";

    if (sample->gpsValid)
        mStream << sample->sats << "," << sample->hdop << ",";
    else
        mStream << ",,";


    mStream.setRealNumberPrecision(2);
    if (!(qIsNaN(sample->xAcc) || qIsNaN(sample->yAcc) || qIsNaN(sample->zAcc)))
        mStream << sample->xAcc << "," << sample->yAcc << "," << sample->zAcc << "\n";
    else
        mStream << ",,\n";

    return true;
}


