/*****************************************************************************
 * GameInfo.h: GameDB info dialog header file.
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

#ifndef GAMEINFO_H
#define GAMEINFO_H

#include <QDialog>
#include <QSqlDatabase>
#include <QModelIndex>
#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <QPointer>

namespace Ui {
class GameInfo;
}

class QSqlTableModel;
class QCloseEvent;

class GameInfo : public QDialog
{
    Q_OBJECT

public:
    explicit GameInfo(QSqlDatabase db, QSqlTableModel *model, QWidget *parent = 0);
    ~GameInfo();

    static int showAdd(QSqlDatabase db, QSqlTableModel *model);
    static void showEdit(QSqlDatabase db, QSqlTableModel *model, const QModelIndex &id);

protected:
    virtual void closeEvent(QCloseEvent *e);

private slots:
    void on_pushButton_GameInfo_Commit_clicked();
    void on_pushButton_GameInfo_Cancel_clicked();
    void on_pushButton_GameInfo_First_clicked();
    void on_pushButton_GameInfo_Previous_clicked();
    void on_pushButton_GameInfo_Next_clicked();
    void on_pushButton_GameInfo_Last_clicked();

    void on_toolButton_GameInfo_OriginSavePath_clicked();
    void on_lineEdit_GameInfo_Name_textChanged(const QString &arg1);
    void on_comboBox_GameInfo_SaveType_currentIndexChanged(int index);
    void on_pushButton_GameInfo_Lock_toggled(bool checked);
    void on_lineEdit_GameInfo_PrimaryUrl_textChanged(const QString &arg1);
    void on_plainTextEdit_GameInfo_BackupUrls_textChanged();
    void on_plainTextEdit_GameInfo_OriginSavePath_textChanged();
    void on_plainTextEdit_GameInfo_Memo_textChanged();

    void on_pushButton_GameInfo_Ok_clicked();

private:
    enum DialogState
    {
        Inited,
        Editing,
        Committed,
        WillBeRollbacked,
    };

    void updateUi();

    void markDataChanged();
    void blockEditors();
    void unblockEditors();

    Ui::GameInfo *ui;
    QPointer<QSqlTableModel> m_model;
    QDataWidgetMapper *m_mapper;
    QSqlDatabase m_db;
    enum DialogState m_state;
    int m_row;
};

#endif // GAMEINFO_H
