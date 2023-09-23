#ifndef MYTHREAD_H
#define MYTHREAD_H

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QTime>
#include <QElapsedTimer>
#include <QDebug>

class MyThread : public QThread
{
    Q_OBJECT
public:
    explicit MyThread(QObject *parent = nullptr, quint64 FileSize=0);
    void run();
signals:
    void transferrate(QString rate);
private:
    quint64 Fs = 0;
};

#endif // MYTHREAD_H
