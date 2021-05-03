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

#include "gpsexportwidget.hpp"
#include "ui_gpsexportwidget.h"

#include <QDebug>
#include <QLineEdit>
#include <QFileDialog>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>
#include <QSettings>

#include "toollocator.hpp"
#include "mp4file.hpp"
#include "gpssampleparser.hpp"
#include "gpsexport.hpp"

GpsExportWidget::GpsExportWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GpsExportWidget),
    mFFmpegProc(nullptr),
    mSubsData(nullptr),
    mOutputFile(),
    mCamera()
{
    ui->setupUi(this);

    QComboBox* outputFormatComboBox = findChild<QComboBox*>("outputFormatComboBox");
    outputFormatComboBox->addItem(tr("GPX"), QVariant(int(GpsExportFormat::GPX)));
    outputFormatComboBox->addItem(tr("CSV"), QVariant(int(GpsExportFormat::CSV)));

    connect(
        findChild<QPushButton*>("inputFileButton"),
        &QPushButton::released,
        this,
        &GpsExportWidget::selectInputFile);

    connect(
        findChild<QPushButton*>("outputFileButton"),
        &QPushButton::released,
        this,
        &GpsExportWidget::selectOutputFile);

    connect(
        findChild<QPushButton*>("exportButton"),
        &QPushButton::released,
        this,
        &GpsExportWidget::startExport);

    connect(
        findChild<QLineEdit*>("inputFileEdit"),
        &QLineEdit::textChanged,
        this,
        &GpsExportWidget::inputFileSelected);

    QSettings settings;
    settings.beginGroup("gpsexport");
    findChild<QLineEdit*>("inputFileEdit")->setText(settings.value("inputFileEdit").toString());
    outputFormatComboBox->setCurrentIndex(settings.value("outputFormatComboBox", QVariant(int(0))).toInt());
    findChild<QLineEdit*>("outputFileEdit")->setText(settings.value("outputFileEdit").toString());
}

GpsExportWidget::~GpsExportWidget()
{
    delete ui;
}

void GpsExportWidget::selectInputFile()
{
    QLineEdit* inputFileEdit = findChild<QLineEdit*>("inputFileEdit");
    QString startDir = QDir::fromNativeSeparators(inputFileEdit->text());
    if (startDir.isEmpty())
        startDir = QDir::homePath();
    QString filename = QFileDialog::getOpenFileName(
        this, tr("Select Input File"), startDir, "MP4 file (*.mp4 *.MP4)");
    if (!filename.isEmpty())
        inputFileEdit->setText(QDir::toNativeSeparators(filename));
}

void GpsExportWidget::selectOutputFile()
{
    QLineEdit* outputFileEdit = findChild<QLineEdit*>("outputFileEdit");
    GpsExportFormat format = GpsExportFormat(findChild<QComboBox*>("outputFormatComboBox")->currentData().toInt());

    QString filter;
    switch (format)
    {
    case GpsExportFormat::GPX: filter = "GPX (*.gpx)"; break;
    case GpsExportFormat::CSV: filter = "CSV (*.csv)"; break;
    default:
        QMessageBox::warning(this, tr("Export"), tr("Export format invalid"));
        return;
    }

    QString startDir = QDir::fromNativeSeparators(outputFileEdit->text());
    if (startDir.isEmpty())
        startDir = QDir::homePath();
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Select Output File"), startDir, filter);
    if (!filename.isEmpty())
        outputFileEdit->setText(QDir::toNativeSeparators(filename));
}

void GpsExportWidget::inputFileSelected(const QString& text)
{
    QPushButton* exportButton = findChild<QPushButton*>("exportButton");
    QLineEdit* cameraTypeLine = findChild<QLineEdit*>("cameraTypeLine");
    QLineEdit* cameraSupportLine = findChild<QLineEdit*>("cameraSupportLine");

    if (text.isEmpty())
    {
        exportButton->setDisabled(true);
        cameraTypeLine->setText(QString());
        cameraSupportLine->setText(QString());
        return;
    }

    Mp4File mp4(text);
    if (!mp4.open(QIODevice::ReadOnly))
    {
        exportButton->setDisabled(true);
        cameraTypeLine->setText(tr("Can not open file"));
        cameraSupportLine->setText(QString());
        return;
    }

    QString infoStr = mp4.readInfoString();
    if (infoStr.isEmpty())
    {
        exportButton->setDisabled(true);
        cameraTypeLine->setText(tr("Can not read file"));
        cameraSupportLine->setText(QString());
        return;
    }

    cameraTypeLine->setText(infoStr);

    QString camera = Mp4File::cameraModel(infoStr);
    bool supported = GpsSampleParser::isCameraSupported(camera);
    qDebug() << infoStr << camera << supported;
    exportButton->setEnabled(supported);
    if (supported)
    {
        cameraSupportLine->setText(tr("%1 Supported").arg(camera));
    }
    else
    {
        cameraSupportLine->setText(tr("%1 Not Supported").arg(camera));
    }
}

void GpsExportWidget::startExport()
{
    QString inputFileName = QDir::fromNativeSeparators(findChild<QLineEdit*>("inputFileEdit")->text());
    mOutputFile = QDir::fromNativeSeparators(findChild<QLineEdit*>("outputFileEdit")->text());
    QComboBox* outputFormatComboBox = findChild<QComboBox*>("outputFormatComboBox");
    mExportFormat = GpsExportFormat(outputFormatComboBox->currentData().toInt());

    if (mOutputFile.isEmpty())
    {
        QMessageBox::warning(this, tr("Export"), tr("Output file not set"));
        return;
    }

    if (mExportFormat == GpsExportFormat::Invalid)
    {
        QMessageBox::warning(this, tr("Export"), tr("Export format invalid"));
        return;
    }

    Mp4File mp4(inputFileName);
    if (!mp4.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this, tr("Export"), tr("Input file not found"));
        return;
    }

    QString infoStr = mp4.readInfoString();
    if (infoStr.isEmpty())
    {
        QMessageBox::warning(this, tr("Export"), tr("Can not read file"));
        return;
    }

    mCamera = Mp4File::cameraModel(infoStr);
    bool supported = GpsSampleParser::isCameraSupported(mCamera);
    if (!supported)
    {
        QMessageBox::warning(this, tr("Export"), tr("Camera not supported"));
        return;
    }

    mSubsData = new QBuffer(this);
    if (!mSubsData->open(QIODevice::ReadWrite))
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to create buffer"));
        return;
    }

    QSettings settings;
    settings.beginGroup("gpsexport");
    settings.setValue("inputFileEdit", QDir::toNativeSeparators(inputFileName));
    settings.setValue("outputFormatComboBox", outputFormatComboBox->currentIndex());
    settings.setValue("outputFileEdit", mOutputFile);
    settings.endGroup();

    QStringList args;
    args
        << "-nostdin" << "-hide_banner"
        << "-i" << QDir::toNativeSeparators(inputFileName)
        << "-map" << "0:s" << "-c:s" << "copy"
        << "-f" << "data" << "-";

    qDebug() << ToolLocator::instance()->ffmpeg() << args;
    
    mFFmpegProc = new QProcess(this);
    mFFmpegProc->setProgram(ToolLocator::instance()->ffmpeg());
    mFFmpegProc->setArguments(args);
    mFFmpegProc->setStandardInputFile(QProcess::nullDevice());
    mFFmpegProc->setProcessChannelMode(QProcess::ForwardedErrorChannel);

    connect(
        mFFmpegProc,
        &QProcess::readyReadStandardOutput,
        this,
        &GpsExportWidget::ffmpegStdout);

    connect(
        mFFmpegProc,
        QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &GpsExportWidget::ffmpegFinished);

    findChild<QPushButton*>("exportButton")->setDisabled(true);

    mFFmpegProc->start();
}

void GpsExportWidget::ffmpegStdout()
{
    if (mFFmpegProc)
    {
        while (mFFmpegProc->bytesAvailable())
        {
            QByteArray chunk = mFFmpegProc->readAllStandardOutput();
            qint64 written = 0;
            while (written < chunk.size())
            {
                qint64 wrote = mSubsData->write(chunk.data() + written, chunk.size() - written);
                if (wrote < 0)
                {
                    qWarning() << "Failed to write to buffer";
                }
                written += wrote;
            }
        }
    }
}

void GpsExportWidget::ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    mFFmpegProc->deleteLater();
    mFFmpegProc = nullptr;
    findChild<QPushButton*>("exportButton")->setDisabled(false);
    if (exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to extract GPS data from file"));
        return;
    }

    qDebug() << "Extracted data, " << mSubsData->size() << "bytes";

    mSubsData->seek(0);

    GpsSampleParser parser(mSubsData, mCamera);
    if (!parser.isValid())
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to create parser for GPS data"));
        return;
    }

    QFile outputFile(mOutputFile);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to open output file"));
        return;
    }

    QScopedPointer<GpsExport> exporter(GpsExport::createExporter(mExportFormat, &outputFile));
    if (!(bool(exporter) && exporter->isValid() && exporter->start()))
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to create exporter"));
        return;
    }


    GpsSample sample;
    while (parser.nextSample(&sample))
    {
        if (!exporter->addSample(&sample))
        {
            QMessageBox::warning(this, tr("Export"), tr("Failed to process sample"));
            return;
        }
    }

    if (!(exporter->finish() && outputFile.flush()))
    {
        QMessageBox::warning(this, tr("Export"), tr("Failed to finish exporter"));
        return;
    }
    outputFile.close();
}


