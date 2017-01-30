/*****************************************************************************
 * GameInfo.cpp: GameDB info dialog source file.
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

#include "GameInfo.h"
#include "ui_GameInfo.h"
#include "GameDB.h"
#include "Utils.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlTableModel>

GameInfo::GameInfo(QSqlDatabase db, QSqlTableModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GameInfo),
    m_model(model),
    m_mapper(NULL),
    m_db(db),
    m_state(Inited),
    m_row(-1)
{
    db.transaction();
    //m_model
    ui->setupUi(this);
    //updateUi();
}

GameInfo::~GameInfo()
{
    delete ui;
}

int GameInfo::showAdd(QSqlDatabase db, QSqlTableModel *model)
{
    GameInfo info(db, model);
    info.blockEditors();
    info.ui->pushButton_GameInfo_First->setEnabled(false);
    info.ui->pushButton_GameInfo_Last->setEnabled(false);
    info.ui->pushButton_GameInfo_Next->setEnabled(false);
    info.ui->pushButton_GameInfo_Previous->setEnabled(false);
    //model->insertRow()
    info.ui->comboBox_GameInfo_SaveType->setEnabled(true);
    info.ui->pushButton_GameInfo_Lock->setEnabled(false);
    info.m_state = Inited;
    info.unblockEditors();
    info.ui->pushButton_GameInfo_Lock->setChecked(false);
    info.ui->pushButton_GameInfo_Commit->setEnabled(false);
    int ret = info.exec();
    //QSqlQuery query("insert into tbl_gameInfo");
    return ret;
}

void GameInfo::showEdit(QSqlDatabase db, QSqlTableModel *model,
                        const QModelIndex &id)
{
    GameInfo info(db, model);
    info.blockEditors();
    //info.m_model = new QSqlTableModel(info);
    info.m_row = id.row();
    info.m_mapper = new QDataWidgetMapper(&info);
    info.m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    info.m_mapper->setModel(model);
//    TBL_gameInfo_ID = 0,
//    TBL_gameInfo_Name = 1,
//    TBL_gameInfo_SaveType = 2,
//    TBL_gameInfo_PrimaryUrl = 3,
//    TBL_gameInfo_BackupUrls = 4,
//    TBL_gameInfo_OriginSavePath = 5,
//    TBL_gameInfo_Memo = 6,
    info.m_mapper->addMapping(info.ui->lineEdit_GameInfo_Name, GameDB::TBL_gameInfo_Name);
    info.m_mapper->addMapping(info.ui->comboBox_GameInfo_SaveType, GameDB::TBL_gameInfo_SaveType, "currentIndex");
    info.m_mapper->addMapping(info.ui->lineEdit_GameInfo_PrimaryUrl, GameDB::TBL_gameInfo_PrimaryUrl);
    info.m_mapper->addMapping(info.ui->plainTextEdit_GameInfo_BackupUrls, GameDB::TBL_gameInfo_BackupUrls);
    info.m_mapper->addMapping(info.ui->plainTextEdit_GameInfo_OriginSavePath, GameDB::TBL_gameInfo_OriginSavePath);
    info.m_mapper->addMapping(info.ui->plainTextEdit_GameInfo_Memo, GameDB::TBL_gameInfo_Memo);
    info.m_mapper->setCurrentModelIndex(id);
    info.ui->comboBox_GameInfo_SaveType->setEnabled(false);
    info.ui->pushButton_GameInfo_Lock->setEnabled(true);
    info.updateUi();
    info.m_state = Inited;
    info.unblockEditors();
    info.exec();
    //QSqlQuery query("insert into tbl_gameInfo");
    //return ret;
}

void GameInfo::closeEvent(QCloseEvent *e)
{
    if (m_state == Inited)
    {
        if (m_mapper != NULL)
        {
            m_mapper->revert();
        }
        m_db.rollback();
        m_model->select();
        this->rejected();
        QDialog::closeEvent(e);
    }
    else if (m_state != Committed)
    {
        enum QMessageBox::StandardButton ret = QMessageBox::question(
                    NULL, tr("rollback"),
                    tr("All data changed will be lost, are you sure?"));
        if ((ret == QMessageBox::Ok) || (ret == QMessageBox::Yes))
        {
            if (m_mapper != NULL)
            {
                m_mapper->revert();
            }
            m_db.rollback();
            m_model->select();
            this->rejected();
            QDialog::closeEvent(e);
        }
        else
        {
            e->ignore();
        }
    }
    else
    {
        QDialog::closeEvent(e);
    }
}

void GameInfo::on_pushButton_GameInfo_Commit_clicked()
{
    if (m_row >= 0)
    {
    }
    else
    {
    }
    if (m_mapper != NULL)
    {
        m_mapper->submit();
        m_model->submitAll();
    }
    else
    {
        QSqlQuery query(m_db);
        bool ret = query.prepare("insert into tbl_gameInfo VALUES(NULL, ?, ?, ?, ?, ?, ?)");
        if (!ret)
        {
            qDebug() << "GameInfo::on_pushButton_GameInfo_Commit_clicked: Error: "
                        "prepare insert into tbl_gameInfo failed.";
            Utils::printQSqlQueryLastError(query);
        }
        query.addBindValue(ui->lineEdit_GameInfo_Name->text());
        query.addBindValue(ui->comboBox_GameInfo_SaveType->currentIndex());
        query.addBindValue(ui->lineEdit_GameInfo_PrimaryUrl->text());
        query.addBindValue(ui->plainTextEdit_GameInfo_BackupUrls->toPlainText());
        query.addBindValue(ui->plainTextEdit_GameInfo_OriginSavePath->toPlainText());
        query.addBindValue(ui->plainTextEdit_GameInfo_Memo->toPlainText());
        ret = query.exec();
        if (!ret)
        {
            qDebug() << "GameInfo::on_pushButton_GameInfo_Commit_clicked: Error: "
                        "insert into tbl_gameInfo failed.";
            Utils::printQSqlQueryLastError(query);
        }
        Utils::printQSqlQueryLastQuery(query);
    }
    m_db.commit();
    m_model->select();
    if (m_row < 0)
    {
        m_row = m_model->rowCount()-1;
    }
    m_state = Inited;
    m_db.transaction();
}

void GameInfo::on_pushButton_GameInfo_Ok_clicked()
{
    if (m_row >= 0)
    {
    }
    else
    {
    }
    if (m_mapper != NULL)
    {
        m_mapper->submit();
        m_model->submitAll();
    }
    else
    {
        QSqlQuery query(m_db);
        bool ret = query.prepare("insert into tbl_gameInfo VALUES(NULL, ?, ?, ?, ?, ?, ?)");
        if (!ret)
        {
            qDebug() << "GameInfo::on_pushButton_GameInfo_Ok_clicked: Error: "
                        "prepare insert into tbl_gameInfo failed.";
            Utils::printQSqlQueryLastError(query);
        }
        query.addBindValue(ui->lineEdit_GameInfo_Name->text());
        query.addBindValue(ui->comboBox_GameInfo_SaveType->currentIndex());
        query.addBindValue(ui->lineEdit_GameInfo_PrimaryUrl->text());
        query.addBindValue(ui->plainTextEdit_GameInfo_BackupUrls->toPlainText());
        query.addBindValue(ui->plainTextEdit_GameInfo_OriginSavePath->toPlainText());
        query.addBindValue(ui->plainTextEdit_GameInfo_Memo->toPlainText());
        ret = query.exec();
        if (!ret)
        {
            qDebug() << "GameInfo::on_pushButton_GameInfo_Ok_clicked: Error: "
                        "insert into tbl_gameInfo failed.";
            Utils::printQSqlQueryLastError(query);
        }
        Utils::printQSqlQueryLastQuery(query);
    }
    m_db.commit();
    m_model->select();
    if (m_row < 0)
    {
        m_row = m_model->rowCount()-1;
    }
    //this->done(m_row+1);
    accept();
    m_state = Committed;
    close();
}

void GameInfo::on_pushButton_GameInfo_Cancel_clicked()
{
    close();
}

void GameInfo::on_pushButton_GameInfo_First_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toFirst();
        updateUi();
        unblockEditors();
    }
}

void GameInfo::on_pushButton_GameInfo_Previous_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toPrevious();
        updateUi();
        unblockEditors();
    }
}

void GameInfo::on_pushButton_GameInfo_Next_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toNext();
        updateUi();
        unblockEditors();
    }
}

void GameInfo::on_pushButton_GameInfo_Last_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toLast();
        updateUi();
        unblockEditors();
    }
}

void GameInfo::on_toolButton_GameInfo_OriginSavePath_clicked()
{
    QStringList list = QFileDialog::getOpenFileNames(this, tr("Select files"));
    foreach (QString str, list)
    {
        ui->plainTextEdit_GameInfo_BackupUrls->appendPlainText(str);
    }
}

void GameInfo::on_lineEdit_GameInfo_Name_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    markDataChanged();
}

void GameInfo::on_comboBox_GameInfo_SaveType_currentIndexChanged(int index)
{
    Q_UNUSED(index)
    markDataChanged();
}

void GameInfo::on_pushButton_GameInfo_Lock_toggled(bool checked)
{
    if (!checked)
    {
        if ((m_mapper != NULL) && (m_model != NULL))
        {
            QSqlQuery query(m_db);
            QString str("select count(*) from tbl_savesdata where GameID=%1");
            int row = m_mapper->currentIndex();
            bool ok = false;
            int id = m_model->data(m_model->index(row, 0)).toInt(&ok);
            if (!ok)
            {
                qDebug() << "GameInfo::on_pushButton_GameInfo_Lock_toggled: "
                            "get gameid failed.";
                Utils::printQSqlQueryLastError(m_model->query());
                ui->pushButton_GameInfo_Lock->setChecked(true);
                return;
            }
            str=str.arg(id);
            bool ret = query.exec(str);
            if (!ret)
            {
                qDebug() << "GameInfo::on_pushButton_GameInfo_Lock_toggled: "
                            "get saves record count failed.";
                Utils::printQSqlQueryLastError(query);
                ui->pushButton_GameInfo_Lock->setChecked(true);
                return;
            }
            query.first();
            int count = query.value(0).toInt(&ok);
            if (!ok)
            {
                qDebug() << "GameInfo::on_pushButton_GameInfo_Lock_toggled: "
                            "get gameid failed.";
                Utils::printQSqlQueryLastError(m_model->query());
                ui->pushButton_GameInfo_Lock->setChecked(true);
                return;
            }
            if (count > 0)
            {
                enum QMessageBox::StandardButton ret = QMessageBox::question(
                            this, tr("change save type"),
                            tr("Change save type may damage existing saves, are you sure?"));
                if ((ret == QMessageBox::Ok) || (ret == QMessageBox::Yes))
                {
                }
                else
                {
                    ui->pushButton_GameInfo_Lock->setChecked(true);
                    ui->comboBox_GameInfo_SaveType->setEnabled(false);
                    return;
                }
            }
            ui->comboBox_GameInfo_SaveType->setEnabled(true);
        }
    }
    else
    {
        ui->comboBox_GameInfo_SaveType->setEnabled(false);
    }
}

void GameInfo::on_lineEdit_GameInfo_PrimaryUrl_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    markDataChanged();
}

void GameInfo::on_plainTextEdit_GameInfo_BackupUrls_textChanged()
{
    markDataChanged();
}

void GameInfo::on_plainTextEdit_GameInfo_OriginSavePath_textChanged()
{
    markDataChanged();
}

void GameInfo::on_plainTextEdit_GameInfo_Memo_textChanged()
{
    markDataChanged();
}

void GameInfo::updateUi()
{
    if ((m_mapper != NULL) && (m_model != NULL))
    {
        int index = m_mapper->currentIndex();
        if (index <= 0)
        {
            ui->pushButton_GameInfo_First->setEnabled(false);
            ui->pushButton_GameInfo_Previous->setEnabled(false);
        }
        else
        {
            ui->pushButton_GameInfo_First->setEnabled(true);
            ui->pushButton_GameInfo_Previous->setEnabled(true);
        }
        if (index >= m_model->rowCount()-1)
        {
            ui->pushButton_GameInfo_Last->setEnabled(false);
            ui->pushButton_GameInfo_Next->setEnabled(false);
        }
        else
        {
            ui->pushButton_GameInfo_Last->setEnabled(true);
            ui->pushButton_GameInfo_Next->setEnabled(true);
        }
    }
}

void GameInfo::markDataChanged()
{
    if (!this->windowTitle().endsWith("*"))
    {
        this->setWindowTitle(this->windowTitle()+"*");
    }
    m_state = Editing;
}

void GameInfo::blockEditors()
{
    ui->lineEdit_GameInfo_Name->blockSignals(true);
    ui->lineEdit_GameInfo_PrimaryUrl->blockSignals(true);
    ui->comboBox_GameInfo_SaveType->blockSignals(true);
    ui->plainTextEdit_GameInfo_BackupUrls->blockSignals(true);
    ui->plainTextEdit_GameInfo_Memo->blockSignals(true);
    ui->plainTextEdit_GameInfo_OriginSavePath->blockSignals(true);
}

void GameInfo::unblockEditors()
{
    ui->lineEdit_GameInfo_Name->blockSignals(false);
    ui->lineEdit_GameInfo_PrimaryUrl->blockSignals(false);
    ui->comboBox_GameInfo_SaveType->blockSignals(false);
    ui->plainTextEdit_GameInfo_BackupUrls->blockSignals(false);
    ui->plainTextEdit_GameInfo_Memo->blockSignals(false);
    ui->plainTextEdit_GameInfo_OriginSavePath->blockSignals(false);
}
