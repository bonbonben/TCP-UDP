#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>

/*define the biggest data size you can handle*/
#define maxDataSize 1024

#define ERR_EXIT(m) \
        do \
        { \
                perror(m); \
                exit(EXIT_FAILURE); \
        } while(0)

int isReadable(int sock,int * error,int timeOut) { // milliseconds
  fd_set socketReadSet;
  FD_ZERO(&socketReadSet);
  FD_SET(sock,&socketReadSet);
  struct timeval tv;
  if (timeOut) {
    tv.tv_sec  = timeOut / 1000;
    tv.tv_usec = (timeOut % 1000) * 1000;
  } else {
    tv.tv_sec  = 0;
    tv.tv_usec = 0;
  } // if
  if (select(sock+1,&socketReadSet,0,0,&tv) == -1) {
    *error = 1;
    return 0;
  } // if
  *error = 0;
  return FD_ISSET(sock,&socketReadSet) != 0;
} /* isReadable */


int main(int argc,char *argv[])
{
    	int sock;
	
	int timeOut,flags;

	/*tell you to enter ip and port*/
	if(argc<3)
	{
		fprintf(stderr,"usage:%s port host\n",argv[0]);
		exit(1);
	}

    	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket");

	struct sockaddr_in servaddr;
    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(atoi(argv[1]));
    	servaddr.sin_addr.s_addr = inet_addr(argv[2]);

/*    	int ret;
    	char sendbuf[1024] = {0};
    	char recvbuf[1024] = {0};
    	while (fgets(sendbuf, sizeof(sendbuf), stdin) != NULL)
    	{
        	sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
		ret = recvfrom(sock, recvbuf, sizeof(recvbuf), 0, NULL, NULL);
        	if (ret == -1)
        	{
            		if (errno == EINTR)
                	continue;
            		ERR_EXIT("recvfrom");
        	}

        	fputs(recvbuf, stdout);
        	memset(sendbuf, 0, sizeof(sendbuf));
        	memset(recvbuf, 0, sizeof(recvbuf));
    	}
    	close(sock);*/

	flags = 0;
  	timeOut = 100; // ms

	while(1)
	{
		char msg[maxDataSize];
		scanf("%s",msg);/*read message*/
		if(sendto(sock,msg,strlen(msg),0,(struct sockaddr *)&servaddr, sizeof(servaddr))==-1)
		{
			perror("send");
			exit(1);
		}
		/*send file when you type file*/
		if(strcmp(msg,"file")==0)
		{
			char filename3[1024];
			printf("filename:");/*enter file name*/
			scanf("%s",filename3);/*read file name*/
			sendto(sock,filename3,maxDataSize,0,(struct sockaddr *)&servaddr, sizeof(servaddr));/*send file name*/
			char sdbuf[maxDataSize];
			printf("sending %s to server......\n",filename3);
			FILE *fs=fopen(filename3,"r");/*read file*/
			/*can not find file*/
			if(fs==NULL)
			{
				printf("file %s not found\n",filename3);
				exit(1);
			}
			bzero(sdbuf,maxDataSize);
			int fs_block_sz;
			while((fs_block_sz=fread(sdbuf,sizeof(char),maxDataSize,fs))>0)
			{
		    		if(sendto(sock,sdbuf,fs_block_sz,0,(struct sockaddr *)&servaddr, sizeof(servaddr))<0)
		    		{
		        		fprintf(stderr,"ERROR:Failed to send file %s.(errno = %d)\n",filename3,errno);
		        		break;
		    		}
		    		else bzero(sdbuf,maxDataSize);
			}
			printf("file %s from client was sent\n",filename3);
			fclose(fs);
		}
		/*quit when you send quit*/
		if(strcmp(msg,"quit")==0)
		{
			printf("goodbye\n");
			close(sock);
			exit(1);
		}
	}

    	return 0;
}
