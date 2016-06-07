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

#ifndef AUTOMATICCLIENT_H
#define AUTOMATICCLIENT_H

#include <QWidget>
#include <QThread>

namespace Ui {
class AutomaticClient;
}

class ACRunner : public QThread
{
    Q_OBJECT

public:
    explicit ACRunner(QObject *parent = 0);
    ~ACRunner();
    void finishWork();

protected:
    void run();

private:
    bool _stop_work;
    qint64 _tid;
};

class AutomaticClient : public QWidget
{
    Q_OBJECT

public:
    explicit AutomaticClient(QWidget *parent = 0);
    ~AutomaticClient();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AutomaticClient *ui;

    ACRunner _runner;
};
#endif // AUTOMATICCLIENT_H
