#include "MySqlDatabase.h"
#include "ui_MySqlDatabase.h"

MySqlDatabase::MySqlDatabase(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MySqlDatabase)
{
    ui->setupUi(this);

    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("../mydatabase.sqlite");

    if(!db.open()){
        QMessageBox::critical(this,"HistoryChat","problem openning database!");
        return;
    }

    QString sQuery = "CREATE TABLE IF NOT EXISTS chathistory ("
                    "ID INTEGER PRIMARY KEY  AUTOINCREMENT,"
                    "reciver_name VARCHAR(50),"
                    "sender_name VARCHAR(50),"
                    "chat_time DATETIME NOT NULL,"
                    "message LONGTEXT);";

    QSqlQuery qry;

    if(!qry.exec(sQuery)){
        QMessageBox::critical(this,"HistoryChat",QString("Error creating table : %1").arg(db.lastError().text()));
        return;
    }

}

MySqlDatabase::~MySqlDatabase()
{
    db.close();
    delete ui;
}

QMap<QDateTime, QString> MySqlDatabase::ShowValue(QString name)
{
    QMap<QDateTime, QString> myMap;

    QSqlQuery qry;
    qry.prepare("SELECT * FROM 'chathistory' WHERE reciver_name=:name OR sender_name=:nname ORDER BY chat_time;");
    qry.bindValue(":name",name);
    qry.bindValue(":nname",name);

    if(qry.exec()){
        while(qry.next()){
            myMap.insert(QDateTime::fromString(qry.value(3).toString(),Qt::ISODateWithMs),qry.value(4).toString());
        }
    }

    else{
        QMessageBox::critical(this,"HistoryChat",QString("Error in show history chat of %1 : %2").arg(name).arg(qry.lastError().text()));
    }

    return myMap;
}

void MySqlDatabase::AddValue(QString reciver_name, QString sender_name,QString chat_time,QString message)
{
    QSqlQuery qry;
    qry.prepare("INSERT INTO chathistory("
                "reciver_name,"
                "sender_name,"
                "chat_time,"
                "message)"
                "VALUES (:reciver_name,:sender_name,:chat_time,:message);");

    qry.bindValue(":reciver_name",reciver_name);
    qry.bindValue(":sender_name",sender_name);
    qry.bindValue(":chat_time",QDateTime::fromString(chat_time));
    qry.bindValue(":message",message);
    if(!qry.exec()){
        QMessageBox::critical(this,"HistoryChat",QString("Error add value in database! %1").arg(qry.lastError().text()));
        return;
    }

}
