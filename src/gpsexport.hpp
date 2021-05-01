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

#ifndef GPSEXPORT_HPP
#define GPSEXPORT_HPP

#include <QXmlStreamWriter>
#include <QTextStream>

#include "gpssampleparser.hpp"

enum class GpsExportFormat : int
{
    Invalid = 0,
    GPX,
    CSV
};


class GpsExport
{
public:
    virtual ~GpsExport();
    virtual bool isValid() const;
    virtual bool start() = 0;
    virtual bool finish() = 0;
    virtual bool addSample(const GpsSample* sample) = 0;

    static GpsExport* createExporter(GpsExportFormat format, QIODevice* output);

protected:
    GpsExport(QIODevice* output);
    QIODevice* mOutput;
};


class GpsExportGpx : public GpsExport
{
public:
    GpsExportGpx(QIODevice* output);
    bool start() override;
    bool finish() override;
    bool addSample(const GpsSample* sample) override;

private:
    QXmlStreamWriter mWriter;
};

class GpsExportCsv : public GpsExport
{
public:
    GpsExportCsv(QIODevice* output);
    bool start() override;
    bool finish() override;
    bool addSample(const GpsSample* sample) override;

private:
    QTextStream mStream;
};


#endif // GPSEXPORT_HPP
