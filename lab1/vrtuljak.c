#define _XOPEN_SOURCE 500
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <stdatomic.h>
#include <semaphore.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

struct my_msgbuf {
    long mtype;
    char mtext[200];
};

int msqid,msqid_p_v,msqid_prosljedi,msqid_cekaj;

void retreat(int failure)
{
    if (msgctl(msqid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (msgctl(msqid_p_v, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (msgctl(msqid_prosljedi, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    if (msgctl(msqid_cekaj, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        exit(1);
    }

    exit(0);
}

int broj_posjetitelja = 3;
//int znacka = 1;

void posjetitelj(int i,int znacka){

	struct my_msgbuf buf;
	int key = 12345;
	int key_p_v = 1200 + i;
	int key_prosljedi = 1400+(i+1)%8;
	int key_cekaj = 1400+i;
	long message_type = 111;
	struct my_msgbuf znacka_txt;

	if ((msqid = msgget(key, 0600 | IPC_CREAT)) == -1) {

		perror("msgget");
		exit(1);
	}

	if ((msqid_p_v = msgget(key_p_v, 0600 | IPC_CREAT)) == -1) {

		perror("msgget");
		exit(1);
	}

	if ((msqid_prosljedi = msgget(key_prosljedi, 0600 | IPC_CREAT)) == -1) {

		perror("msgget");
		exit(1);
	}

	if ((msqid_cekaj = msgget(key_cekaj, 0600 | IPC_CREAT)) == -1) {

		perror("msgget");
		exit(1);
	}


	for (int j = 0; j<3; j++){

		char text[] = "Zelim se voziti ";
		char integer_string[32];
		sprintf(integer_string, "%d", i);
		//printf("Ja sam posjetitelj %d\n",i);
		strcat(text,integer_string);
		memcpy(buf.mtext, text, strlen(text)+1);
		buf.mtype = 1;
		znacka_txt.mtype=1;

		int r = rand()%2000+100;
		usleep(r);
		//printf("%s\n",text);
		if(znacka!=i){
			//printf("%d nema znacku!\n",key_cekaj);
			if (msgrcv(msqid_cekaj, (struct msgbuf *)&znacka_txt, sizeof(znacka_txt)-sizeof(long), 0, 0) == -1) {

				    perror("msgrcv");
				    exit(1);
			}
			znacka = i;
			//printf("Dobio sam znacku");
		}

		if(znacka==i){

			if (msgsnd(msqid, (struct msgbuf *)&buf, strlen(text)+1, 0) == -1){

				perror("msgsnd");
			}

			if (msgrcv(msqid_p_v, (struct msgbuf *)&buf, sizeof(buf)-sizeof(long), 0, 0) == -1) {

				    perror("msgrcv");
				    exit(1);
			}

			printf("%s\n",buf.mtext);
			znacka++;

			char text[] = "Eo ti znacka";
			memcpy(znacka_txt.mtext, text, strlen(text)+1);
			znacka_txt.mtype = 1;
			//printf("%d prosljedujem znacku!\n",key_prosljedi);
			if (msgsnd(msqid_prosljedi, (struct msgbuf *)&buf, strlen(text)+1, 0) == -1){

				perror("msgsnd");
			}

		}
		if (msgrcv(msqid_p_v, (struct msgbuf *)&buf, sizeof(buf)-sizeof(long), 0, 0) == -1) {

			    perror("msgrcv");
			    exit(1);
		}
		printf("%s\n",buf.mtext);
	}

	printf("Posjetitelj %d zavrsio\n",i);

}

void vrtuljak(){

	struct my_msgbuf buf;
	int key = 12345;
	int max_mjesta= 4;
	int broj_voznji = 8*3;
	if ((msqid = msgget(key, 0600 | IPC_CREAT)) == -1) { /* connect to the queue */
		perror("msgget");
		exit(1);
	}


	while(broj_voznji!=0){

		int posjetitelji[max_mjesta];
		for(int j = 0; j<max_mjesta; j++){

			if (msgrcv(msqid, (struct msgbuf *)&buf, sizeof(buf)-sizeof(long), 0, 0) == -1) {

				    perror("msgrcv");
				    exit(1);
			}
			printf("%s\n",buf.mtext);
			int id_posjetitelj = buf.mtext[strlen(buf.mtext)-1] - '0';
			posjetitelji[j]=id_posjetitelj;
			int key_p_v=1200+id_posjetitelj;
			//printf("Id koji se vozi: %d\n",id_posjetitelj);

			if ((msqid_p_v = msgget(key_p_v, 0600 | IPC_CREAT)) == -1) { /* connect to the queue */
				perror("msgget");
				exit(1);
			}

			char text[] = "Sjedni ";
			char integer_string[32];
			sprintf(integer_string, "%d", id_posjetitelj);
			strcat(text,integer_string);
			memcpy(buf.mtext, text, strlen(text)+1);
			buf.mtype = 1;

			if (msgsnd(msqid_p_v, (struct msgbuf *)&buf, strlen(text)+1, 0) == -1){

				perror("msgsnd");
			}

		}
		sleep(1);
		printf("Pokrecem vrtuljak!\n");

		int r = rand()%3000+1000;
		usleep(r);
		printf("Vrtuljak zaustavljen!\n");

		for(int k = 0; k<max_mjesta; k++){

			int id_posjetitelj = posjetitelji[k];
			int key_p_v=1200+id_posjetitelj;

			if ((msqid_p_v = msgget(key_p_v, 0600 | IPC_CREAT)) == -1) { /* connect to the queue */
				perror("msgget");
				exit(1);
			}

			char text[] = "Ustani ";
			char integer_string[32];
			sprintf(integer_string, "%d", id_posjetitelj);
			strcat(text,integer_string);
			memcpy(buf.mtext, text, strlen(text)+1);
			buf.mtype = 1;

			if (msgsnd(msqid_p_v, (struct msgbuf *)&buf, strlen(text)+1, 0) == -1){

				perror("msgsnd");
			}
		}
	broj_voznji-=4;

	}
}

int main(void){

	int i, pid,msqid;
	srand(time(NULL));
	int znacka = rand()%8;

	sigset(SIGINT, retreat);
	for (i = 0; i < 8; i++) { //stvaranje procesa pusaca

		switch (pid = fork ()) {

		case -1:
			printf ("R: Ne mogu stvoriti novi proces!\n"); //neuspjesno stvaranje procesa
			exit (1);

		case 0:

			posjetitelj(i,znacka);
			exit (0);

		default:

			//vrtuljak();
			break;

		}
}

		switch (pid = fork ()) {

		case -1:
			printf ("R: Ne mogu stvoriti novi proces!\n"); //neuspjesno stvaranje procesa
			exit (1);

		case 0:

			vrtuljak();
			exit (0);

		default:

			//vrtuljak();
			break;

		}

	for (i = 0; i < 9; i++){

        	wait (NULL); //cekaj na procese djece da se nebi stvorili zombiji

	}

	return 0;
}
