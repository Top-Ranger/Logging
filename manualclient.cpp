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
    ui->data->setEnabled(running);
    ui->read->setEnabled(running);
    ui->write->setEnabled(running);
    ui->commit->setEnabled(running);
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
    qint64 page_id = ui->page->value();
    ui->data->setText(_p.read(_tid, page_id));
}

void ManualClient::on_write_clicked()
{
    qint64 page_id = ui->page->value();
    _p.write(_tid, page_id, ui->data->text());
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
