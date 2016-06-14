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

#ifndef LOGGING_H
#define LOGGING_H

#include <QMainWindow>

namespace Ui {
class Logging;
}

class Logging : public QMainWindow
{
    Q_OBJECT

public:
    explicit Logging(QWidget *parent = 0);
    ~Logging();

private slots:
    void on_automatic_clicked();
    void on_manual_clicked();
    void on_pushButton_clicked();
    void on_benchmark_clicked();
    void on_maintenance_clicked();

private:
    Ui::Logging *ui;
};

#endif // LOGGING_H
