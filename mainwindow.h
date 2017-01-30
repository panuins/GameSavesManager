/*****************************************************************************
 * mainwindow.h: main window header file.
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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_listView_games_activated(const QModelIndex &index);
    void on_listView_games_clicked(const QModelIndex &index);
    void on_listView_games_doubleClicked(const QModelIndex &index);
    void on_pushButton_Games_Add_clicked();
    void on_pushButton_Games_Edit_clicked();
    void on_pushButton_Games_Remove_clicked();
    void on_pushButton_Saves_Add_clicked();
    void on_pushButton_Saves_Edit_clicked();
    void on_pushButton_Saves_Commit_clicked();
    void on_pushButton_Saves_Remove_clicked();

    void on_tableView_Saves_doubleClicked(const QModelIndex &index);

private:
    int currentGameId() const;

    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
