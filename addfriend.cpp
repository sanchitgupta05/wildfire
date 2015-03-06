#include "addfriend.h"
#include "ui_addfriend.h"

AddFriend::AddFriend(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddFriend)
{
    ui->setupUi(this);
}

AddFriend::~AddFriend()
{
    delete ui;
}

QString AddFriend::retFriendID()
{
    QString fid = ui->plainTextEdit->toPlainText();

    return fid;
}

QString AddFriend::retMessage()
{
    QString fmsg = ui->plainTextEdit_2->toPlainText();

    return fmsg;

}
