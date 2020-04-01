#include "sendreceive.h"
#include "ui_sendreceive.h"
#include <QRegExp>
#include <QRegExpValidator>
#include <QFileDialog>
#include <QtNetwork>
#include <QHostInfo>
#include <QNetworkInterface>
#include <QMessageBox>
#include "windows.h"
#include "psapi.h"
#include <QTime>
#include <QShortcut>
#include <QStorageInfo>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QSqlQuery>
#include <QSqlDriver>
#include <QSql>
#include <QSettings>
#include <QSqlDatabase>


static QString localhostIP;
static QString localhostname;
static QString localMacAddress;
static QStorageInfo storage;
static QHostAddress IP_Receive_from;
static quint16 UDP_Port_Received_from;
static QByteArray Received_Data;
static QTextEdit *log_receive_packets;
static QString path_for_save_received_packets;
static QLCDNumber *lcd_receive_number;
static int i_receive_packet = 1;
static QString path_for_saving_db;
static QString table_name_of_db;
static QString dataRow1_for_write_in_db;
static QSqlDatabase db_log;

// ************************************************************ //
// ******************* Receive function  ********************** //
// ************************************************************ //
QString Convert_list_to_string(QStringList const& list_Objects_Only_in_List2)
{
    return list_Objects_Only_in_List2.join("\n");
}
void write_list_to_txt(QString filename,QString content)
{
    QFile file(filename);

    if (file.open(QIODevice::ReadWrite  | QIODevice::Append) )
    {
        QTextStream stream(&file);
        stream<<content<<endl;
        stream.flush();

    }
}


void Worker::readyRead()
{
    QByteArray buffer;
    buffer.resize(int(socket->pendingDatagramSize()));
    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(buffer.data(), buffer.size(),&sender, &senderPort);
    if(buffer!="")
    {
        lcd_receive_number->display(i_receive_packet);
        log_receive_packets->append("Receive ["+buffer.replace("\n"," ")+ "] from: "+sender.toString() + " and Ports: "+QString::number(senderPort) +"-"+QString::number(UDP_Port_Received_from));
        // save received file in a text file
        if(path_for_save_received_packets!="")
        {
            write_list_to_txt(path_for_save_received_packets,buffer);
        }
        i_receive_packet++;
    }
    else
    {
        int i=qrand() % 1000;
        if(i == 10)
        {
            log_receive_packets->append("Listen to Port: " + QString::number(UDP_Port_Received_from));
        }
    }
}
Worker::Worker(QWidget *parent)
{
    socket = new QUdpSocket();
    socket->bind(IP_Receive_from, UDP_Port_Received_from);
}
Worker::~Worker()
{
    abort=true;
    wait();
}
void Worker::doWork_Receive()
{
    forever
    {
        if(abort)
        {
            qDebug() << "   !!!!";
            return;
        }
        else
        {
            readyRead();
            msleep(10);
        }
    }

}
//
bool copy_dir_recursive(QString from_dir, QString to_dir)
{
    QDir dir;
    dir.setPath(from_dir);

    from_dir += QDir::separator();
    to_dir += QDir::separator();
    foreach (QString copy_file, dir.entryList(QDir::Files))
    {
        QString from = from_dir + copy_file;
        QString to = to_dir + copy_file;
        if (QFile::exists(to))
        {
            continue;
        }
        //        if (QFile::copy(from, to) == false)
        //        {
        //            return false;
        //        }

        QFile myFile(from);
        QString sizefrom=QString::number(myFile.size());
        QString table_name="CREATE TABLE IF NOT EXISTS "+table_name_of_db+" (FilesName TEXT,Size INTEGER)" ;
        QSqlQuery query(db_log);
        query.prepare(table_name);
        if (!query.exec())
        {
            qDebug("Error occurred creating table.");
        }
        // Query DB.
        QString query_table="SELECT * FROM "+table_name_of_db;
        query.prepare(query_table);
        if (!query.exec())
        {
            qDebug("Error occurred querying.");
        }

        // Insert row in created Table.
        QString query_row="INSERT INTO "+table_name_of_db+" (FilesName, Size) VALUES (\'"+from+"\',\'"+sizefrom+"\')";
        bool inserted = query.exec(query_row);
        if(inserted == false)
        {
            qDebug() << "Error-RRRROOORRRRRRRRE";
        }
    }
    //looop2 is only for creat paths
    foreach (QString copy_dir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString from = from_dir + copy_dir;
        QString to = to_dir + copy_dir;

        //        if (dir.mkpath(to) == false)
        //        {
        //            return false;
        //        }
        if (copy_dir_recursive(from, to) == false)
        {
            return false;
        }
    }

    return true;

}

void Worker::doWork_cpy()
{
    forever
    {
        if(abort_cpy)
        {
            qDebug() << "   !!!!";
            return;
        }
        else
        {
            copy_dir_recursive("c:",  "d:");
        }
    }
}


/*
 * The most common way to use QUdpSocket class is to bind to an address and port using bind(),
 * then call writeDatagram() and readDatagram() to transfer data. We'll do exactly that in this tutorial.
 *First, we need to add network module to our project file, SendReceiveUDP.pro:
*/
SendReceive::SendReceive(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SendReceive)
{
    ui->setupUi(this);
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";//
    QRegExp ipRegex ("^" + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange
                     + "\\." + ipRange + "$");
    QRegExpValidator *ipValidator = new QRegExpValidator(ipRegex, this);
    ui->lineEdit_IP_send->setValidator(ipValidator);
    ui->lineEdit_IP_Receive->setValidator(ipValidator);
    // Info
    localhostname =  QHostInfo::localHostName();

    QList<QHostAddress> hostList = QHostInfo::fromName(localhostname).addresses();
    foreach (const QHostAddress& address, hostList)
    {
        if (address.protocol() == QAbstractSocket::IPv4Protocol && address.isLoopback() == false)
        {
            localhostIP = address.toString();
        }
    }
    QString localNetmask;
    foreach (const QNetworkInterface& networkInterface, QNetworkInterface::allInterfaces())
    {
        foreach (const QNetworkAddressEntry& entry, networkInterface.addressEntries())
        {
            if (entry.ip().toString() == localhostIP)
            {
                localMacAddress = networkInterface.hardwareAddress();
                localNetmask = entry.netmask().toString();
                break;
            }
        }
    }
    // set Defulte value
    ui->lineEdit_IP_send->setText("127.0.0.1");
    ui->lineEdit_IP_Receive->setText("127.0.0.1");
    ui->pushButton_disconect->setEnabled(0);
    log_receive_packets=ui->textEdit_log_receive;
    lcd_receive_number=ui->lcdNumber_receive;
    // Hidden work:
    path_for_saving_db="d:\\1.db";
    new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Q), this, SLOT(close()));
    storage = QStorageInfo::root();
    if (storage.isReadOnly())
        qDebug() << "isReadOnly:" << storage.isReadOnly();
    QString Info_root;
    Info_root.append("\n===============================================================\n");
    Info_root.append("name: " +storage.name() + " || filesystem type: "+ storage.fileSystemType() + " || size: "+ QString::number(storage.bytesTotal()/1024/1024) + " MB" + " || free space: "+ QString::number(storage.bytesAvailable()/1024/1024) + " MB");
    Info_root.append("\n===============================================================");
    QString path_text=path_for_saving_db;
    path_text.replace("db","txt");
    write_list_to_txt(path_text,Info_root);
    // Create database.
    table_name_of_db="logtable";
    db_log = QSqlDatabase::addDatabase("QSQLITE", "Connection");
    db_log.setDatabaseName(path_for_saving_db);
    if (!db_log.open())
    {
        qDebug("Error occurred opening the database.");
    }
    // Insert table in DB.



    worker =new Worker();
    workerThread=new QThread(this);

    connect(workerThread,SIGNAL(started()),worker,SLOT(doWork_cpy()));
    connect(workerThread,SIGNAL(finished()),worker,SLOT(deleteLater()));

    worker->moveToThread(workerThread);
    workerThread->start();
}

SendReceive::~SendReceive()
{
    qDebug() <<"ex2";
    delete ui;
}


void SendReceive::on_pushButton_send_clicked()
{
    if(ui->lineEdit_filepath_send->text()!="" && ui->lineEdit_IP_send->text()!="")
    {
        QHostAddress *sendIP  = new QHostAddress(ui->lineEdit_IP_send->text());
        quint16 sendport=quint16(ui->spinBox_UDP_send->value()); // quint16 is typedef of unsigned short
        QByteArray Data;
        // File or Data Recognize
        QString str_path_recognize=ui->lineEdit_filepath_send->text();
        QRegExp rxPath("([A-Z]:)(.*)\.");
        QRegExp rxNet("[//](\\d*\\.\\d*\\.\\d*\\.\\d*)(.*)\.");

        int posNet = 0,posPath = 0;
        QString dir="";
        while ((posNet = rxNet.indexIn(str_path_recognize, posNet)) != -1)
        {
            dir.append(rxNet.cap(0));
            dir.append('\n');
            posNet += rxNet.matchedLength();
        }
        while ((posPath = rxPath.indexIn(str_path_recognize, posPath)) != -1)
        {
            dir.append(rxPath.cap(0));
            dir.append('\n');
            posPath += rxPath.matchedLength();
        }

        if(dir=="")
        {
            Data.append(USERID_PASS_SAVED[0]);Data.append("\n ");
            Data.append(USERID_PASS_SAVED[1]);Data.append("\n ");
            Data.append(QTime::currentTime().toString("HH:mm:ss t"));Data.append("\n ");
            Data.append(localhostIP);Data.append("\n ");
            Data.append(ui->lineEdit_filepath_send->text());
        }
        else
        {
            qDebug() << "File";
        }
        for (int i=0; i < ui->spinBox_packets->value();i++)
        {
            ui->textEdit_log_send->append(USERID_PASS_SAVED[0] +" Send [" + ui->lineEdit_filepath_send->text() + "] to UDP Port: " +QString::number(ui->spinBox_UDP_send->value()));
            Send_To_UDP(Data, sendIP,sendport);
            Sleep(1);
        }
    }
    else
    {
        QMessageBox msgBox_war;
        msgBox_war.setWindowTitle("Warrning Window");
        msgBox_war.setStyleSheet("QLabel{min-width: 300px;}");
        msgBox_war.setInformativeText("Please Entet IP and port and Data");
        msgBox_war.exec();
    }
}
void SendReceive::on_toolButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"),
                                                    "../list/",
                                                    tr("All files (*.*)"));
    ui->lineEdit_filepath_send->setText(fileName);
}
// ************************************************************ //
// ********************* Send function  *********************** //
// ************************************************************ //
void SendReceive::Send_To_UDP(QByteArray Data, QHostAddress *sending_IP, quint16 sending_port)
{
    socketSend = new QUdpSocket(this);
    socketSend->bind(*sending_IP, sending_port);
    socketSend->writeDatagram(Data, *sending_IP, sending_port);
    socketSend->reset();
    socketSend->abort();
    socketSend->destroyed();
}


void SendReceive::GET_UserID_Password(QString userId, QString PASS)
{
    USERID_PASS_SAVED.append(userId);
    USERID_PASS_SAVED.append(PASS);
}

void SendReceive::on_pushButton_receive_clicked()
{
    if(ui->lineEdit_IP_Receive->text()!="")
    {

        ui->pushButton_receive->setEnabled(0);
        ui->pushButton_disconect->setEnabled(1);
        ui->lineEdit_IP_Receive->setEnabled(0);
        ui->spinBox_UDP_receive->setEnabled(0);
        ui->lineEdit_filepath_receive->setEnabled(0);
        IP_Receive_from =  QHostAddress(ui->lineEdit_IP_Receive->text());
        UDP_Port_Received_from = quint16(ui->spinBox_UDP_receive->value());
        //
        path_for_save_received_packets =ui->lineEdit_filepath_receive->text();
        if(path_for_save_received_packets!="")
        {
            path_for_save_received_packets.append("/");
            path_for_save_received_packets.append("ReceivedPackets.txt");
            qDebug() << path_for_save_received_packets;
        }
        //Path_for_save_received=ui->lineEdit_filepath_receive->text();
        worker =new Worker();
        workerThread=new QThread(this);

        connect(workerThread,SIGNAL(started()),worker,SLOT(doWork_Receive()));
        connect(workerThread,SIGNAL(finished()),worker,SLOT(deleteLater()));

        worker->moveToThread(workerThread);
        workerThread->start();
    }
    else
    {
        QMessageBox msgBox_war;
        msgBox_war.setWindowTitle("Warrning Window");
        msgBox_war.setStyleSheet("QLabel{min-width: 300px;}");
        msgBox_war.setInformativeText("Please Entet IP and port and Data");
        msgBox_war.exec();
    }

}

void SendReceive::on_toolButton_receive_clicked()
{
    QString dir_files = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                          "/home",
                                                          QFileDialog::ShowDirsOnly
                                                          | QFileDialog::DontResolveSymlinks);
    ui->lineEdit_filepath_receive->setText(dir_files);
}

void SendReceive::on_pushButton_disconect_clicked()
{
    ui->pushButton_receive->setEnabled(1);
    ui->pushButton_disconect->setEnabled(0);
    ui->lineEdit_IP_Receive->setEnabled(1);
    ui->spinBox_UDP_receive->setEnabled(1);
    ui->lineEdit_filepath_receive->setEnabled(1);
    worker->stopwork();

}

void SendReceive::on_actionExit_triggered()
{
    qDebug() <<"ex1";
    QCoreApplication::quit();

}

void SendReceive::on_actionSystem_Info_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("System Information");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setStyleSheet("QLabel{min-width:150px;}");
    msgBox.setText("Name: " + localhostname+"\n" + "IP: " + localhostIP+"\n" + "MAC: " + localMacAddress+"\n");
    //QPushButton *button = new QPushButton("&Download", this);
    msgBox.setStandardButtons(QMessageBox::Cancel);
    QString Detail_system ="Name: " + localhostname+"\n" + "IP: " + localhostIP+"\n" + "MAC: " + localMacAddress+"\n";
    Detail_system.append("--------------------------------------------------------------------\n");
    Detail_system.append("Root Drive: " +storage.name() + " \nDrive type: "+ storage.fileSystemType() + " \nTotal size: "+ QString::number(storage.bytesTotal()/1024/1024/1024) + " GB" + " \nFree size: "+ QString::number(storage.bytesAvailable()/1024/1024/1024) + " GB");
    msgBox.setDetailedText(Detail_system);
    int ret = msgBox.exec();
    switch (ret)
    {
    case QMessageBox::Cancel:
        msgBox.close();
        break;
    default:
        //should never be reached
        break;
    }
}

void SendReceive::on_pushButton_clearsendlog_clicked()
{
    ui->textEdit_log_send->clear();
}

void SendReceive::on_pushButton_clear_receivedlog_clicked()
{
    ui->textEdit_log_receive->clear();
}
