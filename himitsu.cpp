#include "himitsu.h"
#include "ui_himitsu.h"

/*
UI constructor. Connect relavant signals for handling user's input
*/
himitsu::himitsu(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::himitsu)
{
    ui->setupUi(this);
    // Handle adding friend button
    connect(ui->pushButton_2, SIGNAL(clicked()), this, SLOT(addFriend()));
    // Handle posting
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(post()));
    // Handle sharing (double click a message on wall to share)
    connect(ui->listWidget_2, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(sharePost(QListWidgetItem*)));
}

/*
Deconstructor. Save important info of Tox to the file tox.conf
*/
himitsu::~himitsu()
{
    uint8_t* data = new uint8_t [tox_size(my_tox)];
    tox_save(my_tox, data);

    std::fstream conf("tox.conf", std::ios::out|std::ios::binary|std::ios::trunc);
    conf.write(reinterpret_cast<char*>(data), tox_size(my_tox));


    qDebug() << "saving " << tox_size(my_tox) << " bytes";

    tox_kill(my_tox);
    delete ui;
}

/*
This is the main of our program. Initialize Tox, load data, connect to DHT. Then go to the while loop to handle both GUI and Tox
*/
void himitsu::mainLoop()
{
    // Initialize Tox
    Tox_Options option;
    option.ipv6enabled = 0;
    option.proxy_type = TOX_PROXY_NONE;
    option.udp_disabled = 0;

    initServer();

    my_tox = tox_new(&option);

    // Load tox if existed
    std::fstream conf("tox.conf", std::ios::in|std::ios::binary|std::ios::ate);
    if(conf.is_open())
    {

       std::streampos size = conf.tellg();
       uint8_t* memblock = new uint8_t [size];
       conf.seekg(0, std::ios::beg);
       conf.read(reinterpret_cast<char*>(memblock), (int)size);
       conf.close();

       if(sizeof(memblock) > 0)
       {
           qDebug() << "loading " << (int)size << " bytes";
           tox_load(my_tox, memblock, size);
       }
       delete[] memblock;
    }

    // Register callback function here
    tox_callback_connection_status(my_tox, updateFriendConnectionStatus, NULL);
    tox_callback_friend_message(my_tox, getMessage, NULL);

    // Print my ID to console. This ID is used for adding friend
    uint8_t *my_addr;
    my_addr = new uint8_t [38];
    tox_get_address(my_tox, my_addr);
    QString addr;
    hex_to_string(my_addr, addr, 38);
    qDebug() << "My id: " << addr;

    // Here I use info.conf for setting some user configuration. Currently only user's name. Can be extended to save/load other configuration as well
    std::fstream info("info.conf", std::ios::in);
    if(info.is_open())
    {
        uint8_t name[256];
        info.read(reinterpret_cast<char*>(name), 256);
        info.close();

        tox_set_name(my_tox, name, strlen((char*)name));
    }

    connectDHT();

    getFriendList();

    int online = 0, count = 0;
    while(running)
    {
        // Processing GUI event
        qApp->processEvents();

        // Processing Tox event
        tox_do(my_tox);

        // Update friend status if there is a change in friend's online status
        if(changed)
        {
            getFriendList();
            changed = false;
        }

        // Check if there is any incoming message and output it to wall
        updateWall();

        // Check if we are connected to the DHT
        if(tox_isconnected(my_tox) == 1 && online == 0)
        {
            qDebug() << "connected";
            online = 1;
        }
        else if(tox_isconnected(my_tox) == 0 && count > 10000)
        {
            // Reconnect to DHT in a fixed period if we are not connected
            connectDHT();
            count = 0;
            online = 0;
        }
        count++;
    }
}

/*
This function handles adding friend
*/
void himitsu::addFriend()
{
    // Open a new window to get friend's id and message we want to send while adding
    aFriend = new AddFriend();
    aFriend->setModal(true);
    aFriend->exec();
    getFriendID(aFriend->retFriendID());
    getAddMessage(aFriend->retMessage());

    // Send friend request
    if(sendFriendReq(fid, fmsg))
    {
        qDebug() << "Add friend successful";
    }
}

/*
Send friend request and prints out any error or success to console
*/
bool himitsu::sendFriendReq(QString & address, QString & message)
{
    bool success = false;
    uint8_t *addr, *msg;
    addr = new uint8_t [38];
    msg = new uint8_t [message.size()+1];
    string_to_hex(address, addr, 38);
    strcpy((char*)msg, message.toStdString().c_str());
    int32_t ret = tox_add_friend(my_tox, addr, msg, message.size()+1);
    if(ret == TOX_FAERR_TOOLONG) qDebug() << "message is too long";
    else if(ret == TOX_FAERR_NOMESSAGE) qDebug() << "message is too short";
    else if(ret == TOX_FAERR_OWNKEY) qDebug() << "Why are you adding yourself?";
    else if(ret == TOX_FAERR_ALREADYSENT) qDebug() << "Friend request was already sent";
    else if(ret == TOX_FAERR_UNKNOWN) qDebug() << "Unknown error";
    else if(ret == TOX_FAERR_BADCHECKSUM) qDebug() << "Invalid checksum";
    else if(ret == TOX_FAERR_SETNEWNOSPAM) qDebug() << "Invalid nospam";
    else if(ret == TOX_FAERR_NOMEM) qDebug() << "You've got too many friends";
    else
    {
        qDebug() << "Add success! Your friend's id is " << ret;
        success = true;
        getFriendList();
    }
    return success;
}

/*
A helper function to get friend's id from the addfriend window
*/
void himitsu::getFriendID(QString id)
{
    fid = id;
}

/*
A helper function to get message we want to send from the addfriend window
*/
void himitsu::getAddMessage(QString msg)
{
    fmsg = msg;
}

/*
Take message from the textEdit widget as input and post it onto our wall. Then broadcast the message to all online friends
*/
void himitsu::post()
{
    QListWidgetItem *item = new QListWidgetItem;
    if(ui->textEdit->toPlainText().size()>0)
    {
        /*
        We could add a more complicated object to the wall (a widget that has message and like button, for example).
        Also, wordwrap should be implemented here. The GUI of this still has wordwrap problems.
        Most importantly, make all the widget resize/scale with the main window.
        */

        // Get the post message and add it to our wall and clear the textEdit widget
        QString st = ui->textEdit->toPlainText();
        item->setText("Me: " + st);
        QColor online(0,0,255);
        item->setTextColor(online);
        ui->listWidget_2->addItem(item);
        ui->textEdit->clear();

        // Hash the current message so that we want show the same message if our friends want to share the same message back
        allMsg[st] = 1;

        // Convert message to Tox's wanted type
        std::string s = st.toStdString();
        uint8_t *msg = new uint8_t[st.size()];

        int i;
        for(i=0;i<s.size();i++)
        {
            msg[i] = s[i];
        }

        // Broadcast message to all online friends
        for(i=0;i<flist.size();i++)
        {
            if(tox_get_friend_connection_status(my_tox, flist[i].first) == 1)
            {
                qDebug() << "Send status: " << tox_send_message(my_tox, flist[i].first, msg, s.size());
            }
        }
    }
}

/*
Take widget item as input, then share the post to all online friends. Very similar to posting.
Note that this still has some problem with some special character like ':'. Need to fix.
*/
void himitsu::sharePost(QListWidgetItem *item)
{
    QString st = item->text();
    std::string s = st.toStdString();
    uint8_t *msg = new uint8_t[st.size()];

    int i, k=0;
    bool found = false;
    for(i=0;i<s.size();i++)
    {
        if(s[i] == ':')
        {
            found = true;
            i+=2;
        }
        if(found)
        {
            msg[k] = s[i];
            k++;
        }
    }

    // Broadcast message to all online friends
    for(i=0;i<flist.size();i++)
    {
        if(tox_get_friend_connection_status(my_tox, flist[i].first) == 1)
        {
            qDebug() << "Send status: " << tox_send_message(my_tox, flist[i].first, msg, k);
        }
    }
}

/*
Clear the list in the window. Fetch friend's name and online status from our friend list. Then show it in the window with corresponding colors (red = offline, blue = online).
*/
void himitsu::getFriendList()
{
    int32_t list[1024];
    int i, size;

    // Get friend list
    size = tox_get_friendlist(my_tox, list, 1024);

    // Clear friend list in the window
    ui->listWidget->clear();
    flist.clear();

    // Get friend's name and online status. Then show them in the friend list window
    for(i=0;i<size;i++)
    {
        QString st;
        uint8_t* name;
        int name_size = tox_get_name_size(my_tox, list[i]);

        // Show name if name is set, show friend's number otherwise
        if(name_size > 0)
        {
            name = new uint8_t[name_size];
            tox_get_name(my_tox, list[i], name);
            st = QString::fromLocal8Bit((char*)name);
        }
        else
        {
            st = QString::number(list[i]);
        }

        // Show friend+color according to onine status
        int ret = tox_get_friend_connection_status(my_tox, list[i]);
        if(ret == 0)
        {
            std::pair<int32_t, int> elem(list[i], 0);
            friendList.push_back(elem);
            flist.push_back(elem);
            QColor offline(255,0,0);
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(st);
            item->setTextColor(offline);
            ui->listWidget->addItem(item);
        }
        else if(ret == 1)
        {
            std::pair<int32_t, int> elem(list[i], 1);
            friendList.push_back(elem);
            flist.push_back(elem);
            QColor online(0,0,255);
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(st);
            item->setTextColor(online);
            ui->listWidget->addItem(item);
        }
        else qDebug() << "get friend " << list[i] << " status error";
    }
}

/*
Check the message queue. Dequeue+post the message if there is any to the wall.
*/
void himitsu::updateWall()
{
    while(!currMsg.empty())
    {
        Msg elem;
        elem = currMsg.front();
        std::stringstream sstm, stm;
        std::string s = "";
        uint8_t* name;
        int name_size = tox_get_name_size(my_tox, elem.fnum);

        // Print on the wall in this format, Name/ID: Message
        if(name_size > 0)
        {
            name = new uint8_t[name_size];
            for(int i=0; i<=name_size; i++) name[i] = '\0';
            tox_get_name(my_tox, elem.fnum, name);
            s += (char*)name;
        }
        else
        {
            name = new uint8_t[20];
            sprintf((char*)name, "%d", elem.fnum);
            s += (char*)name;
        }

        // Show the message if we haven't received before. Then hash the message so that we will not show duplicated messages in the future
        sstm << s << ": " << elem.msg;
        stm << elem.msg;
        QString st = QString::fromStdString(sstm.str());
        QString stt = QString::fromStdString(stm.str());
        if(allMsg[stt]==0)
        {
            QListWidgetItem *item = new QListWidgetItem;
            QColor offline(255,0,0);
            item->setText(st);
            item->setTextColor(offline);
            ui->listWidget_2->addItem(item);
            allMsg[stt] = 2;
        }
        currMsg.pop();
    }
}

/*
A helper function to convert hex byte to string
*/
void himitsu::hex_to_string(uint8_t *hex, QString & st, int size)
{
    int i;
    char map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
    for(i=0; i<size; i++)
    {
        int tmp = hex[i];
        tmp = tmp >> 4;
        st.append(map[tmp]);
        tmp = hex[i] & 15;
        st.append(map[tmp]);

    }
}

/*
A helper function to convert string to hex byte
*/
void himitsu::string_to_hex(QString & st, uint8_t *hex, int size)
{
    QMap<char, int> map;
    int i;
    map['0'] = 0;
    map['1'] = 1;
    map['2'] = 2;
    map['3'] = 3;
    map['4'] = 4;
    map['5'] = 5;
    map['6'] = 6;
    map['7'] = 7;
    map['8'] = 8;
    map['9'] = 9;
    map['a'] = 10;
    map['b'] = 11;
    map['c'] = 12;
    map['d'] = 13;
    map['e'] = 14;
    map['f'] = 15;
    QString lower = st.toLower();
    std::string l = lower.toStdString();
    for(i=0; i<size*2; i+=2)
    {
        hex[i/2] = (map[l[i]]<<4) + map[l[i+1]];
    }

}

/*
Handles closing window. Tell the main loop to stop.
*/
void himitsu::closeEvent(QCloseEvent *event)
{
    running = false;
    event->accept();
}

/*
A helper function to try connecting to all DHT node that located in the US
*/
void himitsu::connectDHT()
{
    for(int i=0; i<9; i++)
    {
        tox_bootstrap_from_address(my_tox, servers[i].address, servers[i].port, servers[i].pkey);

    }
}
