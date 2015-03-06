#include "global.h"

/*
Extern variables used for handling Tox's function callback.
*/
std::vector<std::pair<int32_t, int> > friendList; // For taking updated friend's online status
std::queue<Msg> currMsg; // A queue for incoming messages
bool changed = false; // Indicate there is a change in frineds' online status (can be extended to name change or other things as well
std::vector<Server> servers; // Existing US DHT nodes

/*
A helper function to convert from a string of type QString into hex byte for transferring through Tox
*/
void string_to_hex(QString & st, uint8_t *hex, int size)
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
Bootstrap servers that located in the US
*/
void initServer()
{
    Server s1;
    strcpy(s1.address, "192.254.75.102");
    s1.port = 33445;
    QString st1("951C88B7E75C867418ACDB5D273821372BB5BD652740BCDF623A4FA293E75D2F");
    string_to_hex(st1, s1.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s1);

    Server s2;
    strcpy(s2.address, "23.226.230.47");
    s2.port = 33445;
    QString st2("A09162D68618E742FFBCA1C2C70385E6679604B2D80EA6E84AD0996A1AC8A074");
    string_to_hex(st2, s2.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s2);

    Server s3;
    strcpy(s3.address, "178.62.125.224");
    s3.port = 33445;
    QString st3("10B20C49ACBD968D7C80F2E8438F92EA51F189F4E70CFBBB2C2C8C799E97F03E");
    string_to_hex(st3, s3.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s3);

    Server s4;
    strcpy(s4.address, "192.210.149.121");
    s4.port = 33445;
    QString st4("F404ABAA1C99A9D37D61AB54898F56793E1DEF8BD46B1038B9D822E8460FAB67");
    string_to_hex(st4, s4.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s4);

    Server s5;
    strcpy(s5.address, "104.219.184.206");
    s5.port = 443;
    QString st5("8CD087E31C67568103E8C2A28653337E90E6B8EDA0D765D57C6B5172B4F1F04C");
    string_to_hex(st5, s5.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s5);

    Server s6;
    strcpy(s6.address, "76.191.23.96");
    s6.port = 33445;
    QString st6("93574A3FAB7D612FEA29FD8D67D3DD10DFD07A075A5D62E8AF3DD9F5D0932E11");
    string_to_hex(st6, s6.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s6);

    Server s7;
    strcpy(s7.address, "192.3.173.88");
    s7.port = 33445;
    QString st7("3E1FFDEB667BFF549F619EC6737834762124F50A89C8D0DBF1DDF64A2DD6CD1B");
    string_to_hex(st7, s7.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s7);

    Server s8;
    strcpy(s8.address, "205.185.116.116");
    s8.port = 33445;
    QString st8("A179B09749AC826FF01F37A9613F6B57118AE014D4196A0E1105A98F93A54702");
    string_to_hex(st8, s8.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s8);

    Server s9;
    strcpy(s9.address, "198.98.51.198");
    s9.port = 33445;
    QString st9("1D5A5F2F5D6233058BF0259B09622FB40B482E4FA0931EB8FD3AB8E7BF7DAF6F");
    string_to_hex(st9, s9.pkey, TOX_CLIENT_ID_SIZE);
    servers.push_back(s9);
}

/*
Callback handler when friend(s) change online status. Write changes to an extern variable friendList
*/
void updateFriendConnectionStatus(Tox *tox, int32_t friendnumber, uint8_t status, void *userdata)
{
    int i;
    for(i=0;i<friendList.size();i++)
    {
        if(friendList[i].first == friendnumber)
        {
            friendList[i].second = status;
            break;
        }
    }
    changed = true;
}

/*
Callback handler when there is incoming message from friend. Write incoming message to the extern queue currMsg
*/
void getMessage(Tox *tox, int32_t friendnumber, const uint8_t *msg, uint16_t length, void *userdata)
{
    std::string st((const char*)msg);
    std::string name;
    Msg elem;
    elem.fnum = friendnumber;
    elem.msg = st;
    currMsg.push(elem);
}
