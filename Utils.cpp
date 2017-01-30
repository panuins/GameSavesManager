/*****************************************************************************
 * Utils.cpp: util functions source file.
 *****************************************************************************
 * Authors: panuins <panuins@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GPLv2 or later as published by the Free Software
 * Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *****************************************************************************/

#include "Utils.h"
#include <QApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QRegExp>
#include <QSettings>
#include <QSqlError>
#include <QSqlQuery>
#include <QString>
#include <QStringList>

QString Utils::dbPath()
{
    QDir dir(QApplication::applicationDirPath());
    if (!dir.exists("saves"))
    {
        dir.mkdir("saves");
    }
    dir.cd("saves");
    if (!dir.exists("savefiles"))
    {
        dir.mkdir("savefiles");
    }
    return dir.absoluteFilePath("savesdata.sqlite");
}

void Utils::printQSqlQueryLastError(const QSqlQuery &query)
{
    QSqlError e = query.lastError();
    if (e.isValid())
    {
        qDebug() << "last error: " << e.text();
        qDebug() << "last query: " << query.lastQuery();
    }
}

void Utils::printQSqlQueryLastQuery(const QSqlQuery &query)
{
    qDebug() << "last query: " << query.lastQuery();
}

QDir Utils::absoluteDirOfFilePath(const QString &path)
{
    QFileInfo info(path);
    return info.absoluteDir();
}
