#ifndef PERSISTANCE_H
#define PERSISTANCE_H

#include <QReadWriteLock>
#include <QHash>
#include <QList>
#include <QDataStream>

class Persistance
{
public:
    struct page
    {
        QString data;
        qint64 transactions;
        qint64 lognr;
        QHash<qint64, QString> write_buffer;

        page() :
            data(),
            transactions(0),
            lognr(0),
            write_buffer()
        {
        }
    };

    Persistance();
    qint64 beginTransaction();
    void write(qint64 transaction_id, qint64 page_id, QString data);
    QString read(qint64 transaction_id, qint64 page_id);
    void commit(qint64 transaction_id);
    void rollback(qint64 transaction_id);

private:
    void load_dataset(qint64 page_id);
    void increase_log_number();

    static QReadWriteLock *_rwlock;
    static QHash<qint64, page> _buffer;
    static QHash<qint64, QList<qint64> > _transactions;
    static qint64 _lognr;

    static qint64 constexpr MAX_DATASETS = 5;
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
