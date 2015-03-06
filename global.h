#ifndef GLOBAL_H
#define GLOBAL_H
#include <vector>
#include <stdint.h>
#include <string>
#include <queue>
#include <tox/tox.h>
#include <himitsu.h>
#include <cstring>

typedef struct
{
    std::string msg;
    int32_t fnum;

}Msg;

typedef struct
{
    char address[16];
    uint16_t port;
    uint8_t pkey[TOX_CLIENT_ID_SIZE];
}Server;

extern std::vector<std::pair<int32_t, int> > friendList;
extern std::queue<Msg> currMsg;
extern bool changed;
extern std::vector<Server> servers;

void updateFriendConnectionStatus(Tox *tox, int32_t friendnumber, uint8_t status, void *userdata);
void getMessage(Tox *tox, int32_t friendnumber, const uint8_t *msg, uint16_t length, void *userdata);
void string_to_hex(QString & st, uint8_t *hex, int size);
void initServer();
#endif // GLOBAL_H

