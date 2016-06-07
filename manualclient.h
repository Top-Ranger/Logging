/*
 * Copyright (C) 2016 Marcus Soll
 * This file is part of Logger.
 *
 * Logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Logger. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MANUALCLIENT_H
#define MANUALCLIENT_H

#include <QWidget>
#include "persistance.h"

namespace Ui {
class ManualClient;
}

class ManualClient : public QWidget
{
    Q_OBJECT

public:
    explicit ManualClient(QWidget *parent = 0);
    ~ManualClient();

private slots:
    void on_transaction_clicked();
    void on_read_clicked();
    void on_write_clicked();
    void on_commit_clicked();
    void on_rollback_clicked();
    void on_close_clicked();

private:
    void transaction_running(bool running);

    Ui::ManualClient *ui;
    qint64 _tid;
    Persistance _p;
};

#endif // MANUALCLIENT_H
