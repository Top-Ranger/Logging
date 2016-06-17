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

#include "manualclient.h"
#include "ui_manualclient.h"

ManualClient::ManualClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ManualClient),
    _tid(-1),
    _p()
{
    ui->setupUi(this);
    setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint));
}

ManualClient::~ManualClient()
{
    delete ui;
}

void ManualClient::transaction_running(bool running)
{
    ui->transaction->setEnabled(!running);
    ui->close->setEnabled(!running);
    ui->page->setEnabled(running);
    ui->key->setEnabled(running);
    ui->data->setEnabled(running);
    ui->read->setEnabled(running);
    ui->write->setEnabled(running);
    ui->commit->setEnabled(running);
    ui->remove->setEnabled(running);
    ui->rollback->setEnabled(running);
}

void ManualClient::on_transaction_clicked()
{
    _tid = _p.beginTransaction();
    transaction_running(true);
    ui->tid->setText(QString("%1").arg(_tid));
}

void ManualClient::on_read_clicked()
{
    qlonglong page_id = ui->page->value();
    ui->data->setText(_p.read(_tid, page_id, ui->key->text()).toString());
}

void ManualClient::on_write_clicked()
{
    qlonglong page_id = ui->page->value();
    _p.write(_tid, page_id, ui->key->text(), ui->data->text());
}

void ManualClient::on_remove_clicked()
{
    qlonglong page_id = ui->page->value();
    _p.remove(_tid, page_id, ui->key->text());
}


void ManualClient::on_commit_clicked()
{
    _p.commit(_tid);
    transaction_running(false);
    _tid = -1;
    ui->tid->setText(tr("No transaction"));
}

void ManualClient::on_rollback_clicked()
{
    _p.rollback(_tid);
    transaction_running(false);
    _tid = -1;
    ui->tid->setText(tr("No transaction"));
}

void ManualClient::on_close_clicked()
{
    close();
    this->deleteLater();
}
