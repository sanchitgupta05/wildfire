#ifndef ADDFRIEND_H
#define ADDFRIEND_H

#include <QDialog>
#include <QString>
#include <QtDebug>

namespace Ui {
class AddFriend;
}

class AddFriend : public QDialog
{
    Q_OBJECT

public:
    explicit AddFriend(QWidget *parent = 0);
     QString retFriendID();
     QString retMessage();
    ~AddFriend();

private:
    Ui::AddFriend *ui;

private slots:

};

#endif // ADDFRIEND_H
