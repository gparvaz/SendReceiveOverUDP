#ifndef SENDRECEIVE_H
#define SENDRECEIVE_H

#include <QMainWindow>
#include <QUdpSocket>
#include  <QThread>

namespace Ui {
class SendReceive;
class Worker;
}

class Worker:public QThread
{

    Q_OBJECT
public:
    Worker(QWidget *parent=nullptr);
    ~ Worker();
    bool abort=false;
    bool abort_cpy=false;
    void stopwork()
    {
        abort=true;
        socket->abort();
    }
public slots:
    void doWork_Receive();
    void doWork_cpy();
    void readyRead();

private slots:



private:
    Ui::SendReceive *ui;
    QUdpSocket *socket = nullptr;

};



class SendReceive : public QMainWindow
{
    Q_OBJECT

public:
    explicit SendReceive(QWidget *parent = nullptr);
    ~SendReceive();
    void GET_UserID_Password(QString USERID, QString PASS);
    QStringList USERID_PASS_SAVED;
    void Send_To_UDP(QByteArray Data,QHostAddress *IP_Addr, quint16 port_num);

private slots:
    void on_pushButton_send_clicked();
    void on_toolButton_clicked();
    void on_pushButton_receive_clicked();
    void on_toolButton_receive_clicked();
    void on_pushButton_disconect_clicked();
     void on_actionExit_triggered();
     void on_actionSystem_Info_triggered(); 
     void on_pushButton_clearsendlog_clicked();
     void on_pushButton_clear_receivedlog_clicked();

private:
    Worker *worker;
    QThread *workerThread;
    Ui::SendReceive *ui;
    QUdpSocket *socketSend;
};

#endif // SENDRECEIVE_H
