#include "arena.h"
#include <time.h>

char current_user[50];

void save_history(char enemy[], int win)
{
    FILE *fp;
    char filename[100];

    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    sprintf(filename, "%s_history.txt",
            current_user);

    fp = fopen(filename, "a");

    if (fp == NULL)
        return;

    fprintf(fp,
        "%02d:%02d | %-12s | %s | +%d XP\n",
        tm_info->tm_hour,
        tm_info->tm_min,
        enemy,
        win ? "WIN " : "LOSS",
        win ? 50 : 15);

    fclose(fp);
}

void reward_result(int win)
{
    int shmid;
    SharedData *data;

    shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    data = (SharedData *)shmat(shmid, NULL, 0);

    for (int i = 0; i < data->total_user; i++)
    {
        if (strcmp(data->users[i].username, current_user) == 0)
        {
            if (win)
            {
                data->users[i].xp += 50;
                data->users[i].gold += 120;
            }
            else
            {
                data->users[i].xp += 15;
                data->users[i].gold += 30;
            }

            data->users[i].level =
                (data->users[i].xp / 100) + 1;
        }
    }

    shmdt(data);
}

void battle_arena(char enemy[])
{
    int hp = 100;
    int enemy_hp = 100;
    int dmg = 10;
    
    int shmid;
    SharedData *data;
    
    shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    data = (SharedData *)shmat(shmid, NULL, 0);
    
    for (int i = 0; i < data->total_user; i++)
    {
        if (strcmp(data->users[i].username,
                   current_user) == 0)
        {
            dmg += data->users[i].weapon_bonus;
            hp += data->users[i].xp / 10;
            break;
        }
    }
    
    shmdt(data);

    char cmd[10];
    time_t last = 0;

    while (hp > 0 && enemy_hp > 0)
    {
        system("clear");

        printf("=========== ARENA ===========\n");
        printf("%s VS %s\n\n",
               current_user, enemy);

        printf("Enemy HP : %d\n", enemy_hp);
        printf("Your HP  : %d\n\n", hp);

        printf("Press a = Attack\n");
        printf("Press u = Ultimate\n");
        printf("> ");

        scanf("%9s", cmd);

        time_t now = time(NULL);

        if (strcmp(cmd, "a") == 0)
        {
            if (now - last >= 1)
            {
                enemy_hp -= dmg;
                printf("You hit for %d damage!\n", dmg);
                last = now;
            }
            else
                printf("Cooldown!\n");
        }

        else if (strcmp(cmd, "u") == 0)
        {
            enemy_hp -= dmg * 3;
            printf("ULTIMATE HIT!\n");
        }

        sleep(1);

        if (enemy_hp > 0)
        {
            hp -= 8;
            printf("%s attacks you!\n", enemy);
            sleep(1);
        }
    }

    if (hp <= 0 && enemy_hp > 0)
    {
        printf("\n=== DEFEAT ===\n");
        reward_result(0);
        save_history(enemy, 0);
    }
    else if (enemy_hp <= 0 && hp > 0)
    {
        printf("\n=== VICTORY ===\n");
        reward_result(1);
        save_history(enemy, 1);
    }
    else
    {
        printf("\n=== DEFEAT ===\n");
        reward_result(0);
        save_history(enemy, 0);
    }

    printf("Press ENTER...");
    getchar();
    getchar();
}

void realtime_battle(char enemy[])
{
    int bmid;
    BattleRoom *room;

    bmid = shmget(BATTLE_KEY,
                  sizeof(BattleRoom), 0666);

    room = (BattleRoom *)shmat(bmid, NULL, 0);

    int me = 1;
    int result = -1;

    time_t last_attack = 0;
    time_t last_ult = 0;

    if (strcmp(room->p2, current_user) == 0)
        me = 2;

    while (1)
    {
        system("clear");

        int myhp, enemyhp, mydmg;

        if (me == 1)
        {
            myhp = room->hp1;
            enemyhp = room->hp2;
            mydmg = room->dmg1;
        }
        else
        {
            myhp = room->hp2;
            enemyhp = room->hp1;
            mydmg = room->dmg2;
        }

        printf("=========== ARENA ===========\n");
        printf("%s VS %s\n\n",
               current_user, enemy);

        printf("Enemy HP : %d\n", enemyhp);
        printf("Your HP  : %d\n\n", myhp);

        printf("a = Attack (1s CD)\n");
        printf("u = Ultimate (5s CD)\n");
        printf("r = Refresh\n");
        printf("> ");

        if (myhp <= 0)
        {
            printf("=== DEFEAT ===\n");
            result = 0;
            break;
        }

        if (enemyhp <= 0)
        {
            printf("=== VICTORY ===\n");
            result = 1;
            break;
        }

        char cmd[10];
        scanf("%9s", cmd);

        time_t now = time(NULL);

        if (strcmp(cmd, "a") == 0)
        {
            if (now - last_attack < 1)
            {
                printf("Attack cooldown!\n");
                sleep(1);
                continue;
            }

            if (me == 1)
                room->hp2 -= mydmg;
            else
                room->hp1 -= mydmg;

            last_attack = now;
        }

        else if (strcmp(cmd, "u") == 0)
        {
            if (mydmg <= 10)
            {
                printf("Need weapon first!\n");
                sleep(1);
                continue;
            }

            if (now - last_ult < 5)
            {
                printf("Ultimate cooldown!\n");
                sleep(1);
                continue;
            }

            int ult = mydmg * 3;

            if (me == 1)
                room->hp2 -= ult;
            else
                room->hp1 -= ult;

            last_ult = now;
        }

        else if (strcmp(cmd, "r") == 0)
        {
            continue;
        }

        if (room->hp1 < 0) room->hp1 = 0;
        if (room->hp2 < 0) room->hp2 = 0;
    }

    if (result == 1)
    {
        reward_result(1);
        save_history(enemy, 1);
    }
    else
    {
        reward_result(0);
        save_history(enemy, 0);
    }

    if (me == 1)
        room->active = 0;

    shmdt(room);

    printf("Press ENTER...");
    getchar();
    getchar();
}

void battle_menu(int msgid)
{
    Message msg;

    printf("\nSearching opponent...\n");

    for (int t = 35; t >= 1; t--)
    {
        msg.type = 1;
        msg.action = 5;

        strcpy(msg.username, current_user);

        msgsnd(msgid, &msg,
               sizeof(Message)-sizeof(long), 0);

        msgrcv(msgid, &msg,
               sizeof(Message)-sizeof(long),
               15, 0);

        if (strcmp(msg.text, "WAIT") != 0)
        {
            printf("\nOpponent found: %s\n",
                   msg.text);

            sleep(1);

            realtime_battle(msg.text);
            return;
        }

        printf("\rSearching... [%d] ", t);
        fflush(stdout);

        sleep(1);
    }

    printf("\nNo opponent found.\n");
    printf("Entering BOT battle...\n");

    sleep(1);

    battle_arena("Wild Beast");
}

void armory_menu()
{
    int shmid;
    SharedData *data;

    shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
    data = (SharedData *)shmat(shmid, NULL, 0);

    int idx = -1;

    for (int i = 0; i < data->total_user; i++)
    {
        if (strcmp(data->users[i].username, current_user) == 0)
        {
            idx = i;
            break;
        }
    }

    if (idx == -1)
    {
        shmdt(data);
        return;
    }

    while (1)
    {
        int c;

        printf("\n====== ARMORY ======\n");
        printf("Gold: %d\n", data->users[idx].gold);
        printf("Current Bonus Damage: +%d\n\n",
               data->users[idx].weapon_bonus);

        printf("1. Wood Sword   (100 G)  +5\n");
        printf("2. Iron Sword   (300 G)  +15\n");
        printf("3. Steel Axe    (600 G)  +30\n");
        printf("4. Demon Blade  (1500 G) +60\n");
        printf("5. God Slayer   (5000 G) +150\n");
        printf("0. Back\n");
        printf("Choice: ");

        scanf("%d", &c);

        int price = 0;
        int bonus = 0;

        if (c == 1) { price = 100; bonus = 5; }
        else if (c == 2) { price = 300; bonus = 15; }
        else if (c == 3) { price = 600; bonus = 30; }
        else if (c == 4) { price = 1500; bonus = 60; }
        else if (c == 5) { price = 5000; bonus = 150; }
        else if (c == 0) break;
        else continue;

        if (data->users[idx].gold < price)
        {
            printf("Not enough gold!\n");
            continue;
        }

        data->users[idx].gold -= price;

        if (bonus > data->users[idx].weapon_bonus)
            data->users[idx].weapon_bonus = bonus;

        printf("Weapon purchased!\n");
    }

    shmdt(data);
}

void history_menu()
{
    FILE *fp;
    char filename[100];
    char line[256];

    sprintf(filename, "%s_history.txt",
            current_user);

    fp = fopen(filename, "r");

    printf("\n====== MATCH HISTORY ======\n");

    if (fp == NULL)
    {
        printf("No history yet.\n");
        return;
    }

    while (fgets(line, sizeof(line), fp))
        printf("%s", line);

    fclose(fp);

    printf("\nPress ENTER...");
    getchar();
    getchar();
}

void profile_menu(int msgid)
{
    while (1)
    {
        Message msg;

        msg.type = 1;
        msg.action = 3;

        strcpy(msg.username, current_user);

        msgsnd(msgid, &msg,
               sizeof(Message)-sizeof(long), 0);

        msgrcv(msgid, &msg,
               sizeof(Message)-sizeof(long),
               13, 0);

        char name[50];
        int lvl, gold, xp;

        sscanf(msg.text,
               "%49[^|]|%d|%d|%d",
               name, &lvl, &gold, &xp);

        printf("\n=============================\n");
        printf("     BATTLE OF ETERION\n");
        printf("=============================\n");

        printf("Name : %s\n", name);
        printf("Lvl  : %d\n", lvl);
        printf("Gold : %d\n", gold);
        printf("XP   : %d\n", xp);

        printf("\n1. Battle\n");
        printf("2. Armory\n");
        printf("3. History\n");
        printf("4. Logout\n");
        printf("Choice: ");

        int c;
        scanf("%d", &c);

        if (c == 1)
            battle_menu(msgid);
        
        else if (c == 2)
            armory_menu();
        
        else if (c == 3)
            history_menu();
        
        else if (c == 4)
        {
            msg.type = 1;
            msg.action = 4;

            strcpy(msg.username, current_user);

            msgsnd(msgid, &msg,
                   sizeof(Message)-sizeof(long), 0);

            msgrcv(msgid, &msg,
                   sizeof(Message)-sizeof(long),
                   14, 0);

            break;
        }
    }
}

void register_menu(int msgid)
{
    Message msg;

    msg.type = 1;
    msg.action = 1;

    printf("Username: ");
    scanf("%49s", msg.username);

    printf("Password: ");
    scanf("%49s", msg.password);

    msgsnd(msgid, &msg,
           sizeof(Message)-sizeof(long), 0);

    msgrcv(msgid, &msg,
           sizeof(Message)-sizeof(long),
           11, 0);

    printf("%s\n\n", msg.text);
}

void login_menu(int msgid)
{
    Message msg;
    char uname[50];

    msg.type = 1;
    msg.action = 2;

    printf("Username: ");
    scanf("%49s", uname);

    printf("Password: ");
    scanf("%49s", msg.password);

    strcpy(msg.username, uname);

    msgsnd(msgid, &msg,
           sizeof(Message)-sizeof(long), 0);

    msgrcv(msgid, &msg,
           sizeof(Message)-sizeof(long),
           12, 0);

    printf("%s\n\n", msg.text);

    if (strstr(msg.text,
        "Login success") != NULL)
    {
        strcpy(current_user, uname);
        profile_menu(msgid);
    }
}

int main()
{
    int msgid, shmid;
    int choice;

    msgid = msgget(MSG_KEY, 0666);
    shmid = shmget(SHM_KEY,
                   sizeof(SharedData), 0666);

    if (msgid == -1 || shmid == -1)
    {
        printf("Orion are you there?\n");
        return 0;
    }

    while (1)
    {
        printf("=================================\n");
        printf("      BATTLE OF ETERION\n");
        printf("=================================\n");

        printf("1. Register\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choice: ");

        scanf("%d", &choice);

        if (choice == 1)
            register_menu(msgid);
        else if (choice == 2)
            login_menu(msgid);
        else
            break;
    }

    return 0;
}
