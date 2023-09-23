#include "Server.h"
#include "ui_Server.h"

Server::Server(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->widget);
    ui->stopbtn->setEnabled(false);
    ui->statusserver->setText("Server ...");

    m_server = new QTcpServer(this);

    connect(this, &Server::newMessage, this, &Server::displayMessage);
    connect(m_server, &QTcpServer::newConnection, this, &Server::newConnection);
    connect(ui->lineEdit_message,&QLineEdit::returnPressed,this,&Server::on_pushButton_sendMessage_clicked);
}

Server::~Server()
{
    foreach (auto * socket, connection_set_socket)
    {
        socket->deleteLater();
    }

    m_server->close();
    m_server->deleteLater();

    delete ui;
}

void Server::newConnection()
{
    while(m_server->hasPendingConnections()){
    QTcpSocket *socket = m_server->nextPendingConnection();
    appendToSocketList(socket);
    }
}

void Server::appendToSocketList(QTcpSocket* socket)
{
    connection_set_socket.append(socket);
    connect(socket, &QTcpSocket::readyRead, this, &Server::readyRead);
    connect(socket, &QTcpSocket::disconnected,this, &Server::disconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Server::displayError);
    displayMessage(QString("--> Client with sockd:%1 has just connected the server.").arg(socket->socketDescriptor()));
}

QString Server::showRate(unsigned long long int bytes, unsigned long long int duration)
{
    double Kbps = 0;
    double Mbps = 0;
    double Gbps = 0;
    double Tbps = 0;
    double Pbps = 0;
    double bps = (double)(bytes/duration);

    while (bps > 1024)
       {
           Kbps++;
           bps -= 1024;
       }
       while (Kbps > 1024)
       {
           Mbps++;
           Kbps -= 1024;
       }
       while (Mbps > 1024)
       {
           Gbps++;
           Mbps -= 1024;
       }
       while (Gbps > 1024)
       {
           Tbps++;
           Gbps -= 1024;
       }
       while (Tbps > 1024)
       {
           Pbps++;
           Tbps -= 1024;
       }


   if (Pbps > 0){
          double ret = (double)Pbps + ((double)((double)Tbps / 1024));
          QString s = QString::number(ret,'f',3);
          return ( s + " Pbps");
      }
   else if (Tbps > 0){
          double ret = (double)Tbps + ((double)((double)Gbps / 1024));
          QString s = QString::number(ret,'f',3);
          return ( s + " Tbps");
      }
   else if (Gbps > 0){
          double ret = (double)Gbps + ((double)((double)Mbps / 1024));
          QString s = QString::number(ret,'f',3);
          return( s + " Gbps");
      }
   else if (Mbps > 0){
          double ret = (double)Mbps + ((double)((double)Kbps / 1024));
          QString s = QString::number(ret,'f',3);
          return( s + " Mbps");
      }
   else if (Kbps > 0){
          double ret = (double)Kbps + ((double)((double)bps / 1024));
          QString s = QString::number(ret,'f',3);
          return( s + " Kbps");
      }
   else{
           QString s = QString::number(bps,'f',3);
           return( s + " bps");
          }
}

QString Server::convertFileSizeToHumanReadable(float size)
{
    QString ret;
     if      (size >= 1024*1024*1024) { ret = QString::number(size / 1073741824,'f',3) + " GB"; }
     else if (size >= 1024*1024)      { ret = QString::number(size / 1048576 ,'f' ,3) + " MB"; }
     else if (size >= 1024)       { ret = QString::number(size / 1024,'f', 3) + " KB"; }
     else if (size > 1)           { ret = QString::number(size) + " bytes"; }
     else if (size == 1)          { ret = QString::number(size) + " byte"; }
     else                          { ret = "0 byte"; }
     return ret;
}

void Server::readyRead()
{

    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_5_12);

    QElapsedTimer timer;
    timer.start();

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        duration =  timer.nsecsElapsed();
        QString message = QString("%1 : Waiting for more data to come... speed : %2").arg(MyHashName.value(socket->socketDescriptor())).arg(this->showRate(socket->bytesAvailable()*10e9,duration));
        emit newMessage(message);
        return;
    }

    QString header = buffer.mid(0,128);
    QString fileType = header.split(",")[0].split(":")[1];

    buffer = buffer.mid(128);

    if(fileType=="attachment"){
        QString fileName = header.split(",")[1].split(":")[1];
        QString ext = fileName.split(".")[1];
        QString size = header.split(",")[2].split(":")[1].split(";")[0];
        size = this->convertFileSizeToHumanReadable(size.toFloat());

        if (QMessageBox::Yes == QMessageBox::question(this, "QTCPServer", QString("You are receiving an attachment from %1 with size: %2 , called %3. Do you want to accept it?").arg(MyHashName.value(socket->socketDescriptor())).arg(size).arg(fileName)))
        {
            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+fileName, QString("File (*.%1)").arg(ext));

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){
                file.write(buffer);
                QString message = QString("--> Attachment from %1 successfully stored on disk under the path %2").arg(MyHashName.value(socket->socketDescriptor())).arg(QString(filePath));
                emit newMessage(message);
            }
            else
                QMessageBox::critical(this,"QTCPServer", "An error occurred while trying to write the attachment.");
        }
        else{
            QString message = QString("--> Attachment from %1 discarded").arg(MyHashName.value(socket->socketDescriptor()));
            emit newMessage(message);
        }
    }

    else if(fileType=="message"){
        QString message = QString("%1 : %2").arg(MyHashName.value(socket->socketDescriptor())).arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message);
    }
    else if(fileType=="Name"){
        QString message = QString("--> Username of Client %1 : %2").arg(socket->socketDescriptor()).arg(QString::fromStdString(buffer.toStdString()));
        MyHashName.insert(socket->socketDescriptor(),QString::fromStdString(buffer.toStdString()));
        connection_set_name.append(QString::fromStdString(buffer.toStdString()));
        refreshComboBox();
        emit newMessage(message);

    }
}

void Server::disconnected()
{
    QTcpSocket* socket = reinterpret_cast<QTcpSocket*>(sender());
    foreach(auto S, connection_set_socket){
        if (S == socket){
            displayMessage(QString("--> %1 disconnected from server.").arg(connection_set_name.at(connection_set_socket.indexOf(S))));
        }
    }
    connection_set_socket.removeAll(socket);
    connection_set_name.removeAt(connection_set_socket.indexOf(socket));
    MyHashName.remove(socket->socketDescriptor());

    refreshComboBox();

    socket->deleteLater();

}

void Server::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "MyServer", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "MyServer", "The connection was refused by the peer. Make sure MyServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QTcpSocket* socket = qobject_cast<QTcpSocket*>(sender());
            QMessageBox::information(this, "MyServer", QString("The following error occurred: %1.").arg(socket->errorString()));
        break;
    }
}

void Server::on_pushButton_sendMessage_clicked()
{
    QString receiver ;
    receiver = ui->comboBox_receiver->currentText();
    if(receiver=="Choose client"){
        QMessageBox::critical(this,"MyClient","You haven't selected any client!");
        return;
    }
    else{
    receiver = ui->comboBox_receiver->currentText().split(":")[1];
    }

    foreach (auto * socket,connection_set_socket)
    {
        if(socket->socketDescriptor() == receiver.toLongLong())
        {
            sendMessage(socket);
            break;
        }
    }

    ui->lineEdit_message->clear();
}


void Server::on_pushButton_sendAttachment_clicked()
{
    QString receiver ;
    receiver = ui->comboBox_receiver->currentText();
    if(receiver=="Choose client")
    {
        QMessageBox::critical(this,"MyClient","You haven't selected any client!");
        return;
    }
    else{
        receiver = ui->comboBox_receiver->currentText().split(":")[1];
    }
    QString filePath = QFileDialog::getOpenFileName(this, ("Select an attachment"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ("File (*.json *.txt *.png *.jpg *.jpeg *.mp4 *.avi *.wmv *.mov *.mkv *.webm *.flv *.mp3)"));

    if(filePath.isEmpty()){
        QMessageBox::critical(this,"MyClient","You haven't selected any attachment!");
        return;
    }

    foreach (auto * socket,connection_set_socket)
    {
        if(socket->socketDescriptor() == receiver.toLongLong())
        {
            sendAttachment(socket, filePath);
            break;
        }

    ui->lineEdit_message->clear();
    }
}

void Server::sendMessage(QTcpSocket* socket)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString str = ui->lineEdit_message->text();

            QString message = QString("Server : %1").arg(str);
            emit newMessage(message);

            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str.toUtf8();
            byteArray.prepend(header);

            socketStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);
            socketStream << byteArray;
        }
        else
            QMessageBox::critical(this,"MyServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"MyServer","Not connected");
}

void Server::sendAttachment(QTcpSocket* socket, QString filePath)
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QFile m_file(filePath);
            if(m_file.open(QIODevice::ReadOnly)){

                QFileInfo fileInfo(m_file.fileName());
                QString fileName(fileInfo.fileName());

                QDataStream socketStream(socket);
                socketStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

                QByteArray header;
                header.prepend(QString("fileType:attachment,fileName:%1,fileSize:%2;").arg(fileName).arg(m_file.size()).toUtf8());
                header.resize(128);

                QByteArray byteArray = m_file.readAll();
                byteArray.prepend(header);

                socketStream << byteArray;

            }else
                QMessageBox::critical(this,"MyClient","Couldn't open the attachment!");
        }
        else
            QMessageBox::critical(this,"MyServer","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"MyServer","Not connected");
}


void Server::displayMessage(const QString& str)
{
    ui->textBrowser_receivedMessages->setFontPointSize(9);
    ui->textBrowser_receivedMessages->append(str);
    QTime mytime = QTime::currentTime();
    ui->textBrowser_receivedMessages->setFontPointSize(6);
    ui->textBrowser_receivedMessages->append(mytime.toString()+"\n");
}

void Server::refreshComboBox(){
    ui->comboBox_receiver->clear();
    ui->comboBox_receiver->addItem("Choose client");
    foreach(auto * socket, connection_set_socket)
        ui->comboBox_receiver->addItem(QString("%1:%2").arg(MyHashName.value(socket->socketDescriptor())).arg(socket->socketDescriptor()));
}


void Server::on_startbtn_clicked()
{
    StartedCondition = true;
    statestartd();
}

void Server::on_stopbtn_clicked()
{
    StartedCondition = false;
    statestartd();
}


void Server::statestartd()
{
    if(StartedCondition==true){
        ui->startbtn->setEnabled(false);
        ui->stopbtn->setEnabled(true);

        if(m_server->listen(QHostAddress::LocalHost, ui->spinBox->value()))
        {
           ui->statusBar->showMessage("Server is listening...");
        }
        else
        {
            QMessageBox::critical(this,"MyServer",QString("Unable to start the server: %1.").arg(m_server->errorString()));
            exit(EXIT_FAILURE);
        }
        ui->statusserver->setText("Server started at localhost.");
    }

    else if(StartedCondition==false){
        ui->startbtn->setEnabled(true);
        ui->stopbtn->setEnabled(false);
        ui->statusserver->setText("Server stoped.");
        ui->statusBar->clearMessage();
        this->~Server();
    }
}


void Server::on_pushButton_history_clicked()
{
    QString receiver_descriptor ;
    QString reciver_name;
    receiver_descriptor = ui->comboBox_receiver->currentText();
    if(receiver_descriptor=="Choose client")
    {
        QMessageBox::critical(this,"MyClient","You haven't selected any client!");
        return;
    }
    else{
        receiver_descriptor = ui->comboBox_receiver->currentText().split(":")[1];
        reciver_name = ui->comboBox_receiver->currentText().split(":")[0];
    }

    MySqlDatabase *dioalog = new MySqlDatabase(this,reciver_name);
    dioalog->setWindowTitle("History chat");
    dioalog->setModal(true);
    dioalog->exec();



}

