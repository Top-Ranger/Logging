#include "benchmarkclient.h"

#include "persistance.h"
#include <QMessageBox>
#include <QTime>

BenchmarkClient::BenchmarkClient()
{
    Persistance p;
    QTime start = QTime::currentTime();

    for(int i = 0; i < 1000; ++i)
    {
        int tid = p.beginTransaction();
        p.write(tid, i, "Benchmark", 1000-i);
        p.write(tid, 10000000, QString("%1").arg(i), "Benchmark");
        p.write(tid, 10000000, QString("%1").arg(i+1000), "Benchmark");
        p.write(tid, 10000000, QString("%1").arg(i+2000), "Benchmark");
        p.commit(tid);

        tid = p.beginTransaction();
        p.remove(tid, 10000000, QString("%1").arg(i+2000));
        p.remove(tid, 1000 + qrand()%5, QString("%1").arg(qrand()%5));
        p.commit(tid);
    }
    qint64 needed = start.msecsTo(QTime::currentTime());

    QMessageBox::information(0,
                             tr("Benchmark results"),
                             QString(tr("Actions: %1\nTime: %2 s\nAverage: %3 ms / action")).arg(1000*10).arg(needed/1000).arg(needed / 1000*10));
}
