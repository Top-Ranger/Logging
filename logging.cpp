#include "logging.h"
#include "ui_logging.h"

#include "automaticclient.h"
#include "manualclient.h"

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
