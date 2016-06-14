#ifndef BENCHMARKCLIENT_H
#define BENCHMARKCLIENT_H

#include <QObject>

class BenchmarkClient : QObject
{
public:
    BenchmarkClient();

private:
    static qint32 constexpr ROUNDS = 1000;
};

#endif // BENCHMARKCLIENT_H
