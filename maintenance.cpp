#include "maintenance.h"
#include "ui_maintenance.h"

Maintenance::Maintenance(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Maintenance),
    _p()
{
    ui->setupUi(this);

    setWindowFlags(((windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowCloseButtonHint));
}

Maintenance::~Maintenance()
{
    delete ui;
}

void Maintenance::on_flush_clicked()
{
    _p.flush();
}

void Maintenance::on_restore_clicked()
{
    _p.restore_all();
}

void Maintenance::on_vacuum_clicked()
{
    _p.vacuum_logs();
}

void Maintenance::on_close_clicked()
{
    this->deleteLater();
}
