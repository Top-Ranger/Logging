/*
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

#ifndef PERSISTANCE_H
#define PERSISTANCE_H

#include <QReadWriteLock>
#include <QHash>
#include <QList>
#include <QDataStream>
#include <QVariant>
#include <QSet>
#include <QLinkedList>

class Persistance
{
public:
    struct page
    {
        QHash<QString, QVariant> data;
        qint64 lognr;
        QHash<qint64, QHash<QString, QVariant> > write_buffer;
        QHash<qint64, QSet<QString> > delete_buffer;

        page() :
            data(),
            lognr(0),
            write_buffer(),
            delete_buffer()
        {
        }
    };

    Persistance();
    qint64 beginTransaction();
    void write(qint64 transaction_id, qint64 page_id, QString key, QVariant data);
    QVariant read(qint64 transaction_id, qint64 page_id, QString key);
    void remove(qint64 transaction_id, qint64 page_id, QString key);
    void commit(qint64 transaction_id);
    void rollback(qint64 transaction_id);
    void flush();
    void restore_all();

private:
    void load_dataset(qint64 page_id);
    void increase_log_number();
    void flush_buffer();
    void update_lru(qint64 page_id);

    static QReadWriteLock *_rwlock;
    static QHash<qint64, page> _buffer;
    static QHash<qint64, QList<qint64> > _transactions;
    static qint64 _lognr;
    static qint64 _last_tid;
    static QLinkedList<qint64> _lru_cache;

    static qint64 constexpr MAX_DATASETS = 10;
    static QDataStream::Version constexpr DATASTREAM_VERSION = QDataStream::Qt_5_0;
};

inline QDataStream& operator <<(QDataStream &stream, Persistance::page &p)
{
    stream << p.data << p.lognr;
    return stream;
}

static inline QDataStream& operator >>(QDataStream &stream, Persistance::page &p)
{
    stream >> p.data;
    stream >> p.lognr;
    return stream;
}

#endif // PERSISTANCE_H
