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

#include "clipmergewidget.hpp"
#include "ui_clipmergewidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QProcess>
#include <QTemporaryFile>
#include <QComboBox>
#include <QSpinBox>
#include <QDebug>
#include <QtGlobal>
#include <QDateTime>
#include <QSettings>

#include "mp4file.hpp"
#include "toollocator.hpp"


ClipMergeWidget::ClipMergeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClipMergeWidget),
    mInputFileModel(new QFileSystemModel(this)),
    mInputFileList(),
    mOutputFile(),
    mFFmpegProc(nullptr),
    mFFmpegStream(),
    mProgDlg(new QProgressDialog(this)),
    mFFmpegRegex("time=(\\d\\d):(\\d\\d):(\\d\\d.\\d\\d)"),
    mHaveNvenc(false),
    mUdtaData()
{
    Q_ASSERT(mFFmpegRegex.isValid());
    ui->setupUi(this);

    mInputFileModel->setFilter(QDir::Files | QDir::Readable);
    mInputFileModel->setNameFilters(QStringList("*.mp4"));

    QTableView* inputFileView = findChild<QTableView*>("inputFileView");
    inputFileView->setSelectionMode(QAbstractItemView::MultiSelection);
    inputFileView->setSelectionBehavior(QAbstractItemView::SelectRows);
    inputFileView->verticalHeader()->setVisible(false);
    inputFileView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    inputFileView->horizontalHeader()->setHighlightSections(false);

    mProgDlg->setWindowTitle(tr("Merge"));
    mProgDlg->setWindowModality(Qt::WindowModal);
    mProgDlg->setMinimumDuration(500);
    mProgDlg->setAutoReset(false);
    mProgDlg->reset();

    QComboBox* videoEncodeComboBox = findChild<QComboBox*>("videoEncodeComboBox");
    videoEncodeComboBox->addItem(tr("Copy Video (fast)"), QVariant(int(VideoEncodeCopy)));
    videoEncodeComboBox->addItem(tr("Re-encode Video (slow)"), QVariant(int(VideoEncodeSoftware)));
    videoEncodeComboBox->setCurrentIndex(0);

    QMetaObject::invokeMethod(this, &ClipMergeWidget::nvencCheckStart, Qt::QueuedConnection);

    connect(
        findChild<QPushButton*>("inputDirButton"),
        &QPushButton::released,
        this,
        &ClipMergeWidget::inputDirSelect);

    connect(
        findChild<QPushButton*>("outputFileButton"),
        &QPushButton::released,
        this,
        &ClipMergeWidget::outputFileSelect);

    connect(
        findChild<QLineEdit*>("inputDirEdit"),
        &QLineEdit::textChanged,
        this,
        &ClipMergeWidget::inputDirChanged);

    connect(
        findChild<QPushButton*>("matchingInputFileButton"),
        &QPushButton::released,
        this,
        &ClipMergeWidget::selectFilesInRoute);

    connect(
        findChild<QPushButton*>("mergeButton"),
        &QPushButton::released,
        this,
        &ClipMergeWidget::startMerge);

    connect(
        mProgDlg,
        &QProgressDialog::canceled,
        this,
        &ClipMergeWidget::cancelMerge);

    connect(
        videoEncodeComboBox,
        QOverload<int>::of(&QComboBox::currentIndexChanged),
        this,
        &ClipMergeWidget::encodeChanged);


    QSettings settings;
    settings.beginGroup("clipmerge");
    findChild<QLineEdit*>("inputDirEdit")->setText(settings.value("inputDirEdit").toString());
    findChild<QLineEdit*>("outputFileEdit")->setText(settings.value("outputFileEdit").toString());
    videoEncodeComboBox->setCurrentIndex(settings.value("videoEncodeComboBox", 0).toInt());
    findChild<QSpinBox*>("compFactorSpinBox")->setValue(settings.value("compFactorSpinBox", 30).toInt());
    findChild<QCheckBox*>("includeGpsCheckBox")->setChecked(settings.value("includeGpsCheckBox", true).toBool());
}

ClipMergeWidget::~ClipMergeWidget()
{
    delete ui;
}


void ClipMergeWidget::inputDirSelect()
{
    QLineEdit* inputDirEdit = findChild<QLineEdit*>("inputDirEdit");
    QString startDir = QDir::fromNativeSeparators(inputDirEdit->text());
    if (startDir.isEmpty())
        startDir = QDir::homePath();
    QString dir = QFileDialog::getExistingDirectory(
        this, tr("Select Input Directory"), startDir, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty())
        inputDirEdit->setText(QDir::toNativeSeparators(dir));
}


void ClipMergeWidget::outputFileSelect()
{
    QLineEdit* outputFileEdit = findChild<QLineEdit*>("outputFileEdit");
    QString startDir = QDir::fromNativeSeparators(outputFileEdit->text());
    if (startDir.isEmpty())
        startDir = QDir::homePath();
    QString filename = QFileDialog::getSaveFileName(
        this, tr("Select Output File"), startDir, "MP4 File (*.mp4)");
    if (!filename.isEmpty())
        outputFileEdit->setText(QDir::toNativeSeparators(filename));
}


void ClipMergeWidget::inputDirChanged()
{
    QLineEdit* inputDirEdit = findChild<QLineEdit*>("inputDirEdit");
    QTableView* inputFileView = findChild<QTableView*>("inputFileView");
    QDir inputDir(QDir::fromNativeSeparators(inputDirEdit->text()));
    if (inputDir.exists())
    {
        qDebug() << "Input dir exists" << inputDir.absolutePath();
        mInputFileModel->setRootPath(inputDir.path());
        mInputFileModel->setFilter(QDir::Files);
        inputFileView->setModel(mInputFileModel);
        inputFileView->setRootIndex(mInputFileModel->index(inputDir.path()));
    }
    else
    {
        qDebug() << "Input dir invalid";
        inputFileView->clearSelection();
        inputFileView->setModel(nullptr);
    }
}


void ClipMergeWidget::selectFilesInRoute()
{
    QPushButton* selectionButton = findChild<QPushButton*>("matchingInputFileButton");
    QTableView* inputFileView = findChild<QTableView*>("inputFileView");
    if (inputFileView->model() == nullptr || inputFileView->selectionModel()->selectedRows().count() != 1)
    {
        QMessageBox::information(this, selectionButton->text(), tr("Select the first file in the route to match"));
        return;
    }

    QRegularExpression filenameRegex(
                "^(\\d{6}_\\d{6})_(\\d{3})_([BFR][HL])\\.MP4$",
                QRegularExpression::CaseInsensitiveOption);
    filenameRegex.optimize();

    const QString dtPattern("yyyyMMdd_HHmmss");

    QModelIndex idx = inputFileView->selectionModel()->selectedRows().at(0);
    //qDebug() << "idx" << idx;
    QString startFilename = mInputFileModel->data(idx, QFileSystemModel::FileNameRole).toString();
    //qDebug() << "startFilename" << startFilename;

    QRegularExpressionMatch match = filenameRegex.match(startFilename);
    if (!match.hasMatch())
    {
        QMessageBox::information(this, selectionButton->text(), tr("Filename not in expected format"));
        return;
    }


    QDateTime lastDT(QDateTime::fromString(QLatin1String("20") + match.captured(1), dtPattern));
    //qDebug() << "match" << match << "lastDT" << lastDT;
    //int lastCount = match.captured(2).toInt();
    QString quality = match.captured(3);

    int rowCount = mInputFileModel->rowCount(inputFileView->rootIndex());
    for (int row = idx.row() + 1; row < rowCount; ++row)
    {
        idx = mInputFileModel->index(row, 0, inputFileView->rootIndex());
        //qDebug() << "idx" << idx;
        QString filepath = mInputFileModel->data(idx, QFileSystemModel::FileNameRole).toString();
        match = filenameRegex.match(filepath);
        if (!match.hasMatch())
            continue;
        //qDebug() << "filepath" << filepath << match.captured(3);
        if (match.captured(3) != quality)
            continue;

        QDateTime dt(QDateTime::fromString(QLatin1String("20") + match.captured(1), dtPattern));
        //qDebug() << "match" << match << "dt" << dt;
        qint64 timeDiff = lastDT.secsTo(dt);
        //qDebug() << "timeDiff" << timeDiff;
        if (timeDiff == 0 || timeDiff > 300)
            break;

        qDebug() << "filepath" << filepath;
        lastDT = dt;

        inputFileView->selectionModel()->select(idx, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }

    inputFileView->setFocus();
}


void ClipMergeWidget::startMerge()
{
    QPushButton* mergeButton = findChild<QPushButton*>("mergeButton");
    QTableView* inputFileView = findChild<QTableView*>("inputFileView");
    QLineEdit* outputFileEdit = findChild<QLineEdit*>("outputFileEdit");
    const bool includeGpsData = findChild<QCheckBox*>("includeGpsCheckBox")->isChecked();

    QModelIndexList selectionList = inputFileView->selectionModel()->selectedRows();
    mInputFileList.clear();

    if (selectionList.isEmpty())
    {
        QMessageBox::warning(this, tr("Merge"), tr("No files selected"));
        return;
    }

    mProgDlg->setMaximum(selectionList.size());
    mProgDlg->setLabelText(tr("Preparing for merge"));
    mProgDlg->setCancelButtonText(QString());

    for (const QModelIndex& idx: selectionList)
    {
        mInputFileList << mInputFileModel->data(idx, QFileSystemModel::FilePathRole).toString();
    }

    mInputFileList.sort();
    mUdtaData.clear();

    float duration = 0.0f;
    for (int i = 0; i < mInputFileList.size(); ++i)
    {
        const QString& filepath = mInputFileList[i];
        mProgDlg->setValue(i);

        Mp4File probeFile(filepath);
        if (!probeFile.open(QIODevice::ReadOnly))
        {
            mProgDlg->reset();
            QMessageBox::warning(this, tr("Merge"), tr("Input file not found:\n%1").arg(filepath));
            return;
        }
        QString errmsg;
        double probeDuration = probeFile.readDuration(&errmsg);

        if (includeGpsData && (i == 0))
        {
            mUdtaData = probeFile.readUdta(&errmsg);
            if (mUdtaData.isEmpty())
            {
                if (errmsg.isEmpty())
                    errmsg = tr("Failed to read camera info");
                QMessageBox::warning(this, tr("Merge"), errmsg);
                return;
            }
        }

        probeFile.close();
        if (qIsNaN(probeDuration))
        {
          mProgDlg->reset();
          QMessageBox::warning(this, tr("Merge"), errmsg);
          return;
        }

        qDebug() << filepath << probeDuration;
        duration += probeDuration;
    }

    mOutputFile = QDir::fromNativeSeparators(outputFileEdit->text());
    if (mOutputFile.isEmpty())
    {
        mProgDlg->reset();
        QMessageBox::warning(this, tr("Merge"), tr("Output file not set"));
        return;
    }

    mFFmpegProc = new QProcess(this);
    QString tmpFormat(QDir(QDir::tempPath()).absoluteFilePath("nbtools.XXXXXX"));
    QTemporaryFile* concatFile = new QTemporaryFile(tmpFormat, mFFmpegProc);
    if (!concatFile->open())
    {
        mProgDlg->reset();
        QMessageBox::warning(this, tr("Merge"), tr("Failed to create temp concat file"));
        return;
    }

    { // Scope for stream
        QTextStream concatStream(concatFile);
        for (const QString& file : mInputFileList)
        {
            concatStream << "file '" << QDir::toNativeSeparators(file) << "'\n";
        }
    }
    concatFile->close();

    QComboBox* videoEncodeComboBox = findChild<QComboBox*>("videoEncodeComboBox");
    VideoEncode encode = VideoEncode(videoEncodeComboBox->currentData().toInt());

    QStringList args;
    args << "-hide_banner" << "-y" << "-nostdin"; // Global args

    // Use nvdec
    if (encode == VideoEncodeNVidia)
    {
        args << "-hwaccel" << "cuda" << "-hwaccel_output_format" << "cuda";
    }

    // Input args
    args << "-f" << "concat" << "-safe" << "0" << "-i" << QDir::toNativeSeparators(concatFile->fileName());

    QSpinBox* compFactorSpinBox = findChild<QSpinBox*>("compFactorSpinBox");
    QString crfStr(QString::number(compFactorSpinBox->value()));

    switch (encode)
    {
    case VideoEncodeCopy:
        args << "-c:v" << "copy";
        break;
    case VideoEncodeSoftware:
        args << "-c:v" << "libx264" << "-crf" << crfStr;
        break;
    case VideoEncodeNVidia:
        args << "-c:v" << "h264_nvenc" << "-rc" << "vbr" << "-cq" << crfStr;
        break;
    }

    // Subtitle track is GPS data
    if (includeGpsData)
    {
        args << "-c:s" << "copy"; // Copy subtitles
    }
    else
    {
        args << "-map" << "0:v" << "-map" << "0:a"; // Only merge video & audio
    }

    args << QDir::toNativeSeparators(mOutputFile);
    qDebug() << ToolLocator::instance()->ffmpeg() << args;


    QSettings settings;
    settings.beginGroup("clipmerge");
    settings.setValue("inputDirEdit", findChild<QLineEdit*>("inputDirEdit")->text());
    settings.setValue("outputFileEdit", findChild<QLineEdit*>("outputFileEdit")->text());
    settings.setValue("videoEncodeComboBox", videoEncodeComboBox->currentIndex());
    settings.setValue("compFactorSpinBox", compFactorSpinBox->value());
    settings.setValue("includeGpsCheckBox", findChild<QCheckBox*>("includeGpsCheckBox")->isChecked());
    settings.endGroup();

    mFFmpegProc->setProgram(ToolLocator::instance()->ffmpeg());
    mFFmpegProc->setArguments(args);
    mFFmpegProc->setStandardInputFile(QProcess::nullDevice());
    mFFmpegProc->setProcessChannelMode(QProcess::MergedChannels);
    mFFmpegStream.setDevice(mFFmpegProc);

    mProgDlg->reset();
    mProgDlg->setValue(0);
    mProgDlg->setMaximum(duration);
    mProgDlg->setCancelButtonText(tr("Cancel"));

    connect(
        mFFmpegProc,
        &QProcess::readyRead,
        this,
        &ClipMergeWidget::ffmpegStdout);

    connect(
        mFFmpegProc,
        QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &ClipMergeWidget::ffmpegFinished);

    mFFmpegProc->start();
    mergeButton->setDisabled(true);

}

void ClipMergeWidget::ffmpegStdout()
{
    if (mProgDlg->wasCanceled())
        return;

    QString line;
    while (mFFmpegStream.readLineInto(&line))
    {
        //qDebug() << "FFMPEG: " << line;
        QRegularExpressionMatch match = mFFmpegRegex.match(line);
        if (match.hasMatch())
        {
            float pos = (match.captured(1).toInt() * 3600) + (match.captured(2).toInt() * 60) + match.captured(3).toFloat();
            mProgDlg->setValue(pos);
            mProgDlg->setLabelText(tr("Merging: %1 / %2").arg(pos, 0, 'f', 1).arg(mProgDlg->maximum()));
        }
    }
}

void ClipMergeWidget::ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "FFmpeg finished" << exitCode << exitStatus;
    mFFmpegProc->deleteLater();
    mFFmpegProc = nullptr;
    mProgDlg->reset();
    findChild<QPushButton*>("mergeButton")->setDisabled(false);

    if (exitStatus != QProcess::NormalExit || exitCode != 0)
    {
        QMessageBox::warning(this, tr("Merge"), tr("Failed to merge files"));
        return;
    }

    // If not adding GPS data, don't copy camera info
    if (!findChild<QCheckBox*>("includeGpsCheckBox")->isChecked())
        return;

    Mp4File outFile(mOutputFile);
    if (!outFile.open(QFile::ReadWrite | QFile::ExistingOnly))
    {
        QMessageBox::warning(this, tr("Merge"), tr("Failed to open output file to add GPS data."));
        return;
    }
    QString err;
    if (!outFile.appendUdta(mUdtaData, &err))
    {
        QMessageBox::warning(this, tr("Merge"), err);
        return;
    }
    outFile.close();

}

void ClipMergeWidget::cancelMerge()
{
    if (mFFmpegProc)
    {
        mFFmpegProc->terminate();
    }
}

void ClipMergeWidget::nvencCheckStart()
{
    QStringList nvencCheckArgs;
    nvencCheckArgs
        << "-hide_banner"
        << "-f" << "lavfi" << "-i" << "nullsrc=s=256x256:d=5"
        << "-c:v" << "hevc_nvenc"
        << "-gpu" << "list"
        << "-f" << "null" << QProcess::nullDevice();

    mFFmpegProc = new QProcess(this);
    mFFmpegProc->setProgram(ToolLocator::instance()->ffmpeg());
    mFFmpegProc->setArguments(nvencCheckArgs);
    mFFmpegProc->setStandardInputFile(QProcess::nullDevice());

    connect(
        mFFmpegProc,
        QOverload<int,QProcess::ExitStatus>::of(&QProcess::finished),
        this,
        &ClipMergeWidget::nvencCheckFinished);

    mFFmpegProc->start();
}

void ClipMergeWidget::nvencCheckFinished(int, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit)
    {
        QTextStream stream(mFFmpegProc->readAllStandardError());
        QString line;
        while (stream.readLineInto(&line))
        {
            //qDebug() << "NVENC PROBE: " << line;
            if (line.startsWith("[hevc_nvenc") && line.contains("Compute SM"))
            {
                mHaveNvenc = true;
                QComboBox* videoEncodeComboBox = findChild<QComboBox*>("videoEncodeComboBox");
                videoEncodeComboBox->addItem(tr("Re-encode Video Using NVidia"), QVariant(int(VideoEncodeNVidia)));

                QSettings settings;
                videoEncodeComboBox->setCurrentIndex(settings.value("clipmerge/videoEncodeComboBox", 0).toInt());
            }
        }
    }
    mFFmpegProc->deleteLater();
    mFFmpegProc = nullptr;
}

void ClipMergeWidget::encodeChanged()
{
    VideoEncode encode = VideoEncode(findChild<QComboBox*>("videoEncodeComboBox")->currentData().toInt());
    QLabel* compressionLabel = findChild<QLabel*>("compressionLabel");
    QSpinBox* compFactorSpinBox = findChild<QSpinBox*>("compFactorSpinBox");
    compressionLabel->setEnabled(encode != VideoEncodeCopy);
    compFactorSpinBox->setEnabled(encode != VideoEncodeCopy);
}



