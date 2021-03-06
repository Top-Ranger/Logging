﻿/*
 * Copyright (C) 2016 Marcus Soll
 * This file is part of Logger.
 *
 * Logger is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Logger is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Logger. If not, see <http://www.gnu.org/licenses/>.
 */

#include "persistance.h"

#include <QDir>
#include <QtCore/qmath.h>
#include <QDebug>
#include <QReadLocker>
#include <QWriteLocker>

#include <limits>

QReadWriteLock *Persistance::_rwlock = new QReadWriteLock();
QHash<qlonglong, Persistance::page> Persistance::_buffer = QHash<qlonglong, Persistance::page>();
QHash<qlonglong, QList<qlonglong> > Persistance::_transactions = QHash<qlonglong, QList<qlonglong> >();
qulonglong Persistance::_lognr = std::numeric_limits<qlonglong>::max();
qlonglong Persistance::_last_tid = 0;
QLinkedList<qlonglong> Persistance::_lru_cache = QLinkedList<qlonglong>();
QLockFile Persistance::_lockfile("./.Persistance.lock");

Persistance::Persistance()
{
    QWriteLocker l(_rwlock);

    if(_lognr == std::numeric_limits<qlonglong>::max())
    {
        _lockfile.setStaleLockTime(0);
        if(!_lockfile.tryLock())
        {
             qFatal("%s error while locking ./.lock (%i) - maybe an other instance is already running? - terminating", Q_FUNC_INFO, _lockfile.error());
        }

        QDir dir("./log/");
        if(dir.exists() && QFile::exists("./log/last_log.dat"))
        {
            QFile f("./log/last_log.dat");
            f.open(QIODevice::ReadOnly);
            QDataStream s(&f);
            s.setVersion(DATASTREAM_VERSION);
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
            s.setVersion(DATASTREAM_VERSION);
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
            s.setVersion(DATASTREAM_VERSION);
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

qlonglong Persistance::beginTransaction()
{
    QWriteLocker l(_rwlock);
    forever
    {
        ++_last_tid;
        if(!_transactions.contains(_last_tid))
        {
            _transactions[_last_tid] = QList<qlonglong>();
            return _last_tid;
        }
    }
}

void Persistance::write(qlonglong transaction_id, qlonglong page_id, QString key, QVariant data)
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

    update_lru(page_id);

    _buffer[page_id].write_buffer[transaction_id][key] = data;
    if(_buffer[page_id].delete_buffer[transaction_id].contains(key))
    {
        _buffer[page_id].delete_buffer[transaction_id].remove(key);
    }

    if(!_transactions[transaction_id].contains(page_id))
    {
        _transactions[transaction_id].append(page_id);
    }
}

QVariant Persistance::read(qlonglong transaction_id, qlonglong page_id, QString key)
{
    QReadLocker l(_rwlock);
    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return "";
    }

    if(!_buffer.contains(page_id))
    {
        l.unlock();
        _rwlock->lockForWrite();
        load_dataset(page_id);
        _rwlock->unlock();
        l.relock();
    }

    l.unlock();
    _rwlock->lockForWrite();
    update_lru(page_id);
    _rwlock->unlock();
    l.relock();

    if(_buffer[page_id].delete_buffer.value(transaction_id).contains(key))
    {
        return QVariant();
    }

    if(_buffer[page_id].write_buffer.contains(transaction_id))
    {
        return _buffer[page_id].write_buffer[transaction_id].value(key);
    }

    return _buffer[page_id].data.value(key);
}

void Persistance::remove(qlonglong transaction_id, qlonglong page_id, QString key)
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

    update_lru(page_id);

    if(!(_buffer[page_id].data.contains(key) || _buffer[page_id].write_buffer.value(transaction_id).contains(key)))
    {
        return;
    }

    _buffer[page_id].delete_buffer[transaction_id].insert(key);

    if(_buffer[page_id].write_buffer[transaction_id].contains(key))
    {
        _buffer[page_id].write_buffer[transaction_id].remove(key);
    }

    if(!_transactions[transaction_id].contains(page_id))
    {
        _transactions[transaction_id].append(page_id);
    }
}

void Persistance::commit(qlonglong transaction_id)
{
    QWriteLocker l(_rwlock);

    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return;
    }

    if(_transactions[transaction_id].empty())
    {
        // Do not write a log file if no data has been written
        _transactions.remove(transaction_id);
        flush_buffer();
        return;
    }

    QFile log_file(QString("./log/%1").arg(_lognr));
    log_file.open(QIODevice::WriteOnly);
    QDataStream log_stream(&log_file);
    log_stream.setVersion(DATASTREAM_VERSION);
    log_stream << _transactions[transaction_id];

    foreach (qlonglong page_id, _transactions[transaction_id])
    {
        log_stream << _buffer[page_id].write_buffer[transaction_id];
        log_stream << _buffer[page_id].delete_buffer[transaction_id];

        foreach (QString key, _buffer[page_id].write_buffer[transaction_id].keys())
        {
            _buffer[page_id].data[key] = _buffer[page_id].write_buffer[transaction_id][key];
        }

        foreach (QString key, _buffer[page_id].delete_buffer[transaction_id])
        {
            _buffer[page_id].data.remove(key);
        }
        _buffer[page_id].write_buffer.remove(transaction_id);
        _buffer[page_id].delete_buffer.remove(transaction_id);
    }

    log_file.close();

    flush_buffer();

    increase_log_number();
    _transactions.remove(transaction_id);
}

void Persistance::rollback(qlonglong transaction_id)
{
    QWriteLocker l(_rwlock);

    if(!_transactions.contains(transaction_id))
    {
        qWarning() << Q_FUNC_INFO << "Unknown id" << transaction_id;
        return;
    }

    foreach (qlonglong page_id, _transactions[transaction_id])
    {
        _buffer[page_id].write_buffer.remove(transaction_id);
        _buffer[page_id].delete_buffer.remove(transaction_id);
    }

    _transactions.remove(transaction_id);

    flush_buffer();
}

void Persistance::flush()
{
    QReadLocker l(_rwlock);

    qDebug() << Q_FUNC_INFO << "Flushing all pages in buffer";

    foreach (qlonglong page_id, _buffer.keys())
    {
        write_dataset(page_id);
    }
}

void Persistance::restore_all()
{
    QWriteLocker l(_rwlock);

    qDebug() << Q_FUNC_INFO << "Setting all known pages to last version";

    QDir page_dir("./pages/");

    foreach (QString page, page_dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot))
    {
        if(page.endsWith(".clean"))
        {
            continue;
        }

        bool ok;
        qlonglong page_id = page.toLongLong(&ok);

        if(ok)
        {
            // No need to restore clean pages
            if(!_buffer.contains(page_id) && !QFile::exists(QString("./pages/%1.clean").arg(page_id)))
            {
                load_dataset(page_id);
                write_dataset(page_id);
                _buffer.remove(page_id);
                _lru_cache.removeLast();
                QFile save_file(QString("./pages/%1.clean").arg(page_id));
                save_file.open(QIODevice::WriteOnly);
                save_file.close();
            }
        }
    }

    // Flush keeps its own lock - so release the write log
    l.unlock();
    flush();
}

void Persistance::vacuum_logs()
{
    QWriteLocker l(_rwlock);

    qDebug() << Q_FUNC_INFO << "Starting log vacuum";

    QDir page_dir("./pages/");
    qulonglong last_save_log = _lognr;

    foreach (QString page, page_dir.entryList(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot))
    {
        if(page.endsWith(".clean"))
        {
            continue;
        }

        bool ok;
        qlonglong page_id = page.toLongLong(&ok);

        if(ok)
        {
            if(_buffer.contains(page_id))
            {
                last_save_log = qMin(last_save_log, _buffer[page_id].lognr);
            }
            else
            {
                load_dataset(page_id);
                write_dataset(page_id);
                last_save_log = qMin(last_save_log, _buffer[page_id].lognr);
                _buffer.remove(page_id);
                _lru_cache.removeLast();
                QFile save_file(QString("./pages/%1.clean").arg(page_id));
                save_file.open(QIODevice::WriteOnly);
                save_file.close();
            }
        }
    }

    qDebug() << Q_FUNC_INFO << "Removing all logs before" << last_save_log;

    for(qulonglong i = 0; i < last_save_log; ++i)
    {
        if(QFile::exists(QString("./log/%1").arg(i)))
        {
            QFile::remove(QString("./log/%1").arg(i));
        }
    }
}

void Persistance::load_dataset(qlonglong page_id)
{
    if(_buffer.contains(page_id))
    {
        // Dataset is already loaded - skipping
        return;
    }

    qulonglong last_log = 0;
    page new_page;

    if(QFile::exists(QString("./pages/%1").arg(page_id)))
    {
        QFile f(QString("./pages/%1").arg(page_id));
        f.open(QIODevice::ReadOnly);
        QDataStream page_stream(&f);
        page_stream.setVersion(DATASTREAM_VERSION);
        page_stream >> new_page;
        f.close();
        last_log = new_page.lognr;

        if(!QFile::exists(QString("./pages/%1.clean").arg(page_id)))
        {
            for(; last_log < _lognr; ++last_log)
            {
                if(QFile::exists(QString("./log/%1").arg(last_log)))
                {
                    QFile f(QString("./log/%1").arg(last_log));
                    f.open(QIODevice::ReadOnly);
                    QDataStream s(&f);
                    s.setVersion(DATASTREAM_VERSION);
                    QList<qlonglong> saved_pages;
                    s >> saved_pages;
                    if(saved_pages.contains(page_id))
                    {
                        while(!s.atEnd())
                        {
                            QHash<QString, QVariant> writes;
                            QSet<QString> deletes;
                            s >> writes >> deletes;
                            if(saved_pages.first() == page_id)
                            {
                                foreach (QString key, writes.keys())
                                {
                                    new_page.data[key] = writes[key];
                                }

                                foreach (QString key, deletes)
                                {
                                    new_page.data.remove(key);
                                }
                            }
                            saved_pages.removeFirst();
                        }
                    }
                    f.close();
                }
                else
                {
                    qWarning() << Q_FUNC_INFO << "Missing log" << last_log;
                }
            }
        }
        else
        {
            QFile::remove(QString("./pages/%1.clean").arg(page_id));
        }
    }
    else if(QFile::exists(QString("./pages/%1.prepare").arg(page_id)))
    {
        qCritical() << "page file" << page_id << "does not exist but prepare file does! Will attempt to recover!";
        // Just rename and load
        QFile::rename(QString("./pages/%1.prepare").arg(page_id), QString("./pages/%1").arg(page_id));
        QFile::remove(QString("./pages/%1.prepare").arg(page_id));
        load_dataset(page_id);
        return;
    }
    else
    {
        // Directly save newly created page - this way we do not need to traverse all logs to search for lost pages
        new_page.lognr = _lognr;
        _buffer[page_id] = new_page;
        _lru_cache.append(page_id);
        write_dataset(page_id);
        return;
    }

    _buffer[page_id] = new_page;
    _lru_cache.append(page_id);
}

void Persistance::write_dataset(qlonglong page_id)
{
    QFile page_file(QString("./pages/%1.prepare").arg(page_id));
    page_file.open(QIODevice::WriteOnly);
    QDataStream page_stream(&page_file);
    page_stream.setVersion(DATASTREAM_VERSION);
    _buffer[page_id].lognr = _lognr;
    page_stream << _buffer[page_id];
    page_file.close();
    QFile::remove(QString("./pages/%1").arg(page_id));
    QFile::rename(QString("./pages/%1.prepare").arg(page_id), QString("./pages/%1").arg(page_id));
    QFile::remove(QString("./pages/%1.prepare").arg(page_id));
}

void Persistance::increase_log_number()
{
    _lognr += 1;
    QFile f("./log/last_log.dat.prepare");
    f.open(QIODevice::WriteOnly);
    QDataStream s(&f);
    s.setVersion(DATASTREAM_VERSION);
    s << _lognr;
    f.close();
    QFile::remove("./log/last_log.dat");
    QFile::rename("./log/last_log.dat.prepare", "./log/last_log.dat");
    QFile::remove("./log/last_log.dat.prepare");
}

void Persistance::flush_buffer()
{
    if(_buffer.size() > MAX_DATASETS)
    {
        qDebug() << Q_FUNC_INFO << "Pages before flush:" << _buffer.size();

        QLinkedList<qlonglong> locked_pages;

        while(_buffer.size() > MAX_DATASETS/2 && !_lru_cache.empty())
        {
            qlonglong page_id = _lru_cache.takeFirst();

            if(_buffer[page_id].write_buffer.size() == 0 && _buffer[page_id].delete_buffer.size() == 0)
            {
                write_dataset(page_id);
                _buffer.remove(page_id);

                // Create save file
                QFile save_file(QString("./pages/%1.clean").arg(page_id));
                save_file.open(QIODevice::WriteOnly);
                save_file.close();
            }
            else
            {
                // We can not flush this page
                locked_pages.append(page_id);
            }
        }
        locked_pages << _lru_cache;
        _lru_cache = locked_pages;
        qDebug() << Q_FUNC_INFO << "Pages after flush:" << _buffer.size();
    }
}

void Persistance::update_lru(qlonglong page_id)
{
    for(QLinkedList<qlonglong>::iterator i = _lru_cache.begin(); i != _lru_cache.end(); ++i)
    {
        if(*i == page_id)
        {
            _lru_cache.erase(i);
            _lru_cache.append(page_id);
            return;
        }
    }
}
