 /* servTcpConc.c - Exemplu de server TCP concurent
    Asteapta un nume de la clienti; intoarce clientului sirul
    "Hello nume".
    */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

#include "server_config.h"
int sd;
struct game* games;
struct user* users;
int* users_count;

/* portul folosit */
#define PORT 2024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

void InitGame(){
	int i, j, k;
	for(i = 0; i < 100; ++i){
		games[i].id1 = 0;
		games[i].id2 = 0;
		games[i].turn = 0;
		games[i].disconnected = 0;
		
		for(j = 0; j < 6; ++j)
			for(k = 0; k < 7; ++k)
				games[i].field[j][k] = 0;
	}
	
	int id;
	char userBuffer[64];
	char passwordBuffer[64];
	int last_login, score, level, exp, admin, banned;
	
	FILE* IN = NULL;
	IN = fopen("users.txt", "r+");
	if(NULL == IN)
		return;
	
	*users_count = 0;
	while(fscanf(IN, "%d%s%s%d%d%d%d%d%d", &id, userBuffer, passwordBuffer, &last_login, &score, &level, &exp, &admin, &banned) != EOF && *users_count < 100){
		users[*users_count].id = id;
		strcpy(users[*users_count].user, userBuffer);
		strcpy(users[*users_count].password, passwordBuffer);
		users[*users_count].last_login = last_login;
		users[*users_count].score = score;
		users[*users_count].level = level;
		users[*users_count].exp = exp;
		users[*users_count].admin = admin;
		users[*users_count].banned = banned;
		users[*users_count].online = 0;
		++(*users_count);
	}
	
	fclose(IN);
	
}

void SaveUsersData() {
	int i;
	FILE* IN = NULL;
	IN = fopen("users.txt", "w+");
	if(NULL == IN)
		return;
	for(i = 0; i < *users_count; ++i) {
		if(users[i].id != 0) {
			fprintf(IN, "%d %s\t%s\t%d\t%d\t%d\t%d\t%d\t%d\n",
			users[i].id, users[i].user, users[i].password, users[i].last_login, users[i].score, users[i].level, users[i].exp, users[i].admin, users[i].banned);
		}
	
	}
	
	fclose(IN);
}


void EndGame(int gameId) {
	games[gameId].id1 = 0;
	games[gameId].id2 = 0;
	games[gameId].turn = 0;
	games[gameId].disconnected = 0;
	
	int i, j;
	for(i = 0; i < 6; ++i)
		for(j = 0; j < 7; ++j)
			games[gameId].field[i][j] = 0;
		
	printf("Am resetat camera %d!\n", gameId);
}


int VerifyWinner(int id) {
	
	int i, j, contor;
	
	
	for(i = 0; i < 3; ++i){ // sub-diagonala si diagonala principala
		contor = 1;
		for(j = 1; j < 6 - i; ++j) {
			if(games[id].field[i + j][j] == games[id].field[i + j - 1][j - 1] && games[id].field[i + j][j] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[i + j][j]; // Returnam id-ul castigatorului
			}
		}
	}
	
	for(i = 1; i < 4; ++i){ // deasupra diagonalei principale
		contor = 1;
		for(j = 1; j < 7 - i; ++j) {
			if(games[id].field[j][i + j] == games[id].field[j - 1][i + j - 1] && games[id].field[j][i + j] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[j][i + j]; // Returnam id-ul castigatorului
			}
		}
	}
	
	
	for(i = 3; i < 6; ++i){ //deasupra diagonalei secundare
		contor = 1;
		for(j = 1; j <= i; ++j) {
			if(games[id].field[i - j][j] == games[id].field[i - j + 1][j - 1] && games[id].field[i - j][j] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[i - j][j]; // Returnam id-ul castigatorului
			}
		}
	}
	
	for(i = 0; i < 3; ++i){ // dedesubtul diagonalei principale
		contor = 1;
		for(j = i + 1; j < 6; ++j) {
			if(games[id].field[j][6 - j + i] == games[id].field[j - 1][6 - j + i + 1] && games[id].field[j][6 - j + i] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[j][6 - j + i]; // Returnam id-ul castigatorului
			}
		}
	}
	
	
	for(i = 0; i < 6; ++i) { // pe linii
		contor = 1;
		for(j = 1; j < 7; ++j){
			if(games[id].field[i][j - 1] == games[id].field[i][j] && games[id].field[i][j] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[i][j]; // Returnam id-ul castigatorului
			}
		}
	}
	
	for(i = 0; i < 7; ++i) { // pe coloane
		contor = 1;
		for(j = 1; j < 6; ++j){
			if(games[id].field[j][i] == games[id].field[j - 1][i] && games[id].field[j][i] != 0)
				++contor;
			else
				contor = 1;
			
			if(contor == 4){
				return games[id].field[j][i]; // Returnam id-ul castigatorului
			}
		}
	}
	
	contor = 0;
	for(i = 0; i < 7; ++i) {
		for(j = 0; j < 6; ++j){
			if(games[id].field[i][j])
				++contor;
		}
	}
	if(42 == contor)
		return -1; // Remiza
	
	return 0; // Jocul continua
}


struct user_info GetUserInfo(int id) {
	struct user_info userInfo;
	
	for(int i = 0; i < *users_count; ++i) {
		if(users[i].id == id){
			userInfo.id = users[i].id;
			strcpy(userInfo.user, users[i].user);
			userInfo.last_login = users[i].last_login;
			userInfo.score = users[i].score;
			userInfo.level = users[i].level;
			userInfo.exp = users[i].exp;
			break;
		}
	}
	
	return userInfo;
}

struct user_info GetUserInfoByName(char name[]) {
	struct user_info userInfo;
	
	for(int i = 0; i < *users_count; ++i) {
		if(strcmp(users[i].user, name) == 0){
			userInfo.id = users[i].id;
			strcpy(userInfo.user, users[i].user);
			userInfo.last_login = users[i].last_login;
			userInfo.score = users[i].score;
			userInfo.level = users[i].level;
			userInfo.exp = users[i].exp;
			break;
		}
	}
	
	return userInfo;
}


int JoinGame(int id) {
	int i = 0;
	
	for(i = 0; i < 100; ++i){ // Cautam o camera in asteptare (doar cu 1 jucator)
		if(games[i].id1 == 0 && games[i].id2 != 0 && games[i].disconnected == 0){
			games[i].id1 = id;
			games[i].userInfo1 = GetUserInfo(id);
			games[i].turn = games[i].id1;
			return i;
		}else if(games[i].id1 != 0 && games[i].id2 == 0 && games[i].disconnected == 0){
			games[i].id2 = id;
			games[i].userInfo2 = GetUserInfo(id);
			games[i].turn = games[i].id1;
			printf("[server] Am gasit un coechipier la %d.\n", i);
			return i;
		}
	}
	
	for(i = 0; i < 100; ++i){  // Cautam o camera de meci goala
		if(games[i].id1 == 0 && games[i].id2 == 0){
			games[i].id1 = id;
			games[i].userInfo1 = GetUserInfo(id);
			printf("[server] Am gasit o camera goala la la %d.\n", i);
			return i;
		}
	}
	
	return -1;
}
int MakeMove(int gameId, int userId, int position) {
	int i, j, contor = 0;
	for(i = 0; i < 7; ++i) {
		for(j = 0; j < 6; ++j){
			if(games[gameId].field[i][j])
				++contor;
		}
	}
	if(42 == contor)
		return 0; // Remiza*/
	
	if(position < 0 || position > 6 || games[gameId].field[0][position] != 0)
		return 0;
	
	if(games[gameId].field[5][position] == 0) {
		games[gameId].field[5][position] = userId;
		return 1;
	}
	
	for(i = 0; i < 5; ++i){
		if(games[gameId].field[i + 1][position]){
			break;
		}
	}
	
	games[gameId].field[i][position] = userId;
	return 1;
}

int CheckUserStatus(int userId) {
	int i;
	for(i = 0; i < *users_count; ++i) {
		if(users[i].id == userId){
			return users[i].online;
		}
	}
	
	return 0;
}

int userLogin(char user[], char password[], int* isAdmin) {
	printf("Compar: %s %s\n", user, password);
	int i;
	for(i = 0; i < *users_count; ++i)
		if(!strcmp(users[i].user, user) && !strcmp(users[i].password, password)) {
			if(users[i].banned)
				return 0;
			*isAdmin = users[i].admin;
			users[i].online = 1; // Marchez utilizatorul ca fiind online
			return users[i].id;
		}
	return 0;
}


int userRegister(char user[], char password[]) {
	printf("Inregistrez: %s %s\n", user, password);
	int id;
	int last_login, score, level, exp;
	
	int i;
	for(i = 0; i < *users_count; ++i){
		if(!strcmp(users[i].user, user)){
			return 0;
		}
	}
	
	FILE * IN = NULL;
	IN = fopen("users.txt", "a");
	if(NULL == IN)
		return 0;
	
	fprintf(IN, "\n%d %s\t%s\t%d\t%d\t%d\t%d\t%d", ++(*users_count), user, password, 0, 0, 0, 0, 0);
	
	fclose(IN);
	
	users[*users_count - 1].id = *users_count;
	strcpy(users[*users_count - 1].user, user);
	strcpy(users[*users_count - 1].password, password);
	users[*users_count - 1].last_login = 0;
	users[*users_count - 1].score = 0;
	users[*users_count - 1].level = 0;
	users[*users_count - 1].exp = 0;
	users[*users_count - 1].admin = 0;
	
	for(i = 0; i < *users_count; ++i){
		printf("%d %s\t%s\t%d\t%d\t%d\t%d\t%d\n",
		users[i].id,
		users[i].user,
		users[i].password,
		users[i].last_login,
		users[i].score,
		users[i].level,
		users[i].exp,
		users[i].admin);
	}
	
	return 1;
}

int DeleteUserByName(char name[]) {
	int i;
	for(i = 0; i < *users_count; ++i) {
		if(strcmp(users[i].user, name) == 0){
			if(users[i].online == 1)
				return -1;
			users[i].id = 0;
			users[i].user[0] = '\0';
			users[i].password[0] = '\0';
			users[i].last_login = 0;
			users[i].score = 0;
			users[i].level = 0;
			users[i].exp = 0;
			users[i].admin = 0;
			
			SaveUsersData();
			
			printf("Am sters utilizatorul\n");
			return 1;
		}
	}
	return 0;
}

int BanUserByName(char name[]) {
	int i;
	for(i = 0; i < *users_count; ++i) {
		if(strcmp(users[i].user, name) == 0){
			users[i].banned = 1;
			printf("Am banat utilizatorul %s.\n", name);
			
			SaveUsersData();
			return 1;
		}
	}
	
	return 0;
}

void MarkOfflineUser(int userId) {
	int i;
	for(i = 0; i < *users_count; ++i) {
		if(users[i].id == userId){
			users[i].online = 0;
			
			printf("Marchez utilizatorul ca fiind deconectat\n");
			return;
		}
	}
}

void MarkOfflineUserByName(char name[]) {
	int i;
	for(i = 0; i < *users_count; ++i) {
		if(strcmp(users[i].user, name) == 0){
			users[i].online = 0;
			
			printf("Marchez utilizatorul ca fiind deconectat\n");
			return;
		}
	}
}

void sig_handler(int sig) {
    switch (sig) {
    case SIGINT:
        fprintf(stderr, "Semnal de inchidere...\n");
		close(sd);
        abort();
    default:
        fprintf(stderr, "wasn't expecting that!\n");
        abort();
    }
}

int main ()
{
	printf("%ld\n", sizeof(struct user));
	games = mmap(NULL, 100*sizeof(struct game), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	users = mmap(NULL, 100*sizeof(struct user), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	users_count = mmap(NULL, sizeof(int), PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_SHARED, -1, 0);
	
	if(games == MAP_FAILED){
		perror ("[server]Eroare la initializare camere de joc.\n");
    	return errno;
	}
	
	if(users == MAP_FAILED){
		perror ("[server]Eroare la alocare memorie baza de date.\n");
    	return errno;
	}
	
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    //int sd;			//descriptorul de socket - facut global

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);
	//setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
    /* atasam socketul */
	
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));
	
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }
	
	signal(SIGINT, sig_handler);

    /* servim in mod concurent clientii... */
	
	InitGame();
    while (1)
    {
    	int client;
    	int length = sizeof (from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} else if (pid > 0) {
    		// parinte
    		close(client);
    		while(waitpid(-1, NULL, WNOHANG));
    		continue;
    	} else if (pid == 0) {
    		// copil
    		close(sd);
			
    		/* s-a realizat conexiunea, se astepta mesajul */
    		printf ("[server]Asteptam mesajul...\n");
    		fflush (stdout);
			
			struct command cmd;
			//send(client, &cmd, sizeof(cmd), 0)
			
			
			
			cmd.index = 0;
			strcpy(cmd.buffer, "");
			
			if (send (client, &cmd, sizeof(cmd), 0) <= 0)
			{
				perror ("[server]Eroare la write() catre client.\n");
				return errno;
			}
			
			int loggedIn = 0;
			int userId = 0;
			int isAdmin = 0;
			int gameId = -1;
			
			while(1) {
				//printf("Conexiune cu clientul %d\n", userId);
				WHILE_START:
				
				if (recv (client, &cmd, sizeof(cmd), 0) <= 0)
				{
					perror ("[server] Client deconectaaat\n");
					goto CLIENT_END;
				}
				
				
				if(userId && !CheckUserStatus(userId)) {
					cmd.index = SC_DISCONNECT;
					
					SEND_PACKET(client, cmd);
					goto CLIENT_END;
				}
				
				switch(cmd.index) {
					/* ----- CONECTARE ---- */
					case CS_LOGIN:;
						struct account_packet account;
						if (recv (client, &account, sizeof(account), 0) < 0)
						{
							
							perror ("[server] Client deconectat\n");
							goto CLIENT_END;
						}
						
						if(userId = userLogin(account.user, account.password, &isAdmin)){

							if(isAdmin)
								printf("Este administrator!\n");
							loggedIn = 1; // Conectam utilizatorul
							printf("S-a conectat utilizatorul cu id-ul %d\n", userId);
							if(isAdmin)
								cmd.index = SC_ADMIN_MENU;
							else
								cmd.index = SC_USER_MENU;
							strcpy(cmd.buffer, "\033[0;33mConectarea a fost reazlizata cu succes\033[0;37m\n");
							
						} else {
							cmd.index = SC_LOGIN_MENU;
							strcpy(cmd.buffer, "\033[0;31mUtilizatorul nu a fost gasit in baza de date sau este blocat.\033[0;37m\n");
						}
						
						SEND_PACKET(client, cmd);
						break;
						
						
						
					/* ----- INREGISTRARE ---- */
					case CS_REGISTER:;
						struct account_packet acc;
						if (recv (client, &acc, sizeof(acc), 0) < 0)
						{
							perror ("[server] Client deconectat\n");
							goto CLIENT_END;
						}
						
						if(isAdmin)
							cmd.index = SC_ADMIN_MENU;
						else
							cmd.index = SC_LOGIN_MENU;
						
						if(userRegister(acc.user, acc.password)){
							strcpy(cmd.buffer, "\033[0;33mUtilizator inregistrat cu succes!\033[0;37m\n");
							
						} else {
							strcpy(cmd.buffer, "\033[0;31mA aparut o eroare in timpul inregistrarii\033[0;37m\n");
						}
						
						SEND_PACKET(client, cmd);
						
						break;
						
					case CS_GET_USER_INFO:;
						struct user_info userInfo;
						userInfo = GetUserInfo(userId);
						SEND_PACKET(client, userInfo);
						break;
						
					case CS_ADMIN_GET_USER_INFO:;
						struct user_info userInfo2;
						userInfo2 = GetUserInfoByName(cmd.buffer);
						SEND_PACKET(client, userInfo2);
						break;
						
					case CS_ADMIN_DELETE_USER:;
						int response = DeleteUserByName(cmd.buffer);
						if(response == -1) {
							strcpy(cmd.buffer, "Nu poti sterge un utilizator conectat la server\n");
						}
						else if(response == 0) {
							strcpy(cmd.buffer, "Utilizatorul nu a fost gasit in baza de date\n");
						}
						else {
							strcpy(cmd.buffer, "Utilizatorul a fost sters cu succes\n");
						}
						cmd.index = SC_ADMIN_MENU;
						SEND_PACKET(client, cmd);
						break;
						
					case CS_ADMIN_BAN_USER:
						if(BanUserByName(cmd.buffer)) {
							strcpy(cmd.buffer, "Utilizatorul a fost blocat cu succes\n");
						}
						else {
							strcpy(cmd.buffer, "Utilizatorul nu exista in baza de date\n");
						}
						
						cmd.index = SC_ADMIN_MENU;
						SEND_PACKET(client, cmd);
						break;
						
					case CS_ADMIN_DISCONNECT_USER:
						MarkOfflineUserByName(cmd.buffer);
						cmd.index = SC_ADMIN_MENU;
						strcpy(cmd.buffer, "Am deconectat utlizatorul\n");
						SEND_PACKET(client, cmd);
						break;
						
					/* ----- START JOC ---- */
					case CS_PLAY:
						
						//printf("Un jucator vrea sa joace(%d)!\n", userId);
						if(loggedIn){
							gameId = JoinGame(userId);
							//printf("gameId = %d\n", gameId);
							if(gameId == -1){ // Verificare daca serverul este plin
								if(isAdmin)
									cmd.index = SC_ADMIN_MENU;
								else
									cmd.index = SC_USER_MENU;
								strcpy(cmd.buffer, "Serverul este FULL\n");
								
								SEND_PACKET(client, cmd);
							}
							//printf("Info: %d %d\n", games[gameId].id1,games[gameId].id2);
							
							else if(games[gameId].id1 && games[gameId].id2) { // Verificare daca exista adversar
								cmd.index = SC_START_GAME;
								strcpy(cmd.buffer, "");
								
								SEND_PACKET(client, cmd);
								
								struct player_packet playerPacket;
								playerPacket.id = userId;
								playerPacket.winner = 0;
								
								playerPacket.match = games[gameId];
								
								SEND_PACKET(client, playerPacket);
							} else{ // Daca nu exista adversar, astept
								while(1){ 
									if(!CheckUserStatus(userId)) {
										printf("Am detectat kick-ul!\n");
										cmd.index = SC_DISCONNECT;
										SEND_PACKET(client, cmd);
										goto CLIENT_END;
									}
									
									/* VERIFICARE CONEXIUNE */
									cmd.index = SC_CHECK_CONNECTION;
									SEND_PACKET(client, cmd);
									if (recv (client, &cmd, sizeof(cmd), 0) <= 0)
									{
										perror ("[server] Client deconectaaat\n");
										goto CLIENT_END;
									}
									/* SFARSIT VERIFICARE CONEXIUNE */
									
									
									
									if(games[gameId].id1 && games[gameId].id2){
										cmd.index = SC_START_GAME;
										strcpy(cmd.buffer, "");
										
										SEND_PACKET(client, cmd);
										
										struct player_packet playerPacket;
										playerPacket.id = userId;
										playerPacket.winner = 0;
										playerPacket.match = games[gameId];
										
										SEND_PACKET(client, playerPacket);
								
										break;
									}
									sleep(0.1);
								}
								
							}
							
							if(games[gameId].turn != userId) {
								while(1) { // Astept randul meu
								
									if(!CheckUserStatus(userId)) {
										cmd.index = SC_DISCONNECT;
										SEND_PACKET(client, cmd);
										goto CLIENT_END;
									}
									
									/* VERIFICARE CONEXIUNE */
									cmd.index = SC_CHECK_CONNECTION;
									SEND_PACKET(client, cmd);
									if (recv (client, &cmd, sizeof(cmd), 0) <= 0)
									{
										perror ("[server] Client deconectaaat\n");
										goto CLIENT_END;
									}
									/* SFARSIT VERIFICARE CONEXIUNE */
									
									if(games[gameId].disconnected){
										EndGame(gameId);
										gameId = -1;
										if(isAdmin)
											cmd.index = SC_ADMIN_MENU;
										else
											cmd.index = SC_USER_MENU;
										strcpy(cmd.buffer, "Oponentul tau s-a deconectat, deci ai castigat meciul!\n");
										printf("Am detectat deconectarea!\n");
										SEND_PACKET(client, cmd);
										goto WHILE_START;
									}
									else if(games[gameId].turn == userId) {
										struct player_packet playerPacket;
										playerPacket.id = userId;
										playerPacket.winner = 0;
									
										playerPacket.match = games[gameId];
										cmd.index = SC_START_GAME;
										strcpy(cmd.buffer, "\033[0;31mAcum este randul tau!\033[0;37m\n");
										
										SEND_PACKET(client, cmd);
										SEND_PACKET(client, playerPacket);
										
										break;
									}
									sleep(0.1);
								}
							}
							
						}
						break;
						
					case CS_MOVE:;
						// Daca face o miscare corecta, o pun si schimb tura apoi intru in while
						// Daca face o miscare gresita, reiau 
						// Cand incepe meciul trimit o structura cu
						// detalii despre adversar, daca este tura ta, culoarea pionului,
						// si despre cum arata harta
						if(loggedIn && gameId != -1) {
							int wait = 0;
							struct player_packet playerPacket;
							playerPacket.winner = 0;
							
							
							/* VERIFICARE DECONECTARE OPONENT */
							if(games[gameId].disconnected){
								EndGame(gameId);
								gameId = -1;
								if(isAdmin)
									cmd.index = SC_ADMIN_MENU;
								else
									cmd.index = SC_USER_MENU;
								strcpy(cmd.buffer, "\033[0;33mOponentul tau s-a deconectat, deci ai castigat meciul!\033[0;37m\n");
								printf("Am detectat deconectarea!\n");
								SEND_PACKET(client, cmd);
								goto WHILE_START;
							}
							/* FINAL VERIFICARE DECONECTARE OPONENT */
							
							if(games[gameId].turn == userId) {
								int pos = (int)cmd.buffer[0];
								
								//Verific  daca coordonatele primite in varaibila pos sunt corecte
								if(MakeMove(gameId, userId, pos - 1)){
									
									printf("Miscare corecta!\n");
									//games[gameId].field[pos/10 - 1][pos%10 - 1] = userId; // Marchez miscarea pe tabla
									
									playerPacket.winner = VerifyWinner(gameId); // Verific statusul curent al jocului 
									
									if(games[gameId].id1 == userId) // Acum este randul oponentului
										games[gameId].turn = games[gameId].id2;
									else
										games[gameId].turn = games[gameId].id1;
									
									// Verific statusul jocului
									if(playerPacket.winner == -1){ // Remiza
										cmd.index = SC_START_GAME;
										strcpy(cmd.buffer, "\033[0;33mRemiza!\033[0;37m\n");
									}
									else if (playerPacket.winner > 0){ // A castigat unul dintre concurenti
										if(isAdmin)
											cmd.index = SC_ADMIN_MENU;
										else
											cmd.index = SC_START_GAME; // TODO
										strcpy(cmd.buffer, "\033[0;33mJocul s-a terminat! Ai castigat!\033[0;37m\n");
										
									}
									else{// Jocul continua
										cmd.index = SC_START_GAME; 
										strcpy(cmd.buffer, "\033[0;31mAcum e randul oponentului!\033[0;37m\n");
										wait = 1;
									}
								}
								else{
									cmd.index = SC_START_GAME;
									strcpy(cmd.buffer, "\033[0;31mAlegeti alte coordonate!\033[0;37m\n");
								}
							}
							else {
								cmd.index = SC_START_GAME;
								strcpy(cmd.buffer, "\033[0;31mNu este randul tau!\033[0;37m\n");
							}
							
							SEND_PACKET(client, cmd);
							
							if(cmd.index == SC_START_GAME) {// doar in SC_START_GAME
								playerPacket.id = userId;
								playerPacket.match = games[gameId];
								
								SEND_PACKET(client, playerPacket);
							}
							
							if(playerPacket.winner > 0) // Resetez id-ul
								gameId = -1;
							
							if(wait){
								while(1){
									/* VERIFICARE CONEXIUNE */
									cmd.index = SC_CHECK_CONNECTION;
									SEND_PACKET(client, cmd);
									if (recv (client, &cmd, sizeof(cmd), 0) <= 0)
									{
										perror ("[server] Client deconectaaat\n");
										goto CLIENT_END;
									}
									/* SFARSIT VERIFICARE CONEXIUNE */
									
									int endMatch = 0;
									if(games[gameId].disconnected){
										EndGame(gameId);
										gameId = -1;
										if(isAdmin)
											cmd.index = SC_ADMIN_MENU;
										else
											cmd.index = SC_USER_MENU;
										strcpy(cmd.buffer, "Oponentul tau s-a deconectat, deci ai castigat meciul!\n");
										printf("Am detectat deconectarea!\n");
										SEND_PACKET(client, cmd);
										goto WHILE_START;
									}
									else if(games[gameId].turn == userId){
										playerPacket.match = games[gameId];
										playerPacket.id = userId;
										
										playerPacket.winner = VerifyWinner(gameId);
										if(playerPacket.winner == -1){
											cmd.index = SC_START_GAME;
											strcpy(cmd.buffer, "\033[0;31mRemiza!\033[0;37m\n");
										}
										else if (playerPacket.winner > 0){
											if(isAdmin)
												cmd.index = SC_ADMIN_MENU;
											else
												cmd.index = SC_START_GAME;
											strcpy(cmd.buffer, "\033[0;31mJocul s-a terminat. Ai pierdut!\033[0;37m\n");
											endMatch = 1;
										}
										else{
											cmd.index = SC_START_GAME;
											strcpy(cmd.buffer, "\033[0;31mAcum e randul tau!\033[0;37m\n");
										}
										
										SEND_PACKET(client, cmd);
										
										SEND_PACKET(client, playerPacket);
										
										if(endMatch){
											EndGame(gameId);
											gameId = -1;
										}
										
										break;
									}
										
									sleep(0.1);

									
								}
								
							}
							
						}
						
						break;
						
					/* ----- DECONECTARE JUCATOR DE LA CONT ---- */
					case CS_DISCONNECT:
						loggedIn = 0;
						isAdmin = 0;
						cmd.index = SC_LOGIN_MENU;
						strcpy(cmd.buffer, "\033[0;33mAi fost deconectat din cont!\033[0;37m\n");
						
						SEND_PACKET(client, cmd);
						
						break;
					default:
						/* afisam mesajul primit */
						printf ("[server]Mesajul primit este: %s\n", cmd.buffer);
						break;
				}
				
			
			}
    		
    		/* am terminat cu acest client, inchidem conexiunea */
			
			CLIENT_END:
			if(userId)
				MarkOfflineUser(userId);
			
			if(gameId != -1){
				printf("Utilizatorul cu id-ul %d, aflat in meci, s-a deconectat.\n", userId);
				if((games[gameId].id1 == 0 && games[gameId].id2 != 0) || (games[gameId].id1 != 0 && games[gameId].id2 == 0) || games[gameId].disconnected)
					EndGame(gameId);
				else
					games[gameId].disconnected = userId;
			}
    		close (client);
    		exit(0);
    	}

    }				/* while */
}				/* main */