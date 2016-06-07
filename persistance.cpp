#include "persistance.h"

#include <QDir>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QReadLocker>
#include <QWriteLocker>

QReadWriteLock *Persistance::_rwlock = new QReadWriteLock();
QHash<qint64, Persistance::page> Persistance::_buffer = QHash<qint64, Persistance::page>();
QHash<qint64, QList<qint64> > Persistance::_transactions = QHash<qint64, QList<qint64> >();
qint64 Persistance::_lognr = -1;

Persistance::Persistance()
{
    QWriteLocker l(_rwlock);

    if(_lognr == -1)
    {
        QDir dir("./log/");
        if(dir.exists() && QFile::exists("./log/last_log.dat"))
        {
            QFile f("./log/last_log.dat");
            f.open(QIODevice::ReadOnly);
            QDataStream s(&f);
            s >> _lognr;
            qDebug() << Q_FUNC_INFO << "Last log id" << _lognr;
            f.close();
        }
        else if(dir.exists() && QFile::exists("./log/last_log.dat.prepare"))
        {
            qCritical() << "./log/last_log.dat does not exist but ./log/last_log.dat.prepare does! Will attempt to recover!";
            QFile f("./log/last_log.dat.prepare");
            f.open(QIODevice::ReadOnly);
            QDataStream s(&f);
            s >> _lognr;
            qDebug() << Q_FUNC_INFO << "Last log id" << _lognr;
            f.close();
        }
        else
        {
            dir.mkpath("./");

            qDebug() << Q_FUNC_INFO << "New log directory created";

            QFile f("./log/last_log.dat");
            f.open(QIODevice::WriteOnly);
            QDataStream s(&f);
            s << 0;
            _lognr = 0;
        }
    }

    QDir dir("./pages/");
    if(!dir.exists())
    {
        dir.mkpath("./");
    }
}

qint64 Persistance::beginTransaction()
{
    QWriteLocker l(_rwlock);
    qint64 id = 1;
    forever
    {
        if(!_transactions.contains(id))
        {
            _transactions[id] = QList<qint64>();
            return id;
        }
        ++id;
    }
}

void Persistance::write(qint64 transaction_id, qint64 page_id, QString data)
{
    QWriteLocker l(_rwlock);
    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return;
    }

    if(!_buffer.contains(page_id))
    {
        load_dataset(page_id);
    }

    _buffer[page_id].write_buffer[transaction_id] = data;
    if(!_transactions[transaction_id].contains(page_id))
    {
        _buffer[page_id].transactions += 1;
        _transactions[transaction_id].append(page_id);
    }
}

QString Persistance::read(qint64 transaction_id, qint64 page_id)
{
    QReadLocker l(_rwlock);
    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return "";
    }

    if(!_buffer.contains(page_id))
    {
        load_dataset(page_id);
    }

    if(_buffer[page_id].write_buffer.contains(transaction_id))
    {
        return _buffer[page_id].write_buffer[transaction_id];
    }

     return _buffer[page_id].data;
}

void Persistance::commit(qint64 transaction_id)
{
    QWriteLocker l(_rwlock);

    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return;
    }

    bool persist_pages = _buffer.size() > MAX_DATASETS;

    QFile log_file(QString("./log/%1").arg(_lognr));
    log_file.open(QIODevice::WriteOnly);
    QDataStream log_stream(&log_file);
    log_stream << _transactions[transaction_id];

    foreach (qint64 page_id, _transactions[transaction_id])
    {
        _buffer[page_id].data = _buffer[page_id].write_buffer[transaction_id];
         _buffer[page_id].write_buffer.remove(transaction_id);
        log_stream << _buffer[page_id];
        --_buffer[page_id].transactions;
        if(persist_pages && _buffer[page_id].transactions == 0)
        {
            QFile page_file(QString("./pages/%1").arg(page_id));
            page_file.open(QIODevice::WriteOnly);
            QDataStream page_stream(&page_file);
            _buffer[page_id].lognr = _lognr;
            page_stream << _buffer[page_id];
            _buffer.remove(page_id);
            page_file.close();
        }
    }
    log_file.close();
    increase_log_number();
    _transactions.remove(transaction_id);
}

void Persistance::rollback(qint64 transaction_id)
{
    QWriteLocker l(_rwlock);

    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return;
    }

    foreach (qint64 page_id, _transactions[transaction_id])
    {
        _buffer[page_id].write_buffer.remove(transaction_id);
    }

    _transactions.remove(transaction_id);
}

void Persistance::load_dataset(qint64 page_id)
{
    qint64 last_log = 0;
    page new_page;

    if(QFile::exists(QString("./pages/%1").arg(page_id)))
    {
        QFile f(QString("./pages/%1").arg(page_id));
        f.open(QIODevice::ReadOnly);
        QDataStream page_stream(&f);
        page_stream >> new_page;
        f.close();
        last_log = new_page.lognr;
    }

    for(; last_log < _lognr; ++last_log)
    {
        if(QFile::exists(QString("./log/%1").arg(last_log)))
        {
            QFile f(QString("./log/%1").arg(last_log));
            f.open(QIODevice::ReadOnly);
            QDataStream s(&f);
            QList<qint64> saved_pages;
            s >> saved_pages;
            if(saved_pages.contains(page_id))
            {
                while(!s.atEnd())
                {
                    page current_log;
                    s >> current_log;
                    if(saved_pages.first() == page_id)
                    {
                        new_page = current_log;
                        break;
                    }
                    saved_pages.removeFirst();
                }
            }
            f.close();
        }
    }

    _buffer[page_id] = new_page;
}

void Persistance::increase_log_number()
{
    _lognr += 1;
    QFile f("./log/last_log.dat.prepare");
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s << _lognr;
    f.close();
    QFile::remove("./log/last_log.dat");
    QFile::rename("./log/last_log.dat.prepare", "./log/last_log.dat");
    QFile::remove("./log/last_log.dat.prepare");
}
