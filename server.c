#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <pthread.h>

typedef struct structThread{
	int clientReception;
	int clientEmission;
}StructThread;

int rcvTCP(int sock,char *msg,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/
	int tailleMessage;
	int res=recv(sock,&tailleMessage,sizeof(int),option);
	if(res==-1 || res==0){
		return res;
	}
	int sizeoctets=tailleMessage;
	res=recv(sock,msg,sizeoctets,option);

	int nbTotalSent=res;
	while(nbTotalSent!=sizeoctets && res!=-1 && res!=0){
		res=recv(sock,msg+res,sizeoctets,option);
		nbTotalSent+=res;
	}/*stop quand msg entierement envoyé ou res=-1 ou 0 ou 1*/
	

	if(res==-1 || res==0){
		return res;
	}
	/*si il y a une erreur = 0 ou -1*/
	return nbTotalSent;
}

int sendTCP(int sock,char *msg, int sizeoctets,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/
	int taille[1];
	taille[0]=sizeoctets;
	int res=send(sock,&taille,sizeof(int),option);
	if(res==-1 || res==0){
		return res;
	}


	res=send(sock,msg,sizeoctets,option);
	int nbTotalSent=res;
	while(nbTotalSent!=sizeoctets && res!=-1 && res!=0){
		res=send(sock,msg+res,sizeoctets-res,option);
		nbTotalSent+=res;
	}/*stop quand msg entierement envoyé ou res=-1 ou 0 ou 1*/
	if(nbTotalSent==sizeoctets){
		return 1; 
		/*tout le message a été envoyé*/
	}

	return res;
	/*si il y a une erreur = 0 ou -1*/
}

void *fonctionthread(void *param){
	StructThread *clients=(StructThread*)param;
	char msg[200];
	int res;
	while(1){
		res=rcvTCP(clients->clientReception,msg,0);	
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(clients->clientReception);
			close(clients->clientEmission);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le client est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(clients->clientReception);
			close(clients->clientEmission);
			pthread_exit(NULL);
		}
		else if(res<strlen(msg)+1){
				printf("Message non reçu entièrement\n");
		}
		else if(res==strlen(msg)+1){
				printf("Message reçu en entier\n");
		}

		printf("message : %s\n",msg);

		/*On le transmet au client 2*/
		res=sendTCP(clients->clientEmission,msg,strlen(msg)+1,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(clients->clientReception);
			close(clients->clientEmission);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le client est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(clients->clientReception);
			close(clients->clientEmission);
			pthread_exit(NULL);
		}


		if(strcmp(msg,"fin\n")==0){
			break;
		}
	}
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){
	if (argc!=2){
		printf("Il faut un argument : le numéro de port\n");
		return 0;
	}
	int dS=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in ad;
	/*Initialisation de la socket*/
	ad.sin_family=AF_INET;
	ad.sin_addr.s_addr=INADDR_ANY;
	ad.sin_port=htons(atoi(argv[1]));
	bind(dS,(struct sockaddr*)&ad,sizeof(ad));
	listen(dS,7);
	struct sockaddr_in aC;
	socklen_t lg=sizeof(struct sockaddr_in);

	StructThread *struct1=malloc(sizeof(StructThread));
	StructThread *struct2=malloc(sizeof(StructThread));
	pthread_t thread1=0;
	pthread_t thread2=0;
	while(1){
		/*Connexion des clients*/
		printf("En attente du client 1..\n");
		int SClient1=accept(dS,(struct sockaddr*)&aC,&lg);
		printf("Client 1 connecté, en attente de client 2...\n");
		int SClient2=accept(dS,(struct sockaddr*)&aC,&lg);
		printf("Client 2 connecté, La communication peut demarrer...\n");

		struct1->clientEmission=SClient1;
		struct1->clientReception=SClient2;

		struct2->clientEmission=SClient2;
		struct2->clientReception=SClient1;

		pthread_create(&thread1,NULL,fonctionthread,struct1);
		pthread_create(&thread2,NULL,fonctionthread,struct2);

		pthread_join(thread1,NULL);
		pthread_join(thread2,NULL);
		printf("Arret de la conversation entre ces clients\n");
	}
	close(dS);

	return 0;
	
}
