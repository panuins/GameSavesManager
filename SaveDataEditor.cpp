/*****************************************************************************
 * SaveDataEditor.cpp: save data editor dialog source file.
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

#include "SaveDataEditor.h"
#include "ui_SaveDataEditor.h"

SaveDataEditor::SaveDataEditor(QSqlDatabase db, QSqlTableModel *model, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaveDataEditor)
{
    Q_UNUSED(db)
    Q_UNUSED(model)
    ui->setupUi(this);
}

SaveDataEditor::~SaveDataEditor()
{
    delete ui;
}

QString SaveDataEditor::edit(const QString &text)
{
    SaveDataEditor editor(QSqlDatabase(), NULL);
    editor.ui->plainTextEdit_SaveDataEditor->setPlainText(text);
//    editor.m_buttonImport = editor.ui->buttonBox_SaveDataEditor->addButton(
//                tr("Import"), QDialogButtonBox::ActionRole);
//    editor.m_buttonExport = editor.ui->buttonBox_SaveDataEditor->addButton(
//                tr("Export"), QDialogButtonBox::ActionRole);
    int ret = editor.exec();
    if (ret == QDialog::Rejected)
    {
        return QString();
    }
    else if (ret == QDialog::Accepted)
    {
        return editor.ui->plainTextEdit_SaveDataEditor->toPlainText();
    }
    return QString();
}

void SaveDataEditor::on_buttonBox_SaveDataEditor_clicked(QAbstractButton *button)
{
    if (m_buttonImport.data() == button)
    {
        //
    }
    if (m_buttonExport.data() == button)
    {
        //
    }
}
