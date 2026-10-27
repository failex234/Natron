/* ***** BEGIN LICENSE BLOCK *****
 * This file is part of Natron <https://natrongithub.github.io/>,
 * (C) 2018-2023 The Natron developers
 * (C) 2013-2018 INRIA and Alexandre Gauthier-Foichat
 *
 * Natron is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Natron is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Natron.  If not, see <http://www.gnu.org/licenses/gpl-2.0.html>
 * ***** END LICENSE BLOCK ***** */
#ifndef NATRON_GLOBAL_QTCOMPAT_H
#define NATRON_GLOBAL_QTCOMPAT_H

#include "Global/Macros.h"

#include <QtGlobal> // for Q_OS_*
#include <QString>
#include <QUrl>
#include <QFileInfo>
#include <QEvent>
#include <QMutex>
#include <QMutexLocker>

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
// QEnterEvent lives in QtGui, which not every module (e.g. Engine) links against:
// a forward declaration is enough for the pointer typedef below.
class QEnterEvent;
#endif

NATRON_NAMESPACE_ENTER

namespace QtCompat {
/*Removes the . and the extension from the filename and also
 * returns the extension as a string.*/
inline QString
removeFileExtension(QString & filename)
{
    //qDebug() << "remove file ext from" << filename;
    QFileInfo fi(filename);
    QString extension = fi.suffix();

    if ( !extension.isEmpty() ) {
        filename.truncate(filename.size() - extension.size() - 1);
    }

    //qDebug() << "->" << filename << fi.suffix();
    return extension;
}

// Define compatibility typedefs so code builds with Qt5 & Qt6
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
typedef ::QEnterEvent QEnterEvent;
// QMutexLocker is a class template in Qt6 (QMutexLocker<QMutex>), a plain class in Qt5.
typedef ::QMutexLocker<::QMutex> MutexLocker;
typedef ::QMutexLocker<::QRecursiveMutex> RecursiveMutexLocker;
#elif QT_VERSION >= QT_VERSION_CHECK(5, 0, 0)
typedef QEvent QEnterEvent;
typedef ::QMutexLocker MutexLocker;
typedef ::QMutexLocker RecursiveMutexLocker; // Qt >= 5.14 accepts QRecursiveMutex*
#else
#error "Unsupported version of QT"
#endif

} // namespace QtCompat

NATRON_NAMESPACE_EXIT

#endif // NATRON_GLOBAL_QTCOMPAT_H
