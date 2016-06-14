#include "benchmarkclient.h"

#include "persistance.h"
#include <QMessageBox>
#include <QTime>

BenchmarkClient::BenchmarkClient()
{
    Persistance p;

    p.restore_all();

    QTime start = QTime::currentTime();

    for(int i = 0; i < ROUNDS; ++i)
    {
        int tid = p.beginTransaction();
        p.write(tid, i, "Benchmark", ROUNDS-i);
        p.write(tid, 10000000, QString("%1").arg(i), "Benchmark");
        p.write(tid, 10000000, QString("%1").arg(i+ROUNDS), "Benchmark");
        p.write(tid, 10000000, QString("%1").arg(i+ROUNDS+ROUNDS), "Benchmark");
        p.commit(tid);

        tid = p.beginTransaction();
        p.remove(tid, 10000000, QString("%1").arg(i+ROUNDS+ROUNDS));
        p.remove(tid, 1000 + qrand()%5, QString("%1").arg(qrand()%5));
        p.commit(tid);
    }
    qint64 needed = start.msecsTo(QTime::currentTime());

    QMessageBox::information(0,
                             tr("Benchmark results"),
                             QString(tr("Actions: %1\nTime: %2 s\nAverage: %3 ms / action")).arg(ROUNDS*10).arg((double) needed/1000).arg((double) needed / (ROUNDS*10)));
}
