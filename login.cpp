#include "login.h"
#include "ui_login.h"
#include <QApplication>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDebug>
#include <QByteArray>
#include <QSettings>
static CFG config;
LogIn::LogIn(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LogIn)
{
    ui->setupUi(this);
    config.filename_of_config=QString("config.ini");
    load_config();
    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
   SendReceiveObject= new SendReceive();
}
LogIn::~LogIn()
{
    save_config();
    delete ui;
}
void LogIn::on_pushButton_exit_clicked()
{
    QCoreApplication::quit();

}

void LogIn::on_pushButton_enter_clicked()
{
    if (ui->lineEdit_userid->text()!="" && ui->lineEdit_password->text()!="")
    {
        //Check correct User ID and Paasword
        QString hash_password = QString("%1").arg(QString(QCryptographicHash::hash((ui->lineEdit_password->text)().toUtf8(),QCryptographicHash::Md5).toHex()));
        if ((ui->lineEdit_userid->text()=="host1" || ui->lineEdit_userid->text()=="host2")&& hash_password=="a384b6463fc216a5f8ecb6670f86456a")
        {
            SendReceiveObject->show();
            SendReceiveObject->GET_UserID_Password(ui->lineEdit_userid->text(),hash_password);
            LogIn::close();       

        }
        else
        {
            QMessageBox msgBox_war;
            msgBox_war.setWindowTitle("Warrning Window");
            msgBox_war.setStyleSheet("QLabel{min-width: 300px;}");
            msgBox_war.setInformativeText("UserID or Password is wrong");
            msgBox_war.exec();
        }
    }
    else
    {
        QMessageBox msgBox_war;
        msgBox_war.setWindowTitle("Warrning Window");
        msgBox_war.setStyleSheet("QLabel{min-width: 300px;}");
        msgBox_war.setInformativeText("Please Entet UserID and Password");
        msgBox_war.exec();
    }
}
void LogIn::load_config()
{
    QString FILENAME(config.filename_of_config);
    QSettings settings(FILENAME,QSettings::IniFormat);
    //read

    config.P1_Path=settings.value("Main/Input_Path1",QString()).toString();
    //config.P2_Path=settings.value("Main/Input_Path2",QString()).toString();
    // //////////////////////////////////////////////////////////////////////////////
    ui->lineEdit_userid->setText(config.P1_Path);
    //ui->lineEdit_password->setText(config.P2_Path);
}
void LogIn::save_config()
{
    QString FILENAME(config.filename_of_config);
    QSettings settings(FILENAME,QSettings::IniFormat);
    settings.setValue("Main/Input_Path1",ui->lineEdit_userid->text());
    //settings.setValue("Main/Input_Path2",ui->lineEdit_password->text());
}

void LogIn::on_action_triggered()
{
    ui->lineEdit_password->setText("qwert");
    ui->pushButton_enter->click();
}
