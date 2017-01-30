/*****************************************************************************
 * SaveData.cpp: saves data dialog source file.
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

#include "SavesData.h"
#include "ui_SavesData.h"
#include "Utils.h"
#include <QCryptographicHash>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSqlQuery>
#include <QSignalBlocker>
#include <QTimer>

SavesData::SavesData(QSqlDatabase db, QSqlTableModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SavesData),
    m_model(model),
    m_mapper(NULL),
    m_editor(NULL),
    m_db(db),
    m_saveType(GameDB::UnknownSave),
    m_state(Inited),
    m_row(-1),
    m_gameId(0)
{
    db.transaction();
    ui->setupUi(this);
    updateUi();
    //m_timer->setInterval(100);
    ui->plainTextEdit_SavesData_SaveData->installEventFilter(this);
}

SavesData::~SavesData()
{
    delete ui;
}

int SavesData::showAdd(QSqlDatabase db, QSqlTableModel *model, int gameId)
{
    SavesData dialog(db, model);
    dialog.blockEditors();
    dialog.m_gameId = gameId;
    dialog.ui->pushButton_SavesData_First->setEnabled(false);
    dialog.ui->pushButton_SavesData_Last->setEnabled(false);
    dialog.ui->pushButton_SavesData_Next->setEnabled(false);
    dialog.ui->pushButton_SavesData_Previous->setEnabled(false);
    QDateTime t = QDateTime::currentDateTime();
    QString dateTimeStr = t.toString("yyyy-MM-dd");
    dialog.ui->lineEdit_SavesData_Date->setText(dateTimeStr);
    dialog.unblockEditors();
    dialog.ui->pushButton_SavesData_Lock->setChecked(false);
    dialog.ui->pushButton_SavesData_Commit->setEnabled(false);
    dialog.ui->label_SavesData_CurrentGameNameValue->setText(dialog.queryGameName());
    int ret = dialog.exec();
    return ret;
}

void SavesData::showEdit(QSqlDatabase db, QSqlTableModel *model, const QModelIndex &id)
{
    SavesData dialog(db, model);

    dialog.blockEditors();
    dialog.m_row = id.row();
    dialog.m_mapper = new QDataWidgetMapper(&dialog);
    dialog.m_mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    dialog.m_mapper->setModel(model);
//    TBL_savesData_ID = 0,
//    TBL_savesData_GameID = 1,
//    TBL_savesData_Hash = 2,
//    TBL_savesData_Data = 3,
//    TBL_savesData_DateTime = 4,
//    TBL_savesData_UserDateTime = 5,
//    TBL_savesData_Branch = 6,
//    TBL_savesData_Memo = 7,
    dialog.m_mapper->addMapping(dialog.ui->label_SavesData_HashValue, GameDB::TBL_savesData_Hash, "text");
    dialog.m_mapper->addMapping(dialog.ui->label_SavesData_LastDateTimeValue, GameDB::TBL_savesData_DateTime, "text");
    //dialog.m_mapper->addMapping(ui->lineEdit_SavesData_Branch, GameDB::TBL_savesData_Branch);
    dialog.m_mapper->addMapping(dialog.ui->lineEdit_SavesData_Date, GameDB::TBL_savesData_UserDateTime);
    dialog.m_mapper->addMapping(dialog.ui->lineEdit_SavesData_Tag, GameDB::TBL_savesData_Tag);
    dialog.m_mapper->addMapping(dialog.ui->lineEdit_SavesData_Branch, GameDB::TBL_savesData_Branch);
    dialog.m_mapper->addMapping(dialog.ui->plainTextEdit_SavesData_Memo, GameDB::TBL_savesData_Memo);
    dialog.m_mapper->setCurrentModelIndex(id);
    dialog.updateUi();
    dialog.m_state = Inited;
    dialog.unblockEditors();
    dialog.exec();
    //return ret;
}

void SavesData::closeEvent(QCloseEvent *e)
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

bool SavesData::eventFilter(QObject * watched, QEvent * event)
{
    QPlainTextEdit *edit = dynamic_cast<QPlainTextEdit *>(watched);
    if ((edit != NULL) && (event != NULL))
    {
        switch (event->type())
        {
        case QEvent::FocusIn:
            m_editor = edit;
            //edit->selectAll();
            //edit->setFocus(Qt::OtherFocusReason);
            QTimer::singleShot(100, this, SLOT(on_timer_timeout()));
            break;
        default:
            break;
        }
    }
    return QObject::eventFilter(watched, event);
}

void SavesData::on_pushButton_SavesData_Commit_clicked()
{
    ui->pushButton_SavesData_Lock->setChecked(true);
//    if (!ui->pushButton_SavesData_Lock->isChecked())
//    {
//        writeSaveData();
//    }
    if (m_mapper != NULL)
    {
        m_mapper->submit();
        m_model->submitAll();
    }
    else
    {
        QSqlQuery query(m_db);
        bool ret = query.prepare("insert into tbl_savesData VALUES(NULL, ?, ?, ?, ?, ?, ?, ?, ?)");
        if (!ret)
        {
            qDebug() << "SavesData::on_pushButton_SavesData_Commit_clicked: Error: "
                        "prepare insert into tbl_savesData failed.";
            Utils::printQSqlQueryLastError(query);
        }
        SaveDataBundle d = currentSaveData();
//        qDebug() << "SavesData::on_pushButton_SavesData_Commit_clicked: "
//                 << d.data.toHex() << d.dateTimeStr << d.hashText;
        query.addBindValue(m_gameId);
        query.addBindValue(d.hashText);
        query.addBindValue(QVariant(d.data));
        query.addBindValue(d.dateTimeStr);
        query.addBindValue(ui->lineEdit_SavesData_Date->text());
        query.addBindValue(ui->lineEdit_SavesData_Tag->text());
        query.addBindValue(ui->lineEdit_SavesData_Branch->text());
        query.addBindValue(ui->plainTextEdit_SavesData_Memo->toPlainText());
        ret = query.exec();
        if (!ret)
        {
            qDebug() << "SavesData::on_pushButton_SavesData_Commit_clicked: Error: "
                        "insert into tbl_savesData failed.";
            Utils::printQSqlQueryLastError(query);
        }
        Utils::printQSqlQueryLastQuery(query);
    }
    m_db.commit();
    m_state = Inited;
    m_db.transaction();
}

void SavesData::on_pushButton_SavesData_Ok_clicked()
{
    ui->pushButton_SavesData_Lock->setChecked(true);
//    if (!ui->pushButton_SavesData_Lock->isChecked())
//    {
//        writeSaveData();
//    }
    if (m_mapper != NULL)
    {
        m_mapper->submit();
        m_model->submitAll();
    }
    else
    {
        QSqlQuery query(m_db);
        bool ret = query.prepare("insert into tbl_savesData VALUES(NULL, ?, ?, ?, ?, ?, ?, ?, ?)");
        if (!ret)
        {
            qDebug() << "SavesData::on_pushButton_SavesData_Ok_clicked: Error: "
                        "prepare insert into tbl_savesData failed.";
            Utils::printQSqlQueryLastError(query);
        }
        SaveDataBundle d = currentSaveData();
//        qDebug() << "SavesData::on_pushButton_SavesData_Ok_clicked: "
//                 << d.data.toHex() << d.dateTimeStr << d.hashText;
        query.addBindValue(m_gameId);
        query.addBindValue(d.hashText);
        query.addBindValue(QVariant(d.data));
        query.addBindValue(d.dateTimeStr);
        query.addBindValue(ui->lineEdit_SavesData_Date->text());
        query.addBindValue(ui->lineEdit_SavesData_Tag->text());
        query.addBindValue(ui->lineEdit_SavesData_Branch->text());
        query.addBindValue(ui->plainTextEdit_SavesData_Memo->toPlainText());
        ret = query.exec();
        if (!ret)
        {
            qDebug() << "SavesData::on_pushButton_SavesData_Ok_clicked: Error: "
                        "insert into tbl_savesData failed.";
            Utils::printQSqlQueryLastError(query);
        }
        Utils::printQSqlQueryLastQuery(query);
    }
    m_db.commit();
    m_model->select();
    accept();
    m_state = Committed;
    close();
}

void SavesData::on_pushButton_SavesData_Cancel_clicked()
{
    close();
}

void SavesData::on_pushButton_SavesData_First_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toFirst();
        updateUi();
        unblockEditors();
    }
}

void SavesData::on_pushButton_SavesData_Previous_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toPrevious();
        updateUi();
        unblockEditors();
    }
}

void SavesData::on_pushButton_SavesData_Next_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toNext();
        updateUi();
        unblockEditors();
    }
}

void SavesData::on_pushButton_SavesData_Last_clicked()
{
    if (m_mapper != NULL)
    {
        blockEditors();
        m_mapper->toLast();
        updateUi();
        unblockEditors();
    }
}

void SavesData::on_pushButton_SavesData_Import_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("select file"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        bool ret = file.open(QIODevice::ReadOnly);
        if (!ret)
        {
            QMessageBox::critical(this, tr("Couldn't open file for read"),
                                  tr("Error: Open file %1 failed, please check whether you have enough permission.")
                                  .arg(fileName));
            return;
        }
        QByteArray data = file.readAll();
        enum GameDB::GameSaveType saveType = querySaveType();
        switch (saveType)
        {
        case GameDB::InternalBlob:
        {
            QString text = QString::fromUtf8(data.toHex());
            ui->plainTextEdit_SavesData_SaveData->setPlainText(text);
            break;
        }
        case GameDB::InternalText:
        {
            QString text = QString::fromUtf8(data);
            ui->plainTextEdit_SavesData_SaveData->setPlainText(text);
            break;
        }
        default:
            qDebug() << "SavesData::on_pushButton_SavesData_Import_clicked: Error: "
                        "unsupported save type for import"
                     << saveType;
        }
    }
}

void SavesData::on_pushButton_SavesData_Export_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("select file"));
    if (!fileName.isEmpty())
    {
        QFile file(fileName);
        SaveDataBundle data = currentSaveData();
        bool ret = file.open(QIODevice::WriteOnly);
        if (!ret)
        {
            QMessageBox::critical(this, tr("Couldn't open file for write"),
                                  tr("Error: Open file %1 failed, please check whether you have enough permission.")
                                  .arg(fileName));
            return;
        }
        file.write(data.data);
    }
}

void SavesData::on_pushButton_SavesData_Restore_clicked()
{
}

void SavesData::on_pushButton_SavesData_Overwrite_clicked()
{
    //
}

void SavesData::on_pushButton_SavesData_Lock_toggled(bool checked)
{
    if (checked)
    {
        //switch to lock state
        ui->plainTextEdit_SavesData_SaveData->setReadOnly(true);
        ui->pushButton_SavesData_Lock->setText(tr("Un&lock"));
        writeSaveData();
        updateUi();
        markDataChanged();
    }
    else
    {
        //switch to unlock state
        ui->pushButton_SavesData_First->setEnabled(false);
        ui->pushButton_SavesData_Previous->setEnabled(false);
        ui->pushButton_SavesData_Last->setEnabled(false);
        ui->pushButton_SavesData_Next->setEnabled(false);
        ui->plainTextEdit_SavesData_SaveData->setReadOnly(false);
        ui->pushButton_SavesData_Lock->setText(tr("&Lock"));
    }
}

void SavesData::on_lineEdit_SavesData_Tag_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    markDataChanged();
}

void SavesData::on_lineEdit_SavesData_Branch_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    markDataChanged();
}

void SavesData::on_lineEdit_SavesData_Date_textChanged(const QString &arg1)
{
    Q_UNUSED(arg1)
    markDataChanged();
}

void SavesData::on_plainTextEdit_SavesData_Memo_textChanged()
{
    markDataChanged();
}

void SavesData::on_timer_timeout()
{
    if (m_editor != NULL)
    {
        m_editor->selectAll();
    }
}

SavesData::SaveDataBundle SavesData::currentSaveData() const
{
    SaveDataBundle ret;
    QDateTime t = QDateTime::currentDateTime();
    ret.dateTimeStr = t.toString("yyyy-MM-dd hh:mm:ss");
    {
        enum GameDB::GameSaveType saveType = querySaveType();
        switch (saveType)
        {
        case GameDB::InternalBlob:
        {
            QByteArray hexData = ui->plainTextEdit_SavesData_SaveData->toPlainText().toUtf8();
            ret.data = QByteArray::fromHex(hexData);
            QByteArray hashData = QCryptographicHash::hash(ret.data, QCryptographicHash::Sha512);
            ret.hashText = QString::fromUtf8(hashData.toHex());
            break;
        }
        case GameDB::InternalText:
        {
            ret.data = ui->plainTextEdit_SavesData_SaveData->toPlainText().toUtf8();
            QByteArray hashData = QCryptographicHash::hash(ret.data, QCryptographicHash::Sha512);
            ret.hashText = QString::fromUtf8(hashData.toHex());
            break;
        }
        case GameDB::SingleFile:
        {
            //ret.dateTimeStr = QString();
            break;
        }
        case GameDB::MultiFile:
        {
            ret.data = ui->plainTextEdit_SavesData_SaveData->toPlainText().toUtf8();
            QByteArray hashData = QCryptographicHash::hash(ret.data, QCryptographicHash::Sha512);
            ret.hashText = QString::fromUtf8(hashData.toHex());
            break;
        }
        default:
            qDebug() << "SavesData::currentSaveData: Unknown SaveType" << saveType;
        }
    }
    return ret;
}

void SavesData::updateUi()
{
    if ((m_mapper != NULL) && (m_model != NULL))
    {
        int index = m_mapper->currentIndex();
        QVariant saveData = m_model->data(m_model->index(index, GameDB::TBL_savesData_Data));
        //QVariant gameID = m_model->data(m_model->index(index, GameDB::TBL_savesData_GameID));
        enum GameDB::GameSaveType saveType = querySaveType();
        QByteArray binData = saveData.toByteArray();
        switch (saveType)
        {
        case GameDB::InternalBlob:
        {
            QByteArray hexData = binData.toHex();
            ui->plainTextEdit_SavesData_SaveData->setPlainText(QString::fromUtf8(hexData));
            ui->pushButton_SavesData_Lock->setEnabled(true);
            break;
        }
        case GameDB::InternalText:
        {
            ui->plainTextEdit_SavesData_SaveData->setPlainText(QString::fromUtf8(binData));
            ui->pushButton_SavesData_Lock->setEnabled(true);
            break;
        }
        case GameDB::SingleFile:
        {
            ui->pushButton_SavesData_Lock->setEnabled(false);
            break;
        }
        case GameDB::MultiFile:
        {
            ui->plainTextEdit_SavesData_SaveData->setPlainText(QString::fromUtf8(binData));
            ui->pushButton_SavesData_Lock->setEnabled(false);
            break;
        }
        default:
            qDebug() << "SavesData::updateUi: Unknown SaveType" << saveType;
        }
        ui->label_SavesData_CurrentGameNameValue->setText(queryGameName());

        if (index <= 0)
        {
            ui->pushButton_SavesData_First->setEnabled(false);
            ui->pushButton_SavesData_Previous->setEnabled(false);
        }
        else
        {
            ui->pushButton_SavesData_First->setEnabled(true);
            ui->pushButton_SavesData_Previous->setEnabled(true);
        }
        if (index >= m_model->rowCount()-1)
        {
            ui->pushButton_SavesData_Last->setEnabled(false);
            ui->pushButton_SavesData_Next->setEnabled(false);
        }
        else
        {
            ui->pushButton_SavesData_Last->setEnabled(true);
            ui->pushButton_SavesData_Next->setEnabled(true);
        }
    }
}

QString SavesData::queryGameName() const
{
    if (m_mapper != NULL)
    {
        QString str("select g.Name from "
                    "tbl_savesData as s inner join tbl_gameInfo as g "
                    "on s.GameID = g.ID");
        int index = m_mapper->currentIndex();
        if (index >= 0)
        {
            QVariant id = m_model->data(m_model->index(index, 0));
            if (!id.isValid())
            {
                qDebug() << "SavesData::querySaveType: query game save id failed.";
                Utils::printQSqlQueryLastError(m_model->query());
                return QString();
            }
            str.append(QString(" where s.ID=%1").arg(id.toInt()));
        }
        //str.append(QString(" GROUP BY g.SaveType"));
        QSqlQuery query(m_db);
        bool ret = query.exec(str);
        if (!ret)
        {
            qDebug() << "SavesData::querySaveType: query game save type failed.";
            Utils::printQSqlQueryLastError(query);
            return QString();
        }
        Utils::printQSqlQueryLastQuery(query);
        ret = query.first();
        if (ret)
        {
            QString value = query.value(0).toString();
            return value;
        }
    }
    else
    {
        QString str("select Name from "
                    "tbl_gameInfo where ID=%1");
        str=str.arg(m_gameId);
        QSqlQuery query(m_db);
        bool ret = query.exec(str);
        if (!ret)
        {
            qDebug() << "SavesData::querySaveType: query game save type failed.";
            Utils::printQSqlQueryLastError(query);
            return QString();
        }
        Utils::printQSqlQueryLastQuery(query);
        ret = query.first();
        if (ret)
        {
            QString value = query.value(0).toString();
            return value;
        }
    }
    return QString();
}

enum GameDB::GameSaveType SavesData::querySaveType() const
{
    if (m_mapper != NULL)
    {
        QString str("select g.SaveType from "
                    "tbl_savesData as s inner join tbl_gameInfo as g "
                    "on s.GameID = g.ID");
        int index = m_mapper->currentIndex();
        if (index >= 0)
        {
            QVariant id = m_model->data(m_model->index(index, 0));
            if (!id.isValid())
            {
                qDebug() << "SavesData::querySaveType: query game save id failed.";
                Utils::printQSqlQueryLastError(m_model->query());
                return GameDB::UnknownSave;
            }
            str.append(QString(" where s.ID=%1").arg(id.toInt()));
        }
        //str.append(QString(" GROUP BY g.SaveType"));
        QSqlQuery query(m_db);
        bool ret = query.exec(str);
        if (!ret)
        {
            qDebug() << "SavesData::querySaveType: query game save type failed.";
            Utils::printQSqlQueryLastError(query);
            return GameDB::UnknownSave;
        }
        Utils::printQSqlQueryLastQuery(query);
        ret = query.first();
        if (ret)
        {
            int saveType = query.value(0).toInt();
            return (enum GameDB::GameSaveType)saveType;
        }
    }
    else
    {
        QString str("select SaveType from "
                    "tbl_gameInfo where ID=%1");
        str=str.arg(m_gameId);
        QSqlQuery query(m_db);
        bool ret = query.exec(str);
        if (!ret)
        {
            qDebug() << "SavesData::querySaveType: query game save type failed.";
            Utils::printQSqlQueryLastError(query);
            return GameDB::UnknownSave;
        }
        Utils::printQSqlQueryLastQuery(query);
        ret = query.first();
        if (ret)
        {
            int saveType = query.value(0).toInt();
            return (enum GameDB::GameSaveType)saveType;
        }
    }
    return GameDB::UnknownSave;
}

void SavesData::writeSaveData()
{
    if ((m_mapper != NULL) && (m_model != NULL))
    {
        SaveDataBundle d = currentSaveData();
        int index = m_mapper->currentIndex();
        bool ret = m_model->setData(m_model->index(
                                        index, GameDB::TBL_savesData_Data),
                                    QVariant(d.data));
        if (!ret)
        {
            qDebug() << "SavesData::writeSaveData: setData data field failed.";
            Utils::printQSqlQueryLastError(m_model->query());
        }
        else
        {
            Utils::printQSqlQueryLastQuery(m_model->query());
        }
        ret = m_model->setData(m_model->index(
                                   index, GameDB::TBL_savesData_Hash),
                               QVariant(d.hashText));
        if (!ret)
        {
            qDebug() << "SavesData::writeSaveData: setData hash field failed.";
            Utils::printQSqlQueryLastError(m_model->query());
        }
        else
        {
            Utils::printQSqlQueryLastQuery(m_model->query());
        }
        ret = m_model->setData(m_model->index(
                                   index, GameDB::TBL_savesData_DateTime),
                               QVariant(d.dateTimeStr));
        if (!ret)
        {
            qDebug() << "SavesData::writeSaveData: setData datetime field failed.";
            Utils::printQSqlQueryLastError(m_model->query());
        }
        else
        {
            Utils::printQSqlQueryLastQuery(m_model->query());
        }
    }
}

void SavesData::markDataChanged()
{
    if (!this->windowTitle().endsWith("*"))
    {
        this->setWindowTitle(this->windowTitle()+"*");
    }
    m_state = Editing;
}

void SavesData::blockEditors()
{
    ui->lineEdit_SavesData_Branch->blockSignals(true);
    ui->lineEdit_SavesData_Tag->blockSignals(true);
    ui->lineEdit_SavesData_Date->blockSignals(true);
    ui->plainTextEdit_SavesData_Memo->blockSignals(true);
    ui->plainTextEdit_SavesData_SaveData->blockSignals(true);
}

void SavesData::unblockEditors()
{
    ui->lineEdit_SavesData_Branch->blockSignals(false);
    ui->lineEdit_SavesData_Tag->blockSignals(false);
    ui->lineEdit_SavesData_Date->blockSignals(false);
    ui->plainTextEdit_SavesData_Memo->blockSignals(false);
    ui->plainTextEdit_SavesData_SaveData->blockSignals(false);
}
