#ifndef LOGIN_H
#define LOGIN_H

#include <QMainWindow>
#include "sendreceive.h"
struct CFG
{
    QString filename_of_config;
    QString P1_Path;
    QString P2_Path;
};

namespace Ui {
class LogIn;
}

class LogIn : public QMainWindow
{
    Q_OBJECT

public:
    explicit LogIn(QWidget *parent = nullptr);
    ~LogIn();

private slots:
    void on_pushButton_exit_clicked();
    void on_pushButton_enter_clicked();
    void on_action_triggered();

private:
    Ui::LogIn *ui;
    SendReceive *SendReceiveObject;
    void load_config();
    void save_config();
};

#endif // LOGIN_H
