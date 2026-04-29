#ifndef ARENA_H
#define ARENA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>

#define MAX_USERS 100
#define MAX_TEXT 256

#define MSG_KEY 0x00001234
#define SHM_KEY 0x00009012
#define BATTLE_KEY 0x00005678
#define BASE_GOLD 150
#define BASE_XP 0
#define BASE_LEVEL 1

typedef struct
{
    char username[50];
    char password[50];

    int gold;
    int xp;
    int level;

    int weapon_bonus;
    int online;

} User;

typedef struct
{
    User users[MAX_USERS];
    int total_user;

    char waiting_user[50];
    int waiting;

} SharedData;

typedef struct
{
    char p1[50];
    char p2[50];

    int hp1;
    int hp2;

    int dmg1;
    int dmg2;

    int active;

} BattleRoom;

typedef struct
{
    long type;
    int action;

    char username[50];
    char password[50];
    char text[MAX_TEXT];

} Message;

#endif
