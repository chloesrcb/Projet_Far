#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>

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


int main(int argc, char const*argv[]){
	if (argc!=4){
		printf("Il faut 3 arguments : IP + numéro de port + 1/2\n");
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

	int res1=0;
	char msg[200];

	if(atoi(argv[3])==1){/*Client 1, commence la discussion*/
		while(1){
			/*Envoi*/
			printf("Saisissez une chaine de caractère\n");
			/*On recupere le message de 200 caracteres max*/
			fgets(msg,200,stdin);

			res1=sendTCP(dS,msg,strlen(msg)+1,0);


			if(res1==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de l'envoi");
				close(dS);
				exit(0);
			}
			else if(res1==0){/*Le serveur est fermé on arrete la communication*/
				printf("Socket fermée");
				close(dS);
				exit(0);
			}
			

			/*Ecoute*/
			printf("En attente de message...\n");
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
			else if(res1==strlen(msg)+1){
					printf("Message reçu :\n");
			}

			printf("%s\n",msg);


		}
	}
	else{/*Client 2,commence par ecouter*/
		while(1){
			/* Ecoute*/
			printf("En attente de message...\n");
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
			else if(res1==strlen(msg)+1){
					printf("Message reçu :\n");
			}
			
			printf("%s\n",msg);


			/*Envoi*/
			printf("Saisissez une chaine de caractère\n");
			/*On recupere le message de 200 caracteres max*/
			fgets(msg,200,stdin);

			res1=sendTCP(dS,msg,strlen(msg)+1,0);
			if(res1==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de l'envoi");
				close(dS);
				exit(0);
			}
			else if(res1==0){/*Le serveur est fermé on arrete la communication*/
				printf("Socket fermée");
				close(dS);
				exit(0);
			}
		}
	}


	close(dS);
	return 0;
	
}
