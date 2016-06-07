#ifndef AUTOMATICCLIENT_H
#define AUTOMATICCLIENT_H

#include <QWidget>
#include <QThread>

namespace Ui {
class AutomaticClient;
}

class ACRunner : public QThread
{
    Q_OBJECT

public:
    explicit ACRunner(QObject *parent = 0);
    ~ACRunner();
    void finishWork();

protected:
    void run();

private:
    bool _stop_work;
    qint64 _tid;
};

class AutomaticClient : public QWidget
{
    Q_OBJECT

public:
    explicit AutomaticClient(QWidget *parent = 0);
    ~AutomaticClient();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AutomaticClient *ui;

    ACRunner _runner;
};
#endif // AUTOMATICCLIENT_H
