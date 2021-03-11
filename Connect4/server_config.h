#ifndef _SERVER_HEADER_
#define _SERVER_HEADER_

#define MAX_CLIENTS 10
#define clear() printf("\033[H\033[J")

#define SEND_PACKET(client, packet) {\
    if (send (client, &(packet), sizeof((packet)), 0) <= 0)\
	{\
		perror ("[server]Client deconectat\n");\
		goto CLIENT_END;\
	}\
}
//int login()

// CS = CLIENT-SERVER PACKET ID
enum {
	CS_REGISTER, // 0
	CS_LOGIN,
	CS_PLAY,
	CS_DISCONNECT,
	CS_MOVE,
	CS_GET_USER_INFO,
	CS_ADMIN_DELETE_USER,
	CS_ADMIN_BAN_USER,
	CS_ADMIN_DISCONNECT_USER,
	CS_ADMIN_GET_USER_INFO
};

// SC = SERVER-CLIENT PACKET ID
enum{
	SC_LOGIN_MENU, //afisare meniu conectare
	SC_LOGIN_ERROR,
	SC_USER_MENU,
	SC_ADMIN_MENU,
	SC_START_GAME,
	SC_CHECK_CONNECTION,
	SC_DISCONNECT
};

struct command{
	int index;
	char buffer[1024];
};

struct account_packet{
	char user[64];
	char password[64];
};

struct user_info{
	int id;
	char user[64];
	int last_login, score, level, exp;
};

struct game{
	int id1;
	int id2;
	struct user_info userInfo1, userInfo2;
	int turn;
	int disconnected;
	int field[6][7];
};

struct player_packet{
	int id;
	int winner;
	char name[64];
	struct game match;
};

struct user {
	int id;
	char user[64];
	char password[64];
	int last_login, score, level, exp, admin, online, banned;
};


#endif