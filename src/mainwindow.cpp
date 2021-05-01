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

#include "mainwindow.hpp"

#include <QSettings>
#include <QMessageBox>
#include <QMenuBar>

#include "clipmergewidget.hpp"
#include "gpsexportwidget.hpp"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    mTabs(new QTabWidget()),
    mSettingsCleared(false)
{
    setWindowTitle(tr("NB Dashcam Tools"));
    setCentralWidget(mTabs);
    resize(800, 600);

    mTabs->addTab(new ClipMergeWidget(), tr("Clip Merge"));
    mTabs->addTab(new GpsExportWidget(), tr("GPS Export"));

    QMenu* appMenu = menuBar()->addMenu(tr("App"));
    appMenu->addAction(tr("Clear Settings"), this, &MainWindow::clearSettings);
    appMenu->addAction(tr("Quit"), this, &MainWindow::close);

    QSettings settings;
    settings.beginGroup("mainwindow");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
    mTabs->setCurrentIndex(settings.value("currentTab", 0).toInt());
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!mSettingsCleared)
    {
        QSettings settings;
        settings.beginGroup("mainwindow");
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        settings.setValue("currentTab", mTabs->currentIndex());
    }
    QWidget::closeEvent(event);
}

void MainWindow::clearSettings()
{
    QSettings settings;
    settings.clear();
    settings.sync();
    mSettingsCleared = true;

    QMessageBox::StandardButton btn = QMessageBox::question(
        this, tr("Close Application"),
        tr("To finish clearing the settings the program must now be closed.\nDo you want to close the application now?"));
    if (btn == QMessageBox::Yes)
    {
        QMetaObject::invokeMethod(this, &MainWindow::close, Qt::QueuedConnection);
    }
}

