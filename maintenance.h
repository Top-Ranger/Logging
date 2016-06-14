#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <QWidget>

#include "persistance.h"

namespace Ui {
class Maintenance;
}

class Maintenance : public QWidget
{
    Q_OBJECT

public:
    explicit Maintenance(QWidget *parent = 0);
    ~Maintenance();

private slots:
    void on_flush_clicked();
    void on_restore_clicked();
    void on_vacuum_clicked();

    void on_close_clicked();

private:
    Ui::Maintenance *ui;

    Persistance _p;
};

#endif // MAINTENANCE_H
