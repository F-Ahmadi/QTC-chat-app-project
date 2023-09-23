#ifndef SERVER_H
#define SERVER_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QMetaType>
#include <QSet>
#include <QStandardPaths>
#include <QTcpServer>
#include <QTcpSocket>
#include <QList>
#include <QStringList>
#include <QHash>
#include <QElapsedTimer>
#include "MySqlDatabase.h"

class Client;


namespace Ui {
class MainWindow;
}

class Server : public QMainWindow
{
    Q_OBJECT

public:
    explicit Server(QWidget *parent = nullptr);
    ~Server();

signals:
    void newMessage(QString);
private slots:
    void newConnection();
    void appendToSocketList(QTcpSocket* socket);
    void readyRead();
    void disconnected();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);
    void sendMessage(QTcpSocket* socket);
    void sendAttachment(QTcpSocket* socket, QString filePath);

    void on_pushButton_sendMessage_clicked();
    void on_pushButton_sendAttachment_clicked();

    void refreshComboBox();

    void on_startbtn_clicked();

    void on_stopbtn_clicked();

    void on_pushButton_history_clicked();

private:
    Ui::MainWindow *ui;
    QTcpServer* m_server;
    QList<QTcpSocket*> connection_set_socket;
    QList<QString> connection_set_name;
    QHash<qintptr , QString> MyHashName;
    bool StartedCondition;
    void statestartd();
    QString showRate(unsigned long long int bytes , unsigned long long int duration);
    unsigned long long int  duration;
    QString convertFileSizeToHumanReadable(float size);
};

#endif // SERVER_H
