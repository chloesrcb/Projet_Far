#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>

void *receive(void *pth_arg) {
	int ret = 0;
	int res;
	long cfd = (long)pth_arg;
	char buf[100] = {0};
	while(1) {
		bzero(&buf, sizeof(buf));
		ret = recv(cfd, &buf, sizeof(buf),0);
		if (-1 == ret) {
			perror("recv failed");
		}

		else if (ret > 0){
			printf("recv from client %s \n",buf);
			res=strlen(buf);
		}
		ret = send(cfd,&res,sizeof(int), 0);
		if (-1 == ret)
		perror("send failed");
	}
}

int main (int argc,char *argv[]){
	int dS =socket(PF_INET,SOCK_STREAM,0);

	struct sockaddr_in ad;
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr=INADDR_ANY;
	ad.sin_port=htons(atoi(argv[1]));
	bind(dS,(struct sockaddr*)&ad,sizeof(ad));
	listen(dS,7);
	pthread_t id;
	int resultat=-1;
	while(1){
				struct sockaddr_in aC;
				int size = sizeof(aC);
				resultat=accept(dS,(struct sockaddr*)&aC,&size);
				if(-1==resultat){
					perror("accept error");
				}
				else if(resultat==0){
					printf("Soket ferme");
				}
				int ret = pthread_create(&id,NULL,receive,(void*)resultat);
				if(-1 == ret)
				 perror("accept failed");
	}

	return 0;
}
