/* cliTcpConc.c - Exemplu de client TCP
   Trimite un nume la server; primeste de la server "Hello nume".
         
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include "server_config.h"

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char msg[100];		// mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }
	struct command cmd;
	printf("Asteptam rapsuns de la server\n");
	
	int raspuns;
	char field[6][7][25];
	while(1){
		if (recv (sd, &cmd, sizeof(cmd), 0) <= 0)
		{
			perror ("[client]Conexiunea pierduta cu serverul.\n");
			return errno;
		}
		
		switch(cmd.index){
			case SC_LOGIN_MENU:
				clear();
				printf ("%s", cmd.buffer);
				printf("+++++++++++++++++++\n");
				printf("+      MENIU      +\n");
				printf("+                 +\n");
				printf("+ 1.Inregistrare  +\n");
				printf("+ 2.Conectare     +\n");
				printf("+ 3.Iesire        +\n");
				printf("+++++++++++++++++++\n");
				
				do{
					printf("Raspuns = ");
					scanf("%d", &raspuns);
				} while(raspuns < 1 || raspuns > 3);
				
				if(raspuns == 3)
					return 0;
				else if(raspuns == 1){
					cmd.index = CS_REGISTER;
				}
				else if(raspuns == 2){
					cmd.index = CS_LOGIN;
				}
				
				if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("[client]Eroare la send() catre client.\n");
						return errno;
					}
					
					struct account_packet account;
					printf("User = ");
					scanf("%s", account.user);
					printf("Password = ");
					scanf("%s", account.password);
					
					if (send (sd, &account, sizeof(account), 0) <= 0)
					{
						perror ("[client]Eroare la send() catre client.\n");
						return errno;
					}
				
				break;
			case SC_USER_MENU:
				
				MENU_STEP:
				clear();
				printf("%s\n", cmd.buffer);
				printf("++++++++++++++++++++++++++++++++++++++++\n");
				printf("+            MENIU UTILIZATOR          +\n");
				printf("+                                      +\n");
				printf("+ 1.Start joc                          +\n");
				printf("+ 2.Afisare reguli si instructiuni     +\n");
				printf("+ 3.Statisticile mele                  +\n");
				printf("+ 4.Deconectare cont                   +\n");
				printf("++++++++++++++++++++++++++++++++++++++++\n");
				
				do{
					printf("Raspuns = ");
					scanf("%d", &raspuns);
				} while(raspuns < 1 || raspuns > 4);
				
				if(raspuns == 2){
					clear();
					printf("\033[0;33m++++++++++++++++++++++++++++++++++++++++++\n");
					printf("+            REGULI CONNECT 4            +\n");
					printf("+                                        +\n");
					printf("+  Connect 4 este un joc in 2 jucatori,  +\n");
					printf("+  in care fiecare jucator alege unde sa +\n");
					printf("+  pozitioneze niste discuri pe o tabla  +\n");
					printf("+  de 7 coloane pe 6 randuri. Scopul     +\n");
					printf("+  jocului este sa fii primul care       +\n");
					printf("+  formeaza o linie de 4 piese care sa   +\n");
					printf("+  aiba culoarea ta si care sa fie       +\n");
					printf("+  dispuse in linie, pe coloana sau pe   +\n");
					printf("+  verticala.                            +\n");
					printf("+                                        +\n");
					printf("++++++++++++++++++++++++++++++++++++++++++\n\033[0;37m");
					
					char tmp[1];
					printf("Continuati?\n");
					scanf("%s", tmp);
					goto MENU_STEP;
				}
				else if(raspuns == 1){
					cmd.index = CS_PLAY;
					printf("Se asteapta un adversar...\n");
				}
				else if(raspuns == 3) {
					cmd.index = CS_GET_USER_INFO;
					if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("Conexiunea pierduta cu serverul\n");
						return errno;
					}
					
					struct user_info userInfo;
					
					if (recv (sd, &userInfo, sizeof(userInfo), 0) <= 0)
					{
						perror ("[client]Conexiunea pierduta cu serverul.\n");
						return errno;
					}
					
					printf("\033[0;33m++++++++++++++++++++++++++++++++++++++++++\n");
					printf("+        Informatii despre contul tau            \n");
					printf("+                                        \n");
					printf("+  Id = %d  \n", userInfo.id);
					printf("+  Nume utilizator = %s \n", userInfo.user);
					printf("+  Ultima conectare = %d   \n", userInfo.last_login);
					printf("+  Scor = %d     \n", userInfo.score);
					printf("+  Nivel = %d   \n", userInfo.level);
					printf("+  Experienta = %d \n", userInfo.exp);
					printf("++++++++++++++++++++++++++++++++++++++++++\n\033[0;37m");
					
					char tmp[1];
					printf("Continuati?\n");
					scanf("%s", tmp);
					goto MENU_STEP;
				}
				else if(raspuns == 4){
					cmd.index = CS_DISCONNECT;
				}
				
				if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
				{
					perror ("Conexiunea pierduta cu serverul\n");
					return errno;
				}
				
				break;
			case SC_ADMIN_MENU:
				ADMIN_MENU_STEP:
				clear();
				printf("%s\n", cmd.buffer);
					
				printf("++++++++++++++++++++++++++++++++++++++++\n");
				printf("+          MENIU ADMINISTRATOR         +\n");
				printf("+                                      +\n");
				printf("+ 1.Start joc                          +\n");
				printf("+ 2.Statisticile mele                  +\n");
				printf("+ 3.Lista utilizatori conectati        +\n");
				printf("+ 4.Inregistrare utilizator            +\n");
				printf("+ 5.Afisare informatii utilizator      +\n");
				printf("+ 6.Stergere utilizator                +\n");
				printf("+ 7.Blocare utilizator                 +\n");
				printf("+ 8.Deconectare utilizator             +\n");
				printf("+ 9.Deconectare cont                   +\n");
				printf("++++++++++++++++++++++++++++++++++++++++\n");
				
				do {
					printf("Raspuns = ");
					scanf("%d", &raspuns);
				} while(raspuns < 1 || raspuns > 9);
				
				if(raspuns == 1){
					cmd.index = CS_PLAY;
					printf("Se asteapta un adversar...\n");
				}
				else if(raspuns == 2) {
					cmd.index = CS_GET_USER_INFO;
					if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("Conexiunea pierduta cu serverul\n");
						return errno;
					}
					
					struct user_info userInfo;
					
					if (recv (sd, &userInfo, sizeof(userInfo), 0) <= 0)
					{
						perror ("[client]Conexiunea pierduta cu serverul.\n");
						return errno;
					}
					
					printf("\033[0;33m++++++++++++++++++++++++++++++++++++++++++\n");
					printf("+        Informatii despre contul tau            \n");
					printf("+                                        \n");
					printf("+  Id = %d  \n", userInfo.id);
					printf("+  Nume utilizator = %s \n", userInfo.user);
					printf("+  Ultima conectare = %d   \n", userInfo.last_login);
					printf("+  Scor = %d     \n", userInfo.score);
					printf("+  Nivel = %d   \n", userInfo.level);
					printf("+  Experienta = %d \n", userInfo.exp);
					printf("++++++++++++++++++++++++++++++++++++++++++\n\033[0;37m");
					
					char tmp[1];
					printf("Continuati?\n");
					scanf("%s", tmp);
					goto ADMIN_MENU_STEP;
				}
				else if(raspuns == 4) {
					
					cmd.index = CS_REGISTER;
					if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("[client]Eroare la send() catre client.\n");
						return errno;
					}
					
					struct account_packet account;
					printf("User = ");
					scanf("%s", account.user);
					printf("Password = ");
					scanf("%s", account.password);
					
					if (send (sd, &account, sizeof(account), 0) <= 0)
					{
						perror ("[client]Eroare la send() catre client.\n");
						return errno;
					}
					break;
				}
				else if(raspuns == 5) {
					printf("Introduceti numele utilizatorului: ");
					scanf("%s", cmd.buffer);
					cmd.index = CS_ADMIN_GET_USER_INFO;
					if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("Conexiunea pierduta cu serverul\n");
						return errno;
					}
					
					struct user_info userInfo;
					
					if (recv (sd, &userInfo, sizeof(userInfo), 0) <= 0)
					{
						perror ("[client]Conexiunea pierduta cu serverul.\n");
						return errno;
					}
					
					printf("\033[0;33m++++++++++++++++++++++++++++++++++++++++++\n");
					printf("+      Informatii despre utilizatorul cautat            \n\n");
					printf("+  Id = %d  \n", userInfo.id);
					printf("+  Nume utilizator = %s \n", userInfo.user);
					printf("+  Ultima conectare = %d   \n", userInfo.last_login);
					printf("+  Scor = %d     \n", userInfo.score);
					printf("+  Nivel = %d   \n", userInfo.level);
					printf("+  Experienta = %d \n", userInfo.exp);
					printf("++++++++++++++++++++++++++++++++++++++++++\n\033[0;37m");
					
					char tmp[1];
					printf("Continuati?\n");
					scanf("%s", tmp);
					goto ADMIN_MENU_STEP;
					
					break;
				}
				else if (raspuns == 6) {
					cmd.index = CS_ADMIN_DELETE_USER;
					printf("Introduceti numele utilizatorului: ");
					scanf("%s", cmd.buffer);
				}
				else if (raspuns == 7) {
					cmd.index = CS_ADMIN_BAN_USER;
					printf("Introduceti numele utilizatorului: ");
					scanf("%s", cmd.buffer);
				}
				else if(raspuns == 8) {
					cmd.index = CS_ADMIN_DISCONNECT_USER;
					printf("Introduceti numele utilizatorului: ");
					scanf("%s", cmd.buffer);
					
				}
				else if(raspuns == 9){
					cmd.index = CS_DISCONNECT;
				}
				
				if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
				{
					perror ("Conexiunea pierduta cu serverul\n");
					return errno;
				}
				
				break;
			case SC_START_GAME:;
				
				struct player_packet playerPacket;
				if (recv (sd, &playerPacket, sizeof(playerPacket), 0) <= 0)
				{
					perror ("[client]Conexiunea pierduta cu serverul.\n");
					return errno;
				}
				
				if(playerPacket.winner > 0){
					goto MENU_STEP;
				}
				
				clear();
				printf("%s", cmd.buffer);
				
				char enemyName[64];
				if(playerPacket.id == playerPacket.match.id1){
					printf("Piesa ta este: \033[0;33mX\033[0;37m\n");
					strcpy(enemyName, playerPacket.match.userInfo2.user);
					
				}
				else{
					
					printf("Piesa ta este: \033[0;31mO\033[0;37m\n");
					strcpy(enemyName, playerPacket.match.userInfo1.user);
				}
				
				int i, j;
				for(i = 0; i < 6; ++i)
					for(j = 0; j < 7; ++j){
						
						if(playerPacket.match.field[i][j] != 0 && playerPacket.match.field[i][j] == playerPacket.match.id1){
							//printf("\033[0;33m Caracterul tau: O\033[0;37m\n");
							strcpy(field[i][j], "\033[0;33mX\033[0;37m");
						}
						
						else if(playerPacket.match.field[i][j] != 0){
							//printf("\033[0;31m Caracterul tau: O\033[0;37m\n");
							
							strcpy(field[i][j], "\033[0;31mO\033[0;37m");
						}
						else{
							strcpy(field[i][j], "-");
						}
					}
				
				//printf("Adversari: %s si %s.\n", playerPacket.match.userInfo1.user, playerPacket.match.userInfo2.user);
				
				printf("   1  2  3  4  5  6  7           +++++++++++++++++++++\n");
				printf(" +--+--+--+--+--+--+--+          + 	Adversarul tau\n");
				printf("1|%s |%s |%s |%s |%s |%s |%s |          +\n",
				field[0][0], field[0][1], field[0][2], field[0][3], field[0][4], field[0][5], field[0][6]);
				printf(" +--+--+--+--+--+--+--+          +  nume = %s\n", enemyName);
				printf("2|%s |%s |%s |%s |%s |%s |%s |          +  Nivel = 100\n",
				field[1][0], field[1][1], field[1][2], field[1][3], field[1][4], field[1][5], field[1][6]);
				printf(" +--+--+--+--+--+--+--+          +++++++++++++++++++++\n");
				printf("3|%s |%s |%s |%s |%s |%s |%s |    \n",
				field[2][0], field[2][1], field[2][2], field[2][3], field[2][4], field[2][5], field[2][6]);
				printf(" +--+--+--+--+--+--+--+    \n");
				printf("4|%s |%s |%s |%s |%s |%s |%s |    \n",
				field[3][0], field[3][1], field[3][2], field[3][3], field[3][4], field[3][5], field[3][6]);
				printf(" +--+--+--+--+--+--+--+    \n");
				printf("5|%s |%s |%s |%s |%s |%s |%s |    \n",
				field[4][0], field[4][1], field[4][2], field[4][3], field[4][4], field[4][5], field[4][6]);
				printf(" +--+--+--+--+--+--+--+    \n");
				printf("6|%s |%s |%s |%s |%s |%s |%s |    \n",
				field[5][0], field[5][1], field[5][2], field[5][3], field[5][4], field[5][5], field[5][6]);
				printf(" +--+--+--+--+--+--+--+    \n");
				
				
				// Daca este randul acestui client, citesc pozitia unde sa adauge piesa
				if(playerPacket.id == playerPacket.match.turn){
					do{
						printf("Introduceti indicele coloanei: ");
						scanf("%d", &raspuns);
					} while(raspuns < 1 || raspuns > 7);
				
					cmd.index = CS_MOVE;
					cmd.buffer[0] = raspuns;
					cmd.buffer[1] = '\0';
					if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
					{
						perror ("Conexiunea pierduta cu serverul\n");
						return errno;
					}
				}
				break;
				
			case SC_CHECK_CONNECTION: // Verificare conexiune client-server
				if (send (sd, &cmd, sizeof(cmd), 0) <= 0)
				{
					perror ("Conexiunea pierduta cu serverul\n");
					return errno;
				}
				break;
				
			case SC_DISCONNECT:
				printf ("[client] Ai fost deconectat de la server.\n");
				goto SERVER_END;
				
			default:
				/* afisam mesajul primit */
				printf ("[client]Mesajul primit este: %s\n", cmd.buffer);
				break;
		}
	}

  /* inchidem conexiunea, am terminat */
  SERVER_END:
  close (sd);
}