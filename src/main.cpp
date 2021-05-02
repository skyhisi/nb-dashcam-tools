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

#include <QApplication>
#include <QMessageBox>

#include "toollocator.hpp"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName(QLatin1String("SRP"));
    a.setOrganizationDomain(QLatin1String("silasparker.co.uk"));
    a.setApplicationName(QObject::tr("NB Dashcam Tools"));
    a.setApplicationVersion(QLatin1String("@APP_VER_STR@"));

    ToolLocator* tools = ToolLocator::instance();
    tools->addSearchPath(QCoreApplication::applicationDirPath());
    if (!tools->locate())
    {
        QMessageBox::critical(
            nullptr,
            QObject::tr("NB Dashcam Tools"),
            QObject::tr("Failed to find ffmpeg tools."));
        return 1;
    }

    MainWindow w;
    w.show();
    int rc = a.exec();
    ToolLocator::destroy();
    return rc;
}
