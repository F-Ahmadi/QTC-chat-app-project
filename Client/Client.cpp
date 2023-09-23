#include "Client.h"
#include "ui_Client.h"

Client::Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->mywidget);

    ui->connectbtn->setEnabled(true);
    ui->disconnectbtn->setEnabled(false);
    socket = new QTcpSocket(this);
    db = new MySqlDatabase(this);
    connect(this, &Client::newMessage, this, &Client::displayMessage);
    connect(socket, &QTcpSocket::readyRead, this, &Client::readyRead);
    connect(socket, &QTcpSocket::disconnected, this, &Client::disconnected);
    connect(socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::displayError);
    connect(ui->lineEdit_message,&QLineEdit::returnPressed,this,&Client::on_pushButton_sendMessage_clicked);
}

Client::~Client()
{
    if(socket->isOpen())
        socket->close();
    delete ui;
}

void Client::login()
{
    MycoonectDialog *dialog = new MycoonectDialog(this);
    if(dialog->exec()==QDialog::Accepted){
        username = dialog->returnmyusername();
        port = dialog->returnport();
    }
    // connect & check connecting:
    socket->connectToHost(QHostAddress(QHostAddress::LocalHost),port);
    if(socket->waitForConnected()){
        QMessageBox::information(this,"Client connecting...", QString("Successful connection."));
        // send username
        QDataStream socketStream(socket);
        socketStream.setVersion(QDataStream::Qt_5_12);

        QByteArray header;
        header.prepend(QString("fileType:Name,fileName:null,fileSize:%1;").arg(username.size()).toUtf8());
        header.resize(128);

        QByteArray byteArray = username.toUtf8();
        byteArray.prepend(header);

        socketStream << byteArray;
    }
    if(socket->state() == QAbstractSocket::ConnectedState){ // if the client is connected
        ui->statusBar->showMessage(QString("You are %1 .").arg(username));

        // save database
        QDateTime time = QDateTime::currentDateTime();
        db->AddValue(NULL,NULL,time.toString() ,QString(" --> %1 login to %2.").arg(username).arg("server"));

        //show history chat
        QDateTime mytoday= QDateTime::currentDateTime();

        QMap<QDateTime, QString> myMap = db->ShowValue(username);

        QMapIterator <QDateTime, QString> it(myMap);

        while(it.hasNext()){
            it.next();
            // show message
            ui->textBrowser_receivedMessages->setFontPointSize(9);
            ui->textBrowser_receivedMessages->append(it.value());


            //show message, time & date
            if(it.key().date() == mytoday.date()){
                 ui->textBrowser_receivedMessages->setFontPointSize(6);
                 ui->textBrowser_receivedMessages->append(it.key().time().toString()+"\n");



            }
            else{
                ui->textBrowser_receivedMessages->setFontPointSize(6);
                ui->textBrowser_receivedMessages->append(it.key().toString()+"\n");


            }
        }

    }
}

QString Client::showRate(unsigned long long int bytes, unsigned long long int duration)
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

QString Client::convertFileSizeToHumanReadable(float size)
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

void Client::readyRead()
{
    QByteArray buffer;

    QDataStream socketStream(socket);
    socketStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

    QElapsedTimer timer;
    timer.start();

    socketStream.startTransaction();
    socketStream >> buffer;

    if(!socketStream.commitTransaction())
    {
        duration =  timer.nsecsElapsed();
        QString message = QString(" %1 : Waiting for more data to come... speed : %2 ").arg(username).arg(this->showRate(socket->bytesAvailable()*10e9,duration));
        emit newMessage(message) ;
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

        if (QMessageBox::Yes == QMessageBox::question(this, "MyServer", QString("You are receiving an attachment from %1 with size: %2 , called %3. Do you want to accept it?").arg("Server").arg(size).arg(fileName)))
        {

            QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)+"/"+fileName, QString("File (*.%1)").arg(ext));

            QFile file(filePath);
            if(file.open(QIODevice::WriteOnly)){

                file.write(buffer);

                QString message = QString(" --> Attachment file with size %1 from %2 successfully stored on disk under the path %3").arg(size).arg("Server").arg(QString(filePath));
                emit newMessage(message) ;

                // save database
                QDateTime time = QDateTime::currentDateTime();
                db->AddValue(username,"server",time.toString(),message);

            }
            else
                QMessageBox::critical(this,"MyServer", "An error occurred while trying to write the attachment.");
        }

        else{
            QString message = QString(" --> Attachment from %1 discarded").arg("Server");
            emit newMessage(message) ;
        }
    }

    else if(fileType=="message"){
        QString message = QString(" %1 : %2").arg("Server").arg(QString::fromStdString(buffer.toStdString()));
        emit newMessage(message) ;

        // save database
        QDateTime time = QDateTime::currentDateTime();
        db->AddValue(username,"server",time.toString(),message);
    }
}

void Client::disconnected()
{
    socket->deleteLater();
    socket=nullptr;

    ui->statusBar->showMessage("Disconnected!");

    // save database
    QDateTime time = QDateTime::currentDateTime();
    db->AddValue(NULL,NULL,time.toString(),QString(" --> %1 disconnected from %2.").arg(username).arg("server"));

}

void Client::displayError(QAbstractSocket::SocketError socketError)
{
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
        break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, "MyClient", "The host was not found. Please check the host name and port settings.");
        break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, "MyClient", "The connection was refused by the peer. Make sure MyServer is running, and check that the host name and port settings are correct.");
        break;
        default:
            QMessageBox::information(this, "MyClient", QString("The following error occurred: %1.").arg("Server"));
            exit(EXIT_FAILURE);
        break;
    }
}

void Client::on_pushButton_sendMessage_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString str = ui->lineEdit_message->text();

            QString message = QString("%1 : %2").arg(username).arg(str);
            emit newMessage(message);

//            QTextCursor cursor = ui->textBrowser_receivedMessages->textCursor();
//            QTextBlockFormat textBlockFormat = cursor.blockFormat();
//            textBlockFormat.setAlignment(Qt::AlignLeft);
//            textBlockFormat.setBackground(QColor(166,222,240));
//            textBlockFormat.setNonBreakableLines(true);
//            textBlockFormat.setPageBreakPolicy(QTextFormat::PageBreak_AlwaysBefore);
//            cursor.mergeBlockFormat(textBlockFormat);
//            ui->textBrowser_receivedMessages->setTextCursor(cursor);

            QDataStream socketStream(socket);
            socketStream.setVersion(QDataStream::Qt_5_12);

            QByteArray header;
            header.prepend(QString("fileType:message,fileName:null,fileSize:%1;").arg(str.size()).toUtf8());
            header.resize(128);

            QByteArray byteArray = str.toUtf8();
            byteArray.prepend(header);

            socketStream << byteArray;

            ui->lineEdit_message->clear();

            // save database
            QDateTime time = QDateTime::currentDateTime();
            db->AddValue("server",username,time.toString(),QString(" %1 : %2 ").arg(username).arg(str));
        }
        else
            QMessageBox::critical(this,"MyClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"MyClient","Not connected");
}

void Client::on_pushButton_sendAttachment_clicked()
{
    if(socket)
    {
        if(socket->isOpen())
        {
            QString filePath = QFileDialog::getOpenFileName(this, ("Select an attachment"), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), ("File (*.json *.txt *.png *.jpg *.jpeg *.mp4 *.avi *.wmv *.mov *.mkv *.webm *.flv *.mp3)"));

            if(filePath.isEmpty()){
                QMessageBox::critical(this,"MyClient","You haven't selected any attachment!");
                return;
            }

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

                socketStream.setVersion(QDataStream::Qt_DefaultCompiledVersion);

                socketStream << byteArray;

                // save database
                float fsize = m_file.size();
                QString message = QString(" --> Attachment file with size %1 from %2 successfully stored on disk under the path %3").arg(this->convertFileSizeToHumanReadable(fsize)).arg(username).arg(QString(filePath));
                QDateTime time = QDateTime::currentDateTime();
                db->AddValue("server",username,time.toString(),message);

            }else
                QMessageBox::critical(this,"MyClient","Attachment is not readable!");
        }
        else
            QMessageBox::critical(this,"MyClient","Socket doesn't seem to be opened");
    }
    else
        QMessageBox::critical(this,"MyClient","Not connected");
}


void Client::displayMessage(const QString& str)
{
    ui->textBrowser_receivedMessages->setFontPointSize(9);
    ui->textBrowser_receivedMessages->append(str);
    QTime mytime = QTime::currentTime();
    ui->textBrowser_receivedMessages->setFontPointSize(6);
    ui->textBrowser_receivedMessages->append(mytime.toString()+"\n");
//    QTextCursor cursor = ui->textBrowser_receivedMessages->textCursor();
//    QTextBlockFormat textBlockFormat = cursor.blockFormat();
//    textBlockFormat.setAlignment(Qt::AlignRight);
//    textBlockFormat.setBackground(QColor("white"));
//    textBlockFormat.setNonBreakableLines(true);
//    textBlockFormat.setPageBreakPolicy(QTextFormat::PageBreak_Auto);
//    cursor.mergeBlockFormat(textBlockFormat);
//    ui->textBrowser_receivedMessages->setTextCursor(cursor);
}



void Client::on_connectbtn_clicked()
{
    this->login();
    ui->connectbtn->setEnabled(false);
    ui->disconnectbtn->setEnabled(true);
    //connect(ui->connectbtn,&QPushButton::clicked,this,&Client::login);به جاش می تونم بنویسم
}


void Client::on_disconnectbtn_clicked()
{
    this->~Client();
}





