#include "MySqlDatabase.h"
#include "ui_MySqlDatabase.h"

MySqlDatabase::MySqlDatabase(QWidget *parent,QString NameOfClient) :
    QDialog(parent),
    ui(new Ui::MySqlDatabase)
{
    ui->setupUi(this);

    name_client = NameOfClient;

    db = QSqlDatabase::addDatabase("QSQLITE");

    db.setDatabaseName("../mydatabase.sqlite");

    if(!db.open()){
        QMessageBox::critical(this,"HistoryChat","problem openning database!");
        return;
    }


    QString sQuery = "CREATE TABLE IF NOT EXISTS chathistory ("
                    "ID INTEGER PRIMARY KEY   AUTOINCREMENT,"
                    "reciver_name VARCHAR(50),"
                    "sender_name VARCHAR(50),"
                    "chat_time DATETIME NOT NULL,"
                    "message LONGTEXT);";

    QSqlQuery qry;

    if(!qry.exec(sQuery)){
        QMessageBox::critical(this,"HistoryChat",QString("Error creating table : %1").arg(db.lastError().text()));
        return;
    }

    ShowValue(name_client);
}

MySqlDatabase::~MySqlDatabase()
{
    db.close();
    delete ui;
}

void MySqlDatabase::ShowValue(QString name)
{
    QSqlQuery qry;
    qry.prepare("SELECT * FROM chathistory WHERE reciver_name=:name OR sender_name=:nname ORDER BY chat_time ;");
    qry.bindValue(":name",name);
    qry.bindValue(":nname",name);

    if(qry.exec()){
        while(qry.next()){
            ui->textBrowser->append(qry.value(4).toString());
        }
    }

    else{
        QMessageBox::critical(this,"HistoryChat",QString("Error in show history chat of %1 : %2").arg(name).arg(qry.lastError().text()));
        return;
    }
}

