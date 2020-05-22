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



typedef struct Salon{
	char description[250];
	int nbSalon;
	int nbPlace;
	int clients[2];
	int clientsFile[2];
}Salon;

typedef struct structThread{
	char pseudo[50];
	int numSalon;
	int client;
	Salon *salons;
}StructThread;

typedef struct structThreadSelection{
	Salon *salons;
	int nbSalon;
	int client;
	int clientFile;
}StructThreadSelection;



int rcvSize(int sock,long *size,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/

	/* On recoit d'abord la taille de la chaine pour savoir combien d'octets recevoir */
	long tailleMessage;
	int res=recv(sock,&tailleMessage,sizeof(long),option);
	if(res==-1 || res==0){
		return res;
	}
	*size=tailleMessage;
	return tailleMessage;
}


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

int sendSize(int sock, long sizeoctets,int option){
	/*msg vu ici comme une suite d'octets, un tab d'octets*/
	long taille[1];
	taille[0]=sizeoctets;

	/* On envoie d'abord la taille de la chaine pour prevenir combien d'octets recevoir */
	int res=send(sock,&taille,sizeof(long),option);
	if(res==-1 || res==0){
		return res;
	}
	return res;

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


void *fonctionthread(void *param){
	StructThread *clients=(StructThread*)param;
	char pseudo[50];
	char msgRecv[200];
	char msgSnd[250];
	int res;


	sprintf(msgSnd,"Entrez votre pseudo avant de pouvoir communiquer...\n");
	res=sendTCP(clients->client,msgSnd,strlen(msgSnd)+1,0);

	if(res==-1){/*Erreur lors de la communication, on l'arrete*/
		perror("Erreur lors de l'envoi");
		close(clients->client);
		pthread_exit(NULL);
	}
	else if(res==0){/*Le serveur est fermé on arrete la communication*/
		printf("Socket fermée");
		close(clients->client);
		pthread_exit(NULL);
	}

	res=rcvTCP(clients->client,pseudo,0);	
	if(res==-1){/*Erreur lors de la communication, on l'arrete*/
		perror("Erreur lors de la reception\n");
		close(clients->client);
		pthread_exit(NULL);
	}
	else if(res==0){/*Le client est fermé on arrete la communication*/
		printf("Socket fermée\n");
		close(clients->client);
		pthread_exit(NULL);
	}
	else if(res<strlen(pseudo)+1){
			printf("Message non reçu entièrement\n");
	}
	else if(res==strlen(pseudo)+1){
			printf("Message reçu en entier\n");
	}
	pseudo[strlen(pseudo)-1]='\0';
	printf("pseudo : %s\n",pseudo);
	sprintf(clients->pseudo,"%s",pseudo);

	while(1){
		res=rcvTCP(clients->client,msgRecv,0);	
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception test\n");
			close(clients->client);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le client est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(clients->client);
			pthread_exit(NULL);
		}
		else if(res<strlen(msgRecv)+1){
				printf("Message non reçu entièrement\n");
		}
		else if(res==strlen(msgRecv)+1){
				printf("Message reçu en entier\n");
		}
		msgRecv[strlen(msgRecv)]='\0';
		printf("message : %s\n",msgRecv);
		if(strcmp(msgRecv,"fin\n")==0){
			for(int i=0;i<2;i++){
				if(clients->salons[clients->numSalon].clients[i]!=clients->client && clients->salons[clients->numSalon].clients[i]!=-1){
					res=sendTCP(clients->salons[clients->numSalon].clients[i],msgRecv,strlen(msgRecv)+1,0);
					if(res==-1){/*Erreur lors de la communication, on l'arrete*/
						perror("Erreur lors de l'envoi\n");
						close(clients->client);
						pthread_exit(NULL);
					}
					else if(res==0){/*Le client est fermé on arrete la communication*/
						printf("Socket fermée\n");
						close(clients->client);
						pthread_exit(NULL);
					}
				}
			}
			

			break;
		}


		sprintf(msgSnd,"%s>%s",clients->pseudo,msgRecv);
		printf("transmission au client: %d \n",clients->salons[clients->numSalon].clients[0]);
		/*On le transmet au client 2*/
		for(int i=0;i<2;i++){
			if(clients->salons[clients->numSalon].clients[i]!=clients->client && clients->salons[clients->numSalon].clients[i]!=-1){
				res=sendTCP(clients->salons[clients->numSalon].clients[i],msgSnd,strlen(msgSnd)+1,0);
				if(res==-1){/*Erreur lors de la communication, on l'arrete*/
					char error[250];
					sprintf(error,"Erreur lors de la reception test %d %d %d\n",i,clients->numSalon,clients->salons[clients->numSalon].clients[i]);
					perror(error);
					close(clients->client);
					pthread_exit(NULL);
				}
				else if(res==0){/*Le client est fermé on arrete la communication*/
					printf("Socket fermée\n");
					close(clients->client);
					pthread_exit(NULL);
				}
			}
		}


	}
	pthread_exit(NULL);
}

void *fonctionthreadFile(void *param){
	StructThread *clients=(StructThread*)param;
	char buffer[200],titre[50];
	int res;

	while(1){
		res=rcvTCP(clients->client,titre,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(clients->client);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(clients->client);
			pthread_exit(NULL);
		}
		printf("Filename: %s\n",titre);

		
		
		for(int i=0;i<2;i++){
			if(clients->salons[clients->numSalon].clientsFile[i]!=clients->client){
				res=sendTCP(clients->salons[clients->numSalon].clientsFile[i],titre,strlen(titre)+1,0);
				if(res==-1){/*Erreur lors de la communication, on l'arrete*/
					perror("Erreur lors de la reception\n");
					close(clients->client);
					pthread_exit(NULL);
				}
				else if(res==0){/*Le client est fermé on arrete la communication*/
					printf("Socket fermée\n");
					close(clients->client);
					pthread_exit(NULL);
				}
			}
		}


		long nbOctetFile;
		res=rcvSize(clients->client,&nbOctetFile,0);
		printf("File size=%ld\n",nbOctetFile);

		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(clients->client);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(clients->client);
			pthread_exit(NULL);
		}

		for(int i=0;i<2;i++){
			if(clients->salons[clients->numSalon].clientsFile[i]!=clients->client){
				res=sendSize(clients->salons[clients->numSalon].clientsFile[i],nbOctetFile,0);
				if(res==-1){/*Erreur lors de la communication, on l'arrete*/
					perror("Erreur lors de la reception\n");
					close(clients->client);
					pthread_exit(NULL);
				}
				else if(res==0){/*Le client est fermé on arrete la communication*/
					printf("Socket fermée\n");
					close(clients->client);
					pthread_exit(NULL);
				}
			}
		}

		int nbRecus=0;
		while(nbRecus<nbOctetFile){
			res=rcvTCP(clients->client,buffer,0);
			if(res==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de la reception\n");
				close(clients->client);
				pthread_exit(NULL);
			}
			else if(res==0){/*Le serveur est fermé on arrete la communication*/
				printf("Socket fermée\n");
				close(clients->client);
				pthread_exit(NULL);
			}
			nbRecus+=res-1;


			for(int i=0;i<2;i++){
				if(clients->salons[clients->numSalon].clientsFile[i]!=clients->client){
					res=sendTCP(clients->salons[clients->numSalon].clientsFile[i],buffer,strlen(buffer)+1,0);
					if(res==-1){/*Erreur lors de la communication, on l'arrete*/
						perror("Erreur lors de la reception\n");
						close(clients->client);
						pthread_exit(NULL);
					}
					else if(res==0){/*Le client est fermé on arrete la communication*/
						printf("Socket fermée\n");
						close(clients->client);
						pthread_exit(NULL);
					}
				}
			}
			
		
		}
		printf("Fichier envoyé\n");
	}
	pthread_exit(NULL);
}

int placeDispoSalon(Salon salon){
	for(int i=0;i<salon.nbPlace;i++){
		if(salon.clients[i]==-1){
			return i;
		}
	}
	return -1;
}

void *fonctionthreadChoixSalon(void *param){
	StructThreadSelection *parame=(StructThreadSelection*)param;
	int sClient=parame->client;
	int sClientFile=parame->clientFile;
	int res;
	char msg[250];
	sprintf(msg,"Veuillez choisir un numero de salon en entrant son numero, obtenez la description d'un salon en envoyant '-N'\n");
	res=sendTCP(sClient,msg,strlen(msg)+1,0);
	if(res==-1){/*Erreur lors de la communication, on l'arrete*/
		perror("Erreur lors de la reception\n");
		close(sClient);
		pthread_exit(NULL);
	}
	else if(res==0){/*Le client est fermé on arrete la communication*/
		printf("Socket fermée\n");
		close(sClient);
		pthread_exit(NULL);
	}

	int selectionUnfinished=1;
	int numSalon;
	while(selectionUnfinished){
		res=rcvTCP(sClient,msg,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(sClient);
			pthread_exit(NULL);
		}
		else if(res==0){/*Le client est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(sClient);
			pthread_exit(NULL);
		}
		numSalon=atoi(msg);
		if(numSalon<0 && numSalon*(-1)<parame->nbSalon){
			sprintf(msg,"%s",parame->salons[numSalon*(-1)].description);
			res=sendTCP(sClient,msg,strlen(msg)+1,0);
			if(res==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de la reception\n");
				close(sClient);
				pthread_exit(NULL);
			}
			else if(res==0){/*Le client est fermé on arrete la communication*/
				printf("Socket fermée\n");
				close(sClient);
				pthread_exit(NULL);
			}
		}
		else if(numSalon<parame->nbSalon){
			int numPlace=placeDispoSalon(parame->salons[numSalon]);
			if(numPlace==-1){
				sprintf(msg,"Il n'y a plus de place dans ce salon, veuillez en selectionner un autre.");
				res=sendTCP(sClient,msg,strlen(msg)+1,0);
				if(res==-1){/*Erreur lors de la communication, on l'arrete*/
					perror("Erreur lors de la reception\n");
					close(sClient);
					pthread_exit(NULL);
				}
				else if(res==0){/*Le client est fermé on arrete la communication*/
					printf("Socket fermée\n");
					close(sClient);
					pthread_exit(NULL);
				}
				continue;
			}
			parame->salons[numSalon].clients[numPlace]=sClient;
			parame->salons[numSalon].clientsFile[numPlace]=sClientFile;

			pthread_t t,tf;
			StructThread *struct1=malloc(sizeof(StructThread));
			StructThread *structFile=malloc(sizeof(StructThread));
			struct1->client=sClient;
			struct1->numSalon=numSalon;
			struct1->salons=parame->salons;
			structFile->client=sClientFile;
			structFile->numSalon=numSalon;
			structFile->salons=parame->salons;

			pthread_create(&t,NULL,fonctionthread,struct1);
			pthread_create(&tf,NULL,fonctionthreadFile,structFile);
			selectionUnfinished=0;

		}
		else{
			sprintf(msg,"Numero de salon trop haut, il y a %d salons.",parame->nbSalon);
			res=sendTCP(sClient,msg,strlen(msg)+1,0);
			if(res==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de la reception\n");
				close(sClient);
				pthread_exit(NULL);
			}
			else if(res==0){/*Le client est fermé on arrete la communication*/
				printf("Socket fermée\n");
				close(sClient);
				pthread_exit(NULL);
			}
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
	listen(dS,20);

	int N=10;
	Salon *salons=malloc(sizeof(Salon)*N);
	for(int i=0;i<N;i++){
		sprintf(salons[i].description,"salon n%d",i);
		salons[i].nbSalon=i;
		salons[i].nbPlace=2;
		salons[i].clients[0]=-1;salons[i].clients[1]=-1;
		salons[i].clientsFile[0]=-1;salons[i].clientsFile[1]=-1;
	}



	struct sockaddr_in aC;
	socklen_t lg=sizeof(struct sockaddr_in);
	



	pthread_t thread1=0;
	while(1){
		/*Connexion des clients*/
		printf("En attente de client ...\n");
		int SClient=accept(dS,(struct sockaddr*)&aC,&lg);
		int SClientFile=accept(dS,(struct sockaddr*)&aC,&lg);
		printf("Un client se connecte\n");

		StructThreadSelection *selec=malloc(sizeof(StructThreadSelection));
		selec->client=SClient;
		selec->clientFile=SClientFile;
		selec->nbSalon=N;
		selec->salons=salons;
		pthread_create(&thread1,NULL,fonctionthreadChoixSalon,selec);



	}
	close(dS);

	return 0;
	
}
