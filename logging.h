#ifndef LOGGING_H
#define LOGGING_H

#include <QMainWindow>

namespace Ui {
class Logging;
}

class Logging : public QMainWindow
{
    Q_OBJECT

public:
    explicit Logging(QWidget *parent = 0);
    ~Logging();

private slots:
    void on_automatic_clicked();

    void on_manual_clicked();

private:
    Ui::Logging *ui;
};

#endif // LOGGING_H
