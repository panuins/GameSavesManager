/*****************************************************************************
 * SaveDataEditor.h: save data editor dialog header file.
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

#ifndef SAVEDATAEDITOR_H
#define SAVEDATAEDITOR_H

#include <QDialog>
#include <QSqlDatabase>
#include <QModelIndex>
#include <QSqlTableModel>
#include <QDataWidgetMapper>
#include <QPointer>
#include <QPushButton>

namespace Ui {
class SaveDataEditor;
}

class SaveDataEditor : public QDialog
{
    Q_OBJECT

public:
    explicit SaveDataEditor(QSqlDatabase db, QSqlTableModel *model, QWidget *parent = 0);
    ~SaveDataEditor();

    static QString edit(const QString &text);

private slots:
    void on_buttonBox_SaveDataEditor_clicked(QAbstractButton *button);

private:
    Ui::SaveDataEditor *ui;
    QPointer<QPushButton> m_buttonImport;
    QPointer<QPushButton> m_buttonExport;
    QString m_text;
};

#endif // SAVEDATAEDITOR_H
