#ifndef HIMITSU_H
#define HIMITSU_H

#include <QMainWindow>
#include <QCloseEvent>
#include <tox/tox.h>
#include <addfriend.h>
#include <string>
#include <QString>
#include <QtDebug>
#include <iostream>
#include <QColor>
#include <QByteArray>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <fstream>
#include <QMap>
#include <vector>
#include <utility>
#include "global.h"
#include <sstream>
#include <QVBoxLayout>
#include <QListWidgetItem>



namespace Ui {
class himitsu;
}

class himitsu : public QMainWindow
{
    Q_OBJECT

public:
    // I know I should not put everything in public. But being relatively new to GUI programming myself, I find making everything public relativey easy to code.

    AddFriend *aFriend; // Add friend window
    Tox *my_tox;
    QString fid; // friend id from add friend window
    QString fmsg; // message we want to send from add friend window
    QMap<QString, int> allMsg; // hash map for checking duplicate messages (1 for I posted, 2 for my friend posted)
    std::vector<std::pair<int32_t, int> > flist; // friend list + online status (0 for offline, 1 for online)
    explicit himitsu(QWidget *parent = 0);
    void mainLoop();
    void getFriendID(QString id);
    void getAddMessage(QString msg);
    bool sendFriendReq(QString & address, QString & message);
    void getFriendList();
    void updateWall();
    void hex_to_string(uint8_t *hex, QString & st, int size);
    void string_to_hex(QString & st, uint8_t *hex, int size);
    void closeEvent(QCloseEvent *event);
    void connectDHT();
    bool running = true;
    ~himitsu();

private:
    Ui::himitsu *ui;



private slots:
    void addFriend();
    void post();
    void sharePost(QListWidgetItem *item);


};

#endif // HIMITSU_H
