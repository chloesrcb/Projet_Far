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

int rcvTCP(int sock,char *msg,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/

	/* On recoit d'abord la taille de la chaine pour savoir combien d'octets recevoir */
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
	

	/*si il y a une erreur res= 0 ou -1*/
	if(res==-1 || res==0){
		return res;
	}
	return nbTotalSent;
}

int sendTCP(int sock,char *msg, int sizeoctets,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/
	int taille[1];
	taille[0]=sizeoctets;

	/* On envoie d'abord la taille de la chaine pour prevenir combien d'octets recevoir */
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


void *envoiThread(void *param){
	char msg[200];
	int res;
	int socketServer=*(int *)param;
	while(1){
		printf(">");
		/*On recupere le message de 200 caracteres max*/
		fgets(msg,200,stdin);

		res=sendTCP(socketServer,msg,strlen(msg)+1,0);


		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de l'envoi");
			close(socketServer);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée");
			close(socketServer);
			exit(0);
		}

		if(strcmp(msg,"fin\n")==0){
			printf("Fin de la conversation\n");
			break;
		}
	}
	exit(0);
}

int main(int argc, char const*argv[]){
	if (argc!=3){
		printf("Il faut 2 arguments : IP + numéro de port\n");
		return 0;
	}


	/* Initialisation de la socket*/
	int dS=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in aS;
	aS.sin_family=AF_INET;
	inet_pton(AF_INET,argv[1],&(aS.sin_addr));
	aS.sin_port=htons(atoi(argv[2]));
	socklen_t lgA=sizeof(struct sockaddr_in);
	connect(dS,(struct sockaddr *) &aS,lgA);

	pthread_t thread;
	pthread_create(&thread,NULL,envoiThread,&dS);

	int res1;
	char msg[200];

	while(1){

		res1=rcvTCP(dS,msg,0);	
		if(res1==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(dS);
			exit(0);
		}
		else if(res1==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(dS);
			exit(0);
		}
		else if(res1<strlen(msg)+1){
				printf("Message non reçu entièrement\n");
		}

		printf("%s",msg);
		printf(">");
		fflush(stdout);

		if(strcmp(msg,"fin\n")==0){
			printf("Fin de la conversation\n");
			break;
		}

	}


	close(dS);
	return 0;
	
}