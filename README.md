# SISOP-3-2026-IT-126

| Modul 2 |   Identitas Praktikan  |
|---------|------------------------|
| Nama    | Rayhan Fadhilah Allayn |
| NRP     | 5027251126             |
| Kelas   | Sistem Operasi B       |
| Asisten | MOO                    |

## Struktur Repository
```
├── soal_1
│   ├── navi.c
│   ├── protocol.h
│   └── wired.c
└── soal_2
    ├── Makefile
    ├── arena.h
    ├── eternal.c
    └── orion.c
```
## Soal 1 - Present Day, Present Time
Dalam Soal ini, dibuat sebuah sistem komunikasi berbasis client-server bernama The Wired, yang memungkinkan banyak client (NAVI) terhubung ke satu server pusat dan saling berkomunikasi secara real-time.

Sistem ini mengimplementasikan:
- Multi-client handling (thread).
- Broadcast message.
- Logging aktivitas.
- Autentikasi admin.
- Remote procedure call (RPC sederhana).

### Penjelasan Program
1. Koneksi Client ke Server.
* Membuat socket.
* Connect ke server (127.0.0.1:8080).
* Mengirim nama user.

Implementasi:
```C
sock = socket(AF_INET, SOCK_STREAM, 0);
connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
send(sock, name, strlen(name), 0);
```
Sumber: [navi.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/navi.c).

2. Asynchronous Communication (Thread)
Client menggunakan thread untuk:
* Menerima pesan dari server.
* Mengirim input user secara bersamaan.
```C
pthread_create(&tid, NULL, receive_msg, NULL);
```
Sumber: [navi.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/navi.c).

3. Multi Client Handling (Server)
* `pthread` untuk setiap client.
* Array `clients[]` untuk menyimpan user aktif.
```C
pthread_create(&tid, NULL, handle_client, pclient);
```
Sumber: [wired.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/wired.c).

4. Validasi Username (Unique Identity)
Server memastikan:
* Tidak ada nama yang sama.
```C
if (username_exist(name))
```
Jika duplicate:
```[System] The identity 'alice' is already synchronized.```

5. Broadcast Message
Setiap pesan dari client:
* Dikirim ke semua client lain.
```C
broadcast(msg, sock);
```
Sumber: [wired.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/wired.c).

6. Admin System (The Knights)
Admin login dengan:
* Username: `The Knights`
* Password: `protocol7`

Fitur Admin:
* Cek jumlah user aktif
* Cek uptime server
* Shutdown server
```C
if (strcmp(name, ADMIN_NAME) == 0)
```
Sumber: [protocol.h](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/protocol.h).

7. Logging System
Semua aktivitas disimpan ke:
```
history.log
```
Format:
`
[YYYY-MM-DD HH:MM:SS] [Role] [Message]
`
Implementasi:
```C
fprintf(fp, "[%04d-%02d-%02d %02d:%02d:%02d] [%s] [%s]\n", ...)
```

8. Server Uptime:
Menggunakan:
```C
time_t start_time;
difftime(now, start_time);
```
Sumber: [wired.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/wired.c).

9. Disconnect System
Client keluar dengan:
```
/exit
```
Server:
* Menghapus client dari list
* Menulis log

### Cara Menjalankan
1. Compile
```Bash
gcc wired.c -o server -lpthread
gcc navi.c -o client -lpthread
```
2. Jalankan Server (Terminal 1)
```Bash
./server
```
3. Jalankan Client (Terminal 2)
```Bash
./client
```
4. Jalankan Client (Terminal 3)
```Bash
./client
```
### Contoh Output
Client:
```
Enter your name: alice
--- Welcome to The Wired, alice ---
```
Chat:
```
[alice]: hello lain
[lain]: hello alice
```
Admin:
```
--- THE KNIGHTS CONSOLE ---
1. Check Active Users
2. Check Server Uptime
3. Shutdown Server
```

## Soal 2 - The Battle of Eterion 
Battle of Eterion merupakan sistem simulasi pertarungan berbasis IPC (Inter Process Communication) di mana terdapat dua komponen utama:
* Orion (Server)
* Eternal (Client)

Komunikasi dilakukan menggunakan:
* Message Queue → komunikasi request-response
* Shared Memory → penyimpanan data global & realtime battle

### Penjelasan Fitur
1. Arena Initialization
Arena dibuat dengan struktur:
```C
#define MSG_KEY 0x00001234
#define SHM_KEY 0x00009012
#define BATTLE_KEY 0x00005678
```
Sumber: [arena.h](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/arena.h).
Digunakan untuk:
* Message Queue
* Shared Memory global
* Shared Memory battle

2. Main Menu (Eternal)
Client menampilkan menu awal:
```C
printf("1. Register\n");
printf("2. Login\n");
printf("3. Exit\n");
```
Sumber: [eternal.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/eternal.c).

3. IPC Communication
Komunikasi menggunakan:
-> Message Queue:
```C
msgsnd(msgid, &msg, ...);
msgrcv(msgid, &msg, ...);
```
Digunakan untuk:
* Register
* Login
* Profile
* Matchmaking

-> Shared Memory
```C
shmget(SHM_KEY, sizeof(SharedData), ...);
```
Digunakan untuk:
* Data user
* Status battle

Sumber: [arena.h](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/arena.h).

4. Register & Login System
Fitur:
* Username unik
* Password validasi
* Tidak bisa login jika sudah online
```C
if (find_user(data, msg.username) != -1)
```
Sumber: [orion.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/orion.c).

5. Default User Data
Saat register:
```C
u.gold = 150;
u.xp = 0;
u.level = 1;
```
Sumber: [orion.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/orion.c).

6. Matchmaking System
* Durasi: 35 detik
* Jika tidak dapat lawan -> bot
```C
for (int t = 35; t >= 1; t--)
```
Sumber: [eternal.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/eternal.c).

7. Realtime Battle System
Battle menggunakan:
* Shared Memory (BattleRoom)
```C
room->hp1 -= mydmg;
room->hp2 -= mydmg;
```
Fitur:
* Attack (`a`)
* Ultimate (`u`)
* Refresh (`r`)

Cooldown:
* Attack -> 1 detik
* Ultimate -> 5 detik

8. Reward System

|    Kondisi    |    XP    |    Gold    |
|---------------|----------|------------|
|    Menang     |    +50   |    +120    |
|    Kalah      |    +15   |    +30     |

```C
data->users[i].xp += 50;
```
Sumber: [eternal.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_2/eternal.c).

9. Armory System
Senjata meningkatkan damage:

|    Senjata          |    Harga    |    Bonus   |
|---------------------|-------------|------------|
|    Wood Sword       |    100      |    +120    |
|    Iron Sword       |    300      |    +30     |
|    Steel Axe        |    600      |    +120    |
|    Demon Blade      |    1500     |    +30     |
|    KaGod Slayerlah  |    5000     |    +30     |
