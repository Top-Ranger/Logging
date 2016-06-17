#ifndef BENCHMARKCLIENT_H
#define BENCHMARKCLIENT_H

#include <QObject>

class BenchmarkClient : QObject
{
public:
    BenchmarkClient();

private:
    static int constexpr ROUNDS = 10000;
};

#endif // BENCHMARKCLIENT_H
