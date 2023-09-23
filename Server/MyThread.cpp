#include "MyThread.h"


MyThread::MyThread(QObject *parent,quint64 FileSize) : QThread(parent)
{
    Fs = FileSize;
}

void MyThread::run()
{
    QMutex mte;
    QTime start = QTime::currentTime();

    quint64 bytes = 0;
    quint64 Kbytes = 0;
    quint64 Mbytes = 0;
    quint64 Gbytes = 0;
    quint64 Tbytes = 0;
    quint64 Pbytes = 0;
    while(1){
        mte.lock();
        if(bytes==Fs){
            qint64 duration = start.elapsed(); //milisec
            if (Pbytes > 0){
                   double ret = (double)Pbytes + ((double)((double)Tbytes / 1024));
                   QString s = QString::number(ret/duration);
                   emit transferrate( s + " Pbyte/s");
               }
            else if (Tbytes > 0){
                   double ret = (double)Tbytes + ((double)((double)Gbytes / 1024));
                   QString s = QString::number(ret/duration);
                   emit transferrate( s + " Tbyte/s");
               }
            else if (Gbytes > 0){
                   double ret = (double)Gbytes + ((double)((double)Mbytes / 1024));
                   QString s = QString::number(ret/duration);
                   emit transferrate( s + " Gbyte/s");
               }
            else if (Mbytes > 0){
                   double ret = (double)Mbytes + ((double)((double)Kbytes / 1024));
                   QString s = QString::number(ret/duration);
                   emit transferrate( s + " Mbyte/s");
               }
            else if (Kbytes > 0){
                   double ret = (double)Kbytes + ((double)((double)bytes / 1024));
                   QString s = QString::number(ret/duration);
                   emit transferrate( s + " Kbyte/s");
               }
            else{
                    QString s = QString::number(bytes/duration);
                    emit transferrate( s + " byte/s");
               }


            break;
        }
        bytes += 8;    //add 1 byts from start point--->1 byte = 8 bits
        mte.unlock();

        while (bytes > 1024)
        {
            Kbytes++;
            bytes -= 1024;
        }
        while (Kbytes > 1024)
        {
            Mbytes++;
            Kbytes -= 1024;
        }
        while (Mbytes > 1024)
        {
            Gbytes++;
            Mbytes -= 1024;
        }
        while (Gbytes > 1024)
        {
            Tbytes++;
            Gbytes -= 1024;
        }
        while (Tbytes > 1024)
        {
            Pbytes++;
            Tbytes -= 1024;
        }


//            ------------      show rate      ---------------
//    qint64 duration = start.elapsed(); //milisec
//    if( duration % 500 == 0) { // every 500 milisec show data transfer rate

//        if (Pbytes > 0){
//               double ret = (double)Pbytes + ((double)((double)Tbytes / 1024));
//               QString s = QString::number(ret/duration);
//               emit transferrate( s + " Pbyte/s");
//           }
//        else if (Tbytes > 0){
//               double ret = (double)Tbytes + ((double)((double)Gbytes / 1024));
//               QString s = QString::number(ret/duration);
//               emit transferrate( s + " Tbyte/s");
//           }
//        else if (Gbytes > 0){
//               double ret = (double)Gbytes + ((double)((double)Mbytes / 1024));
//               QString s = QString::number(ret/duration);
//               emit transferrate( s + " Gbyte/s");
//           }
//        else if (Mbytes > 0){
//               double ret = (double)Mbytes + ((double)((double)Kbytes / 1024));
//               QString s = QString::number(ret/duration);
//               emit transferrate( s + " Mbyte/s");
//           }
//        else if (Kbytes > 0){
//               double ret = (double)Kbytes + ((double)((double)bytes / 1024));
//               QString s = QString::number(ret/duration);
//               emit transferrate( s + " Kbyte/s");
//           }
//        else{
//                QString s = QString::number(bytes/duration);
//                emit transferrate( s + " byte/s");
//           }

//        }//if(duration % 500 == 0)
    }//end of while
}

