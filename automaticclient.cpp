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
    qint64 counter = 0;

    std::random_device rnd;
    std::uniform_int_distribution<qint64> page_dist(0,20);

    while(!_stop_work)
    {
        if(_tid == -1)
        {
            _tid = p.beginTransaction();
        }
        else
        {
            switch(rnd() % 4)
            {
            case 0:
                qDebug() << Q_FUNC_INFO << "Writing";
                p.write(_tid, page_dist(rnd), QString("Write %1").arg(++counter));
                break;
            case 1:
                qDebug() << Q_FUNC_INFO << "Reading";
                p.read(_tid, page_dist(rnd));
                break;
            case 2:
                qDebug() << Q_FUNC_INFO << "Comitting";
                p.commit(_tid);
                _tid = -1;
                break;
            case 3:
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
