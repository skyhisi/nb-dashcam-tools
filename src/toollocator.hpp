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

#ifndef TOOLLOCATOR_HPP
#define TOOLLOCATOR_HPP

#include <QStringList>

class ToolLocator
{
public:
    static ToolLocator* instance();
    static void destroy();

    void addSearchPath(const QString& path);
    bool locate();

    const QString& ffmpeg() const {return mFFmpegPath;}

private:
    ToolLocator();
    bool locateTool(const QString& name, QString& path);
    static ToolLocator* mInstance;

    QStringList mSearchPaths;
    QString mFFmpegPath;
};

#endif // TOOLLOCATOR_HPP
