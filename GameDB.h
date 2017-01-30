/*****************************************************************************
 * GameDB.h: GameDB header file.
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

#ifndef GAMEDB_H
#define GAMEDB_H

#include <QObject>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlTableModel>
#include <QPointer>
#include <QUrl>

class GameDB : public QObject
{
    Q_OBJECT

public:
    enum GameSaveType
    {
        UnknownSave = -1,
        InternalBlob = 0,
        InternalText = 1,
        SingleFile = 2,
        MultiFile = 3,
    };
    Q_ENUMS(GameSaveType)

    enum Field_tbl_gameInfo
    {
        TBL_gameInfo_ID = 0,
        TBL_gameInfo_Name = 1,
        TBL_gameInfo_SaveType = 2,
        TBL_gameInfo_PrimaryUrl = 3,
        TBL_gameInfo_BackupUrls = 4,
        TBL_gameInfo_OriginSavePath = 5,
        TBL_gameInfo_Memo = 6,
    };

    enum Field_tbl_savesData
    {
        TBL_savesData_ID = 0,
        TBL_savesData_GameID = 1,
        TBL_savesData_Hash = 2,
        TBL_savesData_Data = 3,
        TBL_savesData_DateTime = 4,
        TBL_savesData_UserDateTime = 5,
        TBL_savesData_Tag = 6,
        TBL_savesData_Branch = 7,
        TBL_savesData_Memo = 8,
    };

    explicit GameDB(QObject *parent = 0);
    ~GameDB();

//    void addGame();
//    void addGame(const QString &name,
//                 enum GameSaveType type,
//                 const QUrl &primaryUrl,
//                 const QList<QUrl> &backupUrls,
//                 const QString &originSavePath,
//                 const QString &memo = QString());
//    void addTextSaveData(int gameId,
//                         const QString &hash,
//                         const QDateTime &userDateTime,
//                         const QString &branch,
//                         const QString &memo = QString());

    bool startTransaction()
    {
        return m_db.transaction();
    }

    bool commit()
    {
        return m_db.commit();
    }

    bool rollback()
    {
        return m_db.rollback();
    }

    QPointer<QSqlTableModel> m_gameInfo;
    QPointer<QSqlTableModel> m_savesData;
    QSqlDatabase m_db;
    static QPointer<GameDB> m_gameDb;

public slots:

signals:

private:
    bool createConnection();
    bool initialDatabase();
    bool resetTables();

};

#endif // GAMEDB_H
