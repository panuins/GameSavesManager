/*****************************************************************************
 * mainwindow.cpp: main window header file.
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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "GameDB.h"
#include "GameInfo.h"
#include "SavesData.h"
#include "Utils.h"
#include <QMessageBox>
#include <QSqlQuery>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->listView_games->setModel(GameDB::m_gameDb->m_gameInfo.data());
    ui->tableView_Saves->setModel(GameDB::m_gameDb->m_savesData.data());
    GameDB::m_gameDb->m_gameInfo->select();
    ui->listView_games->setModelColumn(GameDB::TBL_gameInfo_Name);

    ui->tableView_Saves->hideColumn(GameDB::TBL_savesData_ID);
    ui->tableView_Saves->hideColumn(GameDB::TBL_savesData_GameID);
    ui->tableView_Saves->hideColumn(GameDB::TBL_savesData_Data);
    int gameId = currentGameId();
    if (gameId >= 0)
    {
        GameDB::m_gameDb->m_savesData->setFilter(
                    QString("GameID=%1").arg(gameId));
        GameDB::m_gameDb->m_savesData->select();
    }
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 8);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_listView_games_activated(const QModelIndex &index)
{
    if (index.isValid())
    {
//        QVariant var = GameDB::m_gameDb->m_gameInfo->data(
//                    GameDB::m_gameDb->m_gameInfo->index(index.row(), 0));
//        if (var.isValid())
//        {
//            int gameId = var.toInt();
//            GameDB::m_gameDb->m_savesData->setFilter(QString("GameID=%1").arg(gameId));
//            GameDB::m_gameDb->m_savesData->select();
//        }
        int gameId = currentGameId();
        if (gameId >= 0)
        {
            GameDB::m_gameDb->m_savesData->setFilter(
                        QString("GameID=%1").arg(gameId));
            GameDB::m_gameDb->m_savesData->select();
        }
    }
}

void MainWindow::on_listView_games_clicked(const QModelIndex &index)
{
    on_listView_games_activated(index);
}

void MainWindow::on_listView_games_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    on_pushButton_Games_Edit_clicked();
}

void MainWindow::on_tableView_Saves_doubleClicked(const QModelIndex &index)
{
    Q_UNUSED(index)
    on_pushButton_Saves_Edit_clicked();
}

void MainWindow::on_pushButton_Games_Add_clicked()
{
    GameInfo::showAdd(GameDB::m_gameDb->m_db, GameDB::m_gameDb->m_gameInfo);
}

void MainWindow::on_pushButton_Games_Edit_clicked()
{
    if (ui->listView_games->currentIndex().isValid())
    {
        GameInfo::showEdit(GameDB::m_gameDb->m_db,
                           GameDB::m_gameDb->m_gameInfo,
                           ui->listView_games->currentIndex());
    }
}

void MainWindow::on_pushButton_Games_Remove_clicked()
{
    int gameId = currentGameId();
    if (gameId < 0)
    {
        QMessageBox::critical(this, tr("Select a game"),
                              tr("Please select a game."));
        return;
    }
    QString name;
    QString primaryUrl;
    {
        QSqlQuery query(GameDB::m_gameDb->m_db);
        QString str("select * from tbl_gameinfo where ID=%1");
        str=str.arg(gameId);
        bool ret=query.exec(str);
        if (!ret)
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "query game info failed.";
            QMessageBox::critical(this, tr("Delete game"),
                                  tr("Query game info failed. Last query %1.")
                                  .arg(str));
            Utils::printQSqlQueryLastError(query);
            return;
        }
        query.first();
        name = query.value(GameDB::TBL_gameInfo_Name).toString();
        primaryUrl = query.value(GameDB::TBL_gameInfo_PrimaryUrl).toString();

        Utils::printQSqlQueryLastQuery(query);
    }
    enum QMessageBox::StandardButton ret = QMessageBox::question(
                this, tr("Remove a game"),
                tr("Are you sure remove this game(name=%1, primaryUrl=%2) and all of its saves?")
                .arg(name).arg(primaryUrl));
    if ((ret == QMessageBox::Ok) || (ret == QMessageBox::Yes))
    {
        GameDB::m_gameDb->m_db.transaction();
        QSqlQuery query(GameDB::m_gameDb->m_db);
        QString str("delete from tbl_savesData where GameID=%1");
        QString str1("delete from tbl_gameinfo where ID=%1");
        str=str.arg(gameId);
        bool ret=query.exec(str);
        if (!ret)
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "delete from tbl_savesData failed.";
            Utils::printQSqlQueryLastError(query);
            GameDB::m_gameDb->m_db.rollback();
            return;
        }
        Utils::printQSqlQueryLastQuery(query);
        str1=str1.arg(gameId);
        ret=query.exec(str1);
        if (!ret)
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "delete from tbl_gameinfo.";
            Utils::printQSqlQueryLastError(query);
            GameDB::m_gameDb->m_db.rollback();
            return;
        }
        Utils::printQSqlQueryLastQuery(query);
        GameDB::m_gameDb->m_db.commit();
        GameDB::m_gameDb->m_gameInfo->select();
        GameDB::m_gameDb->m_savesData->select();
    }
}

void MainWindow::on_pushButton_Saves_Add_clicked()
{
    if (GameDB::m_gameDb.isNull())
    {
        return;
    }
    if (GameDB::m_gameDb->m_savesData.isNull())
    {
        return;
    }
    int gameId = currentGameId();
    if (gameId >= 0)
    {
        SavesData::showAdd(GameDB::m_gameDb->m_db,
                           GameDB::m_gameDb->m_savesData,
                           currentGameId());
    }
    else
    {
        QMessageBox::critical(this, tr("No game selected"),
                              tr("Error: you must select a game."));
    }
}

void MainWindow::on_pushButton_Saves_Edit_clicked()
{
    if (GameDB::m_gameDb.isNull())
    {
        return;
    }
    if (GameDB::m_gameDb->m_savesData.isNull())
    {
        return;
    }
    if (ui->tableView_Saves->currentIndex().isValid())
    {
        SavesData::showEdit(GameDB::m_gameDb->m_db,
                            GameDB::m_gameDb->m_savesData,
                            ui->tableView_Saves->currentIndex());
    }
}

void MainWindow::on_pushButton_Saves_Commit_clicked()
{
    if (GameDB::m_gameDb.isNull())
    {
        return;
    }
    if (GameDB::m_gameDb->m_savesData.isNull())
    {
        return;
    }
    GameDB::m_gameDb->m_savesData->submitAll();
    GameDB::m_gameDb->commit();
}

void MainWindow::on_pushButton_Saves_Remove_clicked()
{
    if (!ui->tableView_Saves->currentIndex().isValid())
    {
        QMessageBox::critical(this, tr("Select a save"),
                              tr("Please select a save."));
        return;
    }
    int index=ui->tableView_Saves->currentIndex().row();
    QString hash;
    QString branch;
    QString date;
    if (GameDB::m_gameDb.isNull())
    {
        return;
    }
    if (GameDB::m_gameDb->m_savesData.isNull())
    {
        return;
    }

    QSqlTableModel *savesModel = GameDB::m_gameDb->m_savesData.data();
    {
        QVariant hashVar = savesModel->data(
                    savesModel->index(
                        index, GameDB::TBL_savesData_Hash));
        if (!hashVar.isValid())
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "query hash failed.";
            QMessageBox::critical(this, tr("Delete a game"),
                                  tr("Query hash failed."));
            Utils::printQSqlQueryLastError(savesModel->query());
            return;
        }
        Utils::printQSqlQueryLastQuery(savesModel->query());
        QVariant branchVar = savesModel->data(
                    savesModel->index(
                        index, GameDB::TBL_savesData_Branch));
        if (!branchVar.isValid())
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "query save info failed.";
            QMessageBox::critical(this, tr("Delete a game"),
                                  tr("Query branch failed."));
            Utils::printQSqlQueryLastError(savesModel->query());
            return;
        }
        Utils::printQSqlQueryLastQuery(savesModel->query());
        QVariant dateVar = savesModel->data(
                    savesModel->index(
                        index, GameDB::TBL_savesData_UserDateTime));
        if (!dateVar.isValid())
        {
            qDebug() << "MainWindow::on_pushButton_Games_Remove_clicked: "
                        "query save info failed.";
            QMessageBox::critical(this, tr("Delete a game"),
                                  tr("Query user date time failed."));
            Utils::printQSqlQueryLastError(savesModel->query());
            return;
        }
        Utils::printQSqlQueryLastQuery(savesModel->query());
        hash=hashVar.toString();
        branch=branchVar.toString();
        date=dateVar.toString();
    }
    enum QMessageBox::StandardButton ret = QMessageBox::question(
                this, tr("Remove a save"),
                tr("Are you sure remove this save?(branch=%1, UserDateTime=%2, \nhash=%3)")
                .arg(branch).arg(date).arg(hash));
    if ((ret == QMessageBox::Ok) || (ret == QMessageBox::Yes))
    {
        savesModel->removeRow(index);
        savesModel->select();
    }
}

int MainWindow::currentGameId() const
{
    if (!ui->listView_games->currentIndex().isValid())
    {
        return -1;
    }
    if (GameDB::m_gameDb.isNull())
    {
        return -1;
    }
    if (GameDB::m_gameDb->m_gameInfo.isNull())
    {
        return -1;
    }

    QSqlTableModel *gameInfo = GameDB::m_gameDb->m_gameInfo.data();
    QVariant var = gameInfo->data(
                gameInfo->index(
                    ui->listView_games->currentIndex().row(), 0));
    if (var.isValid())
    {
        Utils::printQSqlQueryLastQuery(gameInfo->query());
        return var.toInt();
    }
    Utils::printQSqlQueryLastError(gameInfo->query());
    return -1;
}
