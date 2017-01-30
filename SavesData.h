/*****************************************************************************
 * SaveData.h: saves data dialog header file.
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

#ifndef SAVESDATA_H
#define SAVESDATA_H

#include "GameDB.h"
#include <QDialog>
#include <QSqlDatabase>
#include <QModelIndex>
#include <QSqlTableModel>
#include <QDataWidgetMapper>

class QPlainTextEdit;

namespace Ui {
class SavesData;
}

class SavesData : public QDialog
{
    Q_OBJECT

public:
    explicit SavesData(QSqlDatabase db, QSqlTableModel *model, QWidget *parent = 0);
    ~SavesData();

    static int showAdd(QSqlDatabase db, QSqlTableModel *model, int gameId);
    static void showEdit(QSqlDatabase db, QSqlTableModel *model, const QModelIndex &id);

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual bool eventFilter(QObject * watched, QEvent * event);

private slots:
    void on_pushButton_SavesData_Commit_clicked();
    void on_pushButton_SavesData_Ok_clicked();
    void on_pushButton_SavesData_Cancel_clicked();

    void on_pushButton_SavesData_First_clicked();
    void on_pushButton_SavesData_Previous_clicked();
    void on_pushButton_SavesData_Next_clicked();
    void on_pushButton_SavesData_Last_clicked();

    void on_pushButton_SavesData_Lock_toggled(bool checked);
    void on_pushButton_SavesData_Import_clicked();
    void on_pushButton_SavesData_Export_clicked();
    void on_pushButton_SavesData_Restore_clicked();
    void on_pushButton_SavesData_Overwrite_clicked();

    void on_lineEdit_SavesData_Tag_textChanged(const QString &arg1);
    void on_lineEdit_SavesData_Branch_textChanged(const QString &arg1);
    void on_lineEdit_SavesData_Date_textChanged(const QString &arg1);
    void on_plainTextEdit_SavesData_Memo_textChanged();

    void on_timer_timeout();

private:
    enum DialogState
    {
        Inited,
        Editing,
        Committed,
        WillBeRollbacked,
    };

    class SaveDataBundle
    {
    public:
        QString hashText;
        QString dateTimeStr;
        QByteArray data;
    };

    SaveDataBundle currentSaveData() const;
    void updateUi();
    QString queryGameName() const;
    enum GameDB::GameSaveType querySaveType() const;
    void writeSaveData();

    void markDataChanged();
    void blockEditors();
    void unblockEditors();

    Ui::SavesData *ui;
    QPointer<QSqlTableModel> m_model;
    QDataWidgetMapper *m_mapper;
    QPlainTextEdit *m_editor;
    QSqlDatabase m_db;
    enum GameDB::GameSaveType m_saveType;
    enum DialogState m_state;
    int m_row;
    int m_gameId;
    bool m_changed;
};

#endif // SAVESDATA_H
