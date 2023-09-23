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


namespace Ui {
class MySqlDatabase;
}

class MySqlDatabase : public QDialog
{
    Q_OBJECT

public:
    explicit MySqlDatabase(QWidget *parent = nullptr, QString NameOfClient = "");
    ~MySqlDatabase();
    void ShowValue(QString name);

private:
    Ui::MySqlDatabase *ui;
    QSqlDatabase db;
    QString name_client;
};

#endif // MYSQLDATABASE_H
