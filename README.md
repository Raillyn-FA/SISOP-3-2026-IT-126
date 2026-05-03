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
Sumber: [navi.c](https://github.com/Raillyn-FA/SISOP-3-2026-IT-126/blob/main/soal_1/wired.c).
4. 
