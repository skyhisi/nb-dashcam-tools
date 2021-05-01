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

#ifndef GPSEXPORTWIDGET_H
#define GPSEXPORTWIDGET_H

#include <QWidget>
#include <QProcess>
#include <QBuffer>

#include "gpsexport.hpp"

namespace Ui {
class GpsExportWidget;
}

class GpsExportWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GpsExportWidget(QWidget *parent = nullptr);
    ~GpsExportWidget();

private slots:
    void selectInputFile();
    void selectOutputFile();
    void inputFileSelected(const QString& text);
    void startExport();
    void ffmpegStdout();
    void ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);


private:
    Ui::GpsExportWidget *ui;

    QProcess* mFFmpegProc;
    QBuffer* mSubsData;
    QString mOutputFile;
    QString mCamera;
    GpsExportFormat mExportFormat;
};

#endif // GPSEXPORTWIDGET_H
