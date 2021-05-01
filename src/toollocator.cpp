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

#include "toollocator.hpp"

#include <QDebug>
#include <QDir>
#include <QStandardPaths>

ToolLocator* ToolLocator::mInstance = nullptr;

ToolLocator* ToolLocator::instance()
{
    if (!mInstance)
        mInstance = new ToolLocator();
    return mInstance;
}

void ToolLocator::destroy()
{
    delete mInstance;
    mInstance = nullptr;
}

void ToolLocator::addSearchPath(const QString& path)
{
    QDir dir(path);
    if (dir.exists(path))
        mSearchPaths << dir.absolutePath();
}

bool ToolLocator::locate()
{
    return locateTool("ffmpeg", mFFmpegPath);
}


ToolLocator::ToolLocator():
    mSearchPaths(),
    mFFmpegPath()
{}

bool ToolLocator::locateTool(const QString& name, QString& path)
{
    if (path.isEmpty() && !mSearchPaths.isEmpty())
        path = QStandardPaths::findExecutable(name, mSearchPaths);
    if (path.isEmpty())
        path = QStandardPaths::findExecutable(name);
    qDebug() << "Tool:" << name << path;
    return !path.isEmpty();
}



