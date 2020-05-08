#include <stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include <limits.h>
#include <dirent.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<netdb.h>
#include <pthread.h> 



int dS,dSFile;

typedef struct paramThread
{
	char filename[100];
}ParamThread;


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

int get_last_tty() {
  FILE *fp;
  char path[1035];
  fp = popen("/bin/ls /dev/pts", "r");
  if (fp == NULL) {
    printf("Impossible d'exécuter la commande\n" );
    exit(1);
  }
  int i = INT_MIN;
  while (fgets(path, sizeof(path)-1, fp) != NULL) {
    if(strcmp(path,"ptmx")!=0){
      int tty = atoi(path);
      if(tty > i) i = tty;
    }
  }

  pclose(fp);
  return i;
}

FILE* new_tty() {
  pthread_mutex_t the_mutex;  
  pthread_mutex_init(&the_mutex,0);
  pthread_mutex_lock(&the_mutex);
  system("gnome-terminal"); sleep(1);
  char *tty_name = ttyname(STDIN_FILENO);
  int ltty = get_last_tty();
  char str[2];
  sprintf(str,"%d",ltty);
  int i;
  for(i = strlen(tty_name)-1; i >= 0; i--) {
    if(tty_name[i] == '/') break;
  }
  tty_name[i+1] = '\0';  
  strcat(tty_name,str);  
  FILE *fp = fopen(tty_name,"wb+");
  pthread_mutex_unlock(&the_mutex);
  pthread_mutex_destroy(&the_mutex);
  return fp;
}

void selecFile(char filename[100]){
	FILE* fp1 = new_tty();
	fprintf(fp1,"%s\n","Ce terminal sera utilisé uniquement pour l'affichage");

	/* Demander à l'utilisateur quel fichier afficher*/
	DIR *dp;
	struct dirent *ep;     
	dp = opendir ("./sendFiles");
	if (dp != NULL) {
		fprintf(fp1,"Voilà la liste de fichiers :\n");
		while (ep = readdir (dp)) {
		if(strcmp(ep->d_name,".")!=0 && strcmp(ep->d_name,"..")!=0) 
		fprintf(fp1,"%s\n",ep->d_name);
		}    
		(void) closedir (dp);
	}
	else {
		perror ("Ne peux pas ouvrir le répertoire");
	}
	printf("Indiquer le nom du fichier : ");

	fgets(filename,100,stdin);
	filename[strlen(filename)-1]='\0';
	printf("%s\n",filename);
}

void *envoiFile(void *param){
	ParamThread *p=(ParamThread *)param;

	int res;
	printf("Entree\n");
	printf("Test %s\n",p->filename);
	
	char filepath[150];
	sprintf(filepath,"./sendFiles/%s",p->filename);

	FILE *fps = fopen(filepath, "r");
	if (fps == NULL){
		printf("Ne peux pas ouvrir le fichier suivant : %s",filepath);
	}
	else {
		printf("On send le title\n");
		res=sendTCP(dSFile,p->filename,strlen(p->filename)+1,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de l'envoi");
			close(dSFile);
			close(dS);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée");
			close(dS);
			close(dSFile);
			exit(0);
		}
		long size;
		fseek(fps,0L,SEEK_END);
		size=ftell(fps);
		rewind(fps);
		printf("On send la size:%ld\n",size);
		res=sendSize(dSFile,size,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de l'envoi");
			close(dSFile);
			close(dS);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée");
			close(dSFile);
			close(dS);
			exit(0);
		}
		char buffer[200];    
		
		while (fgets(buffer, 200, fps) != NULL) {
			res=sendTCP(dSFile,buffer,strlen(buffer)+1,0);
			if(res==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de l'envoi");
				close(dSFile);
				close(dS);
				exit(0);
			}
			else if(res==0){/*Le serveur est fermé on arrete la communication*/
				printf("Socket fermée");
				close(dSFile);
				close(dS);
				exit(0);
			}
			printf("%s\n",buffer);

		}




	}
	fclose(fps);	
	pthread_exit(NULL);
}

void *recvFile(void *param){
	int socketServer=*(int *)param;
	char buffer[200],titre[50];
	int res;
	while(1){
		res=rcvTCP(socketServer,titre,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(dSFile);
			close(dS);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(dSFile);
			close(dS);
			exit(0);
		}
		char filepath[100];
		sprintf(filepath,"./rcvFiles/%s",titre);
		printf("On recois un fichier %s\n",filepath);
		FILE *file=fopen(filepath, "w");

		long nbOctetFile; 
		res=rcvSize(socketServer,&nbOctetFile,0);
		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de la reception\n");
			close(dSFile);
			close(dS);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée\n");
			close(dSFile);
			close(dS);
			exit(0);
		}


		int nbRecus=0;
		while(nbRecus<nbOctetFile){
			res=rcvTCP(socketServer,buffer,0);
			if(nbOctetFile==-1){/*Erreur lors de la communication, on l'arrete*/
				perror("Erreur lors de la reception\n");
				close(dSFile);
				close(dS);
				exit(0);
			}
			else if(nbOctetFile==0){/*Le serveur est fermé on arrete la communication*/
				printf("Socket fermée\n");
				close(dSFile);
				close(dS);
				exit(0);
			}
			fwrite(buffer,1,strlen(buffer),file);
			nbRecus+=res;
		}
		printf("On close\n");
		fclose(file);
	}
	pthread_exit(NULL);
}

void *envoiThread(void *param){
	char msg[200],pseudo[50];
	int res;
	int socketServer=*(int *)param;
	printf("Entrez votre pseudo avant de pouvoir communiquer...\n");
	fgets(pseudo,50,stdin);

	res=sendTCP(socketServer,pseudo,strlen(pseudo)+1,0);


	if(res==-1){/*Erreur lors de la communication, on l'arrete*/
		perror("Erreur lors de l'envoi");
		close(dSFile);
		close(dS);
		exit(0);
	}
	else if(res==0){/*Le serveur est fermé on arrete la communication*/
		printf("Socket fermée");
		close(dSFile);
		close(dS);
		exit(0);
	}
	while(1){
		/*On recupere le message de 200 caracteres max*/
		fgets(msg,200,stdin);

		if(strcmp(msg,"file\n")==0){
			printf("Veuillez entrer le fichier a envoyer\n");
			
			ParamThread *param=malloc(sizeof(ParamThread));

			selecFile(param->filename);

			pthread_t thread;
			pthread_create(&thread,NULL,envoiFile,param);
			continue;
		}



		res=sendTCP(socketServer,msg,strlen(msg)+1,0);


		if(res==-1){/*Erreur lors de la communication, on l'arrete*/
			perror("Erreur lors de l'envoi");
			close(dSFile);
			close(dS);
			exit(0);
		}
		else if(res==0){/*Le serveur est fermé on arrete la communication*/
			printf("Socket fermée");
			close(dSFile);
			close(dS);
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
	dS=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in aS;
	aS.sin_family=AF_INET;
	inet_pton(AF_INET,argv[1],&(aS.sin_addr));
	aS.sin_port=htons(atoi(argv[2]));

	dSFile=socket(PF_INET,SOCK_STREAM,0);
	struct sockaddr_in aSFile;
	aSFile.sin_family=AF_INET;
	inet_pton(AF_INET,argv[1],&(aSFile.sin_addr));
	aSFile.sin_port=htons(atoi(argv[2]));

	socklen_t lgA=sizeof(struct sockaddr_in);
	socklen_t lgAFile=sizeof(struct sockaddr_in);
	connect(dS,(struct sockaddr *) &aS,lgA);
	connect(dSFile,(struct sockaddr *) &aSFile,lgAFile);

	pthread_t thread;
	pthread_create(&thread,NULL,envoiThread,&dS);
	
	pthread_create(&thread,NULL,recvFile,&dSFile);



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
		fflush(stdout);

		if(strcmp(msg,"fin\n")==0){
			printf("Fin de la conversation\n");
			break;
		}

	}


	close(dS);
	return 0;
	
}
