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

#include "automaticclient.h"
#include "ui_automaticclient.h"

#include "persistance.h"
#include <random>

#include <QDebug>

AutomaticClient::AutomaticClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AutomaticClient),
    _runner()
{
    ui->setupUi(this);

    setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint));

    _runner.start();
}

AutomaticClient::~AutomaticClient()
{
    _runner.finishWork();
    _runner.wait();
    delete ui;
}

void AutomaticClient::on_pushButton_clicked()
{
    _runner.finishWork();
    _runner.wait();
    this->deleteLater();
    close();
}

ACRunner::ACRunner(QObject *parent) :
    QThread(parent),
    _stop_work(false),
    _tid(-1)
{
}

ACRunner::~ACRunner()
{
    finishWork();
    wait();
}

void ACRunner::finishWork()
{
    _stop_work = true;
}

void ACRunner::run()
{
    Persistance p;
    qlonglong counter = 0;

    std::random_device rnd;
    std::uniform_int_distribution<qlonglong> page_dist(0,20);

    while(!_stop_work)
    {
        if(_tid == -1)
        {
            _tid = p.beginTransaction();
        }
        else
        {
            switch(rnd() % 5)
            {
            case 0:
                qDebug() << Q_FUNC_INFO << "Writing";
                p.write(_tid, page_dist(rnd), QString("%1").arg(page_dist(rnd)), QString("Write %1").arg(++counter));
                break;
            case 1:
                qDebug() << Q_FUNC_INFO << "Reading";
                p.read(_tid, page_dist(rnd), QString("%1").arg(page_dist(rnd)));
                break;
            case 2:
                qDebug() << Q_FUNC_INFO << "Deleting";
                p.remove(_tid, page_dist(rnd), QString("%1").arg(page_dist(rnd)));
                break;
            case 3:
                qDebug() << Q_FUNC_INFO << "Comitting";
                p.commit(_tid);
                _tid = -1;
                break;
            case 4:
                qDebug() << Q_FUNC_INFO << "Rollback";
                p.rollback(_tid);
                _tid = -1;
                break;
            }
        }

        sleep(1);
    }
    if(_tid != -1)
    {
        p.commit(_tid);
    }
}
