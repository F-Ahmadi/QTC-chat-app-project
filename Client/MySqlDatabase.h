#ifndef MYSQLDATABASE_H
#define MYSQLDATABASE_H

#include <QDialog>
#include <QSql>
#include <QCoreApplication>
#include <QtDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QTime>
#include <QDate>
#include <QDateTime>
#include <QSqlError>
#include <QMessageBox>
#include <QMap>

namespace Ui {
class MySqlDatabase;
}

class MySqlDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit MySqlDatabase(QWidget *parent = nullptr);
    ~MySqlDatabase();
    QMap<QDateTime, QString> ShowValue(QString name);
    void AddValue(QString reciver_name,QString sender_name,QString chat_time,QString message);

private:
    Ui::MySqlDatabase *ui;
    QSqlDatabase db;
};

#endif // MYSQLDATABASE_H
