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
