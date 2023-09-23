#ifndef CLIENT_H
#define CLIENT_H

#include <QMainWindow>
#include <QAbstractSocket>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHostAddress>
#include <QMessageBox>
#include <QMetaType>
#include <QString>
#include <QStandardPaths>
#include <QTcpSocket>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QByteArray>
#include <QDataStream>
#include <limits>
#include <QElapsedTimer>
#include <QTime>
#include <QTextCursor>
#include <QSql>
#include <QScrollBar>
#include "mycoonectdialog.h"
#include "MySqlDatabase.h"


namespace Ui {
class MainWindow;
}

class Client : public QMainWindow
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);
    ~Client();
signals:
    void newMessage(QString);

public slots:
    void login();

    void readyRead();
    void disconnected();
    void displayError(QAbstractSocket::SocketError socketError);

    void displayMessage(const QString& str);

    void on_pushButton_sendMessage_clicked();
    void on_pushButton_sendAttachment_clicked();

private:
    Ui::MainWindow *ui;
    QTcpSocket* socket;
    quint64 port;
    QString username;
    QString showRate(unsigned long long int bytes , unsigned long long int duration);
    unsigned long long int  duration;
    QString convertFileSizeToHumanReadable(float size);
    MySqlDatabase *db;

private slots:
    void on_connectbtn_clicked();
    void on_disconnectbtn_clicked();
};

#endif // CLIENT_H
