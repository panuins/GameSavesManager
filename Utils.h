/*****************************************************************************
 * Utils.h: util functions header file.
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

#ifndef UTILS_H
#define UTILS_H

#include <QDate>
#include <QDir>
#include <QSqlQuery>
#include <QString>

namespace Utils
{
//const char *formatStrings[9];
QString dbPath();
void printQSqlQueryLastError(const QSqlQuery &query);
void printQSqlQueryLastQuery(const QSqlQuery &query);
QDir absoluteDirOfFilePath(const QString &path);
}

#endif // UTILS_H
