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

#include "logging.h"
#include "ui_logging.h"

#include "automaticclient.h"
#include "manualclient.h"
#include "benchmarkclient.h"

Logging::Logging(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Logging)
{
    ui->setupUi(this);
}

Logging::~Logging()
{
    delete ui;
}

void Logging::on_automatic_clicked()
{
    AutomaticClient *c = new AutomaticClient();
    c->show();
}

void Logging::on_manual_clicked()
{
    ManualClient *c = new ManualClient();
    c->show();
}

void Logging::on_pushButton_clicked()
{
    QApplication::aboutQt();
}

void Logging::on_benchmark_clicked()
{
    BenchmarkClient c;
}
