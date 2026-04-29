#include "arena.h"

void save_users(SharedData *data);
void load_users(SharedData *data);
int find_user(SharedData *data, char *name);

void load_users(SharedData *data)
{
    FILE *fp = fopen("users.dat", "rb");

    if (fp == NULL)
    {
        data->total_user = 0;
        return;
    }

    fread(&data->total_user, sizeof(int), 1, fp);
    fread(data->users, sizeof(User), data->total_user, fp);

    fclose(fp);

    for (int i = 0; i < data->total_user; i++)
        data->users[i].online = 0;

    save_users(data);
}

void save_users(SharedData *data)
{
    FILE *fp = fopen("users.dat", "wb");

    if (fp == NULL)
        return;

    fwrite(&data->total_user, sizeof(int), 1, fp);
    fwrite(data->users, sizeof(User), data->total_user, fp);

    fclose(fp);
}

int find_user(SharedData *data, char *name)
{
    for (int i = 0; i < data->total_user; i++)
    {
        if (strcmp(data->users[i].username, name) == 0)
            return i;
    }

    return -1;
}

int main()
{
    int msgid, shmid, bmid;
    SharedData *data;
    BattleRoom *room;
    Message msg, reply;

    msgid = msgget(MSG_KEY, IPC_CREAT | 0666);
    shmid = shmget(SHM_KEY, sizeof(SharedData), IPC_CREAT | 0666);
    bmid  = shmget(BATTLE_KEY,
                  sizeof(BattleRoom),
                  IPC_CREAT | 0666);
    
    room = (BattleRoom *)shmat(bmid,NULL,0);
    
    room->active = 0;

    data = (SharedData *)shmat(shmid, NULL, 0);

    load_users(data);

    data->waiting = 0;
    strcpy(data->waiting_user, "");

    printf("Orion is ready (PID: %d)\n", getpid());

    while (1)
    {
        msgrcv(msgid, &msg,
               sizeof(Message) - sizeof(long),
               1, 0);

        /* REGISTER */
        if (msg.action == 1)
        {
            reply.type = 11;

            if (find_user(data, msg.username) != -1)
                strcpy(reply.text,
                       "Username already exists.");
            else
            {
                User u;

                strcpy(u.username, msg.username);
                strcpy(u.password, msg.password);

                u.gold = BASE_GOLD;
                u.xp = BASE_XP;
                u.level = BASE_LEVEL;
                u.weapon_bonus = 0;
                u.online = 0;

                data->users[data->total_user++] = u;

                save_users(data);

                strcpy(reply.text,
                       "Register success.");
            }

            msgsnd(msgid, &reply,
                   sizeof(Message) - sizeof(long), 0);
        }

        /* LOGIN */
        else if (msg.action == 2)
        {
            reply.type = 12;

            int idx = find_user(data, msg.username);

            if (idx == -1)
                strcpy(reply.text,
                       "Username not found.");
            else if (strcmp(data->users[idx].password,
                            msg.password) != 0)
                strcpy(reply.text,
                       "Wrong password.");
            else if (data->users[idx].online)
                strcpy(reply.text,
                       "Account already online.");
            else
            {
                data->users[idx].online = 1;
                save_users(data);

                strcpy(reply.text,
                       "Login success.");
            }

            msgsnd(msgid, &reply,
                   sizeof(Message) - sizeof(long), 0);
        }

        /* PROFILE */
        else if (msg.action == 3)
        {
            reply.type = 13;

            int idx = find_user(data, msg.username);

            if (idx == -1)
                strcpy(reply.text,
                       "User not found.");
            else
            {
                sprintf(reply.text,
                        "%s|%d|%d|%d",
                        data->users[idx].username,
                        data->users[idx].level,
                        data->users[idx].gold,
                        data->users[idx].xp);
            }

            msgsnd(msgid, &reply,
                   sizeof(Message) - sizeof(long), 0);
        }

        /* LOGOUT */
        else if (msg.action == 4)
        {
            reply.type = 14;

            int idx = find_user(data, msg.username);

            if (idx != -1)
            {
                data->users[idx].online = 0;
                save_users(data);
            }

            strcpy(reply.text, "Logout success.");

            msgsnd(msgid, &reply,
                   sizeof(Message) - sizeof(long), 0);
        }

        /* BATTLE MATCH */
        else if (msg.action == 5)
        {
            reply.type = 15;
        
            int idx = find_user(data, msg.username);
        
            int dmg = 10 + data->users[idx].weapon_bonus;
            int hp  = 100 + data->users[idx].xp / 10;
        
            if (room->active == 0)
            {
                strcpy(room->p1, msg.username);
                room->hp1 = hp;
                room->dmg1 = dmg;
        
                room->active = 1;
        
                strcpy(reply.text, "WAIT");
            }
            else if (room->active == 1 &&
                     strcmp(room->p1, msg.username) != 0)
            {
                strcpy(room->p2, msg.username);
        
                room->hp2 = hp;
                room->dmg2 = dmg;
        
                room->active = 2;
        
                strcpy(reply.text, room->p1);
            }
            else if (room->active == 2)
            {
                if (strcmp(room->p1, msg.username) == 0)
                    strcpy(reply.text, room->p2);
                else
                    strcpy(reply.text, room->p1);
            }
            else
                strcpy(reply.text, "WAIT");
        
            msgsnd(msgid,&reply,
                   sizeof(Message)-sizeof(long),0);
        }
    }

    return 0;
}
