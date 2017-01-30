/*****************************************************************************
 * GameDB.cpp: GameDB source file.
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

#include "GameDB.h"
#include "Utils.h"
#include <QDebug>
#include <QSqlError>

QPointer<GameDB> GameDB::m_gameDb;

/*!
 * \class GameDB
 * \brief GameDB
 * \details 
 */

/*!
 * \fn GameDB::GameDB(QObject *parent)
 * \brief Default constructor.
 * \param parent pointer to parent object
 * \details Default constructor.
 */
GameDB::GameDB(QObject *parent) :
    QObject(parent)
{
    m_gameDb = this;
    initialDatabase();
}

/*!
 * \fn GameDB::GameDB()
 * \brief Destructor.
 * \details Destructor.
 */
GameDB::~GameDB()
{
}

bool GameDB::createConnection()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE", "savesdata");
    m_db.setDatabaseName(Utils::dbPath());
    if (!m_db.open())
    {
        qDebug() << "GameDB::createConnection: Error: can not open database"
                  << Utils::dbPath() << ".";
        qDebug() << "GameDB::createConnection: last error: "
                  << m_db.lastError().text();
        return false;
    }
    else
    {
        return true;
    }
}

bool GameDB::initialDatabase()
{
    bool create = !QFile::exists(Utils::dbPath());
    bool ret = createConnection();
    if (!ret)
    {
        qDebug() << "GameDB::initialDatabase: Error: Cannot open db.";
        return false;
    }
    if (create)
    {
        ret = resetTables();
        if (!ret)
        {
            qDebug() << "GameDB::initialDatabase: Error: Cannot create tables.";
            return false;
        }
    }
    m_gameInfo = new QSqlTableModel(this, m_db);
    m_gameInfo->setTable("tbl_gameInfo");
    m_savesData = new QSqlTableModel(this, m_db);
    m_savesData->setTable("tbl_savesData");
    m_savesData->setHeaderData(TBL_savesData_ID, Qt::Horizontal, tr("ID"));
    m_savesData->setHeaderData(TBL_savesData_GameID, Qt::Horizontal, tr("GameID"));
    m_savesData->setHeaderData(TBL_savesData_Hash, Qt::Horizontal, tr("Hash"));
    m_savesData->setHeaderData(TBL_savesData_Data, Qt::Horizontal, tr("Data"));
    m_savesData->setHeaderData(TBL_savesData_DateTime, Qt::Horizontal, tr("DateTime"));
    m_savesData->setHeaderData(TBL_savesData_UserDateTime, Qt::Horizontal, tr("UserDateTime"));
    m_savesData->setHeaderData(TBL_savesData_Branch, Qt::Horizontal, tr("Branch"));
    m_savesData->setHeaderData(TBL_savesData_Memo, Qt::Horizontal, tr("Memo"));
    m_savesData->setHeaderData(TBL_savesData_Tag, Qt::Horizontal, tr("Tag"));
    return true;
}

bool GameDB::resetTables()
{
    QSqlQuery query(m_db);
    bool ret;
    query.exec("drop table tbl_gameInfo");
    Utils::printQSqlQueryLastQuery(query);
    query.exec("drop table tbl_savesData");
    Utils::printQSqlQueryLastQuery(query);
//    query.exec("drop table tbl_savesData_bin");
//    query.exec("drop table tbl_savesData_text");
//    query.exec("drop table tbl_savesData_mutiFile");
    ret = query.exec("CREATE TABLE tbl_gameInfo ("
                     "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                     "Name TEXT NOT NULL,"
                     "SaveType INTEGER NOT NULL,"
                     "PrimaryUrl TEXT NULL,"
                     "BackupUrls TEXT NULL,"
                     "OriginSavePath TEXT NULL,"
                     "Memo TEXT NULL"
                     ")");
    Utils::printQSqlQueryLastQuery(query);
    if (!ret)
    {
        qDebug() << "GameDB::resetTables: Error: Create table tbl_gameInfo failed.";
        Utils::printQSqlQueryLastError(query);
        return false;
    }
    ret = query.exec("CREATE TABLE tbl_savesData ("
                     "ID INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,"
                     "GameID INTEGER NOT NULL,"
                     "Hash TEXT NOT NULL,"
                     "Data BLOB NOT NULL,"
                     "Datetime TEXT NOT NULL,"
                     "UserDatetime TEXT NULL,"
                     "Branch TEXT NULL,"
                     "Memo TEXT NULL,"
                     "Tag TEXT NULL"
                     ")");
    if (!ret)
    {
        qDebug() << "GameDB::resetTables: Error: Create table tbl_savesData failed.";
        Utils::printQSqlQueryLastError(query);
        return false;
    }
    Utils::printQSqlQueryLastQuery(query);
    return true;
}

