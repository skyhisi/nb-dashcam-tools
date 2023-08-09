/* Copyright 2021-2023 Silas Parker.
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

#ifndef CLIPMERGEWIDGET_H
#define CLIPMERGEWIDGET_H

#include <QWidget>

#include <QFileSystemModel>
#include <QProcess>
#include <QProgressDialog>
#include <QRegularExpression>
#include <QTextStream>

namespace Ui {
class ClipMergeWidget;
}

class ClipMergeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ClipMergeWidget(QWidget *parent = nullptr);
    ~ClipMergeWidget();

private slots:
    void inputDirSelect();
    void outputFileSelect();
    void inputDirChanged();
    void selectFilesInRoute();
    void startMerge();
    void ffmpegStdout();
    void ffmpegFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void cancelMerge();
    void nvencCheckStart();
    void nvencCheckFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void qsvCheckStart();
    void qsvCheckFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void encodeChanged();

private:
    Ui::ClipMergeWidget *ui;
    QFileSystemModel* mInputFileModel;
    QStringList mInputFileList;
    QString mOutputFile;
    QProcess* mFFmpegProc;
    QTextStream mFFmpegStream;
    QProgressDialog* mProgDlg;
    QRegularExpression mFFmpegRegex;
    bool mHaveNvenc;
    bool mHaveQsv;
    QByteArray mUdtaData;

    enum VideoEncode
    {
        VideoEncodeCopy = 0,
        VideoEncodeSoftware,
        VideoEncodeNVidia,
        VideoEncodeQsv
    };

};

#endif // CLIPMERGEWIDGET_H
