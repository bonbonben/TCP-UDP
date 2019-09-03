#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>
#include <netdb.h>

/*define the most connect request you can handle sumultaneuosly*/
#define backlog 10
/*define the biggest data size you can handle*/
#define maxDataSize 1024

int sock_fd,new_fd;
pthread_t receiveThread;

/*function receive text*/
void receiveText(void)
{
	while(1)
	{
		int numBytes;
		char buf[maxDataSize];
		if((numBytes=recv(sock_fd,buf,maxDataSize,0))==-1)
		{
			perror("recv");
			exit(1);
		}
		buf[numBytes]='\0';
		/*send file when you receive file*/
		if(strcmp(buf,"file")==0)
		{
			char filename2[1024];
			recv(sock_fd,filename2,maxDataSize,0);/*receive file name*/
			FILE *fr=fopen(filename2,"w");/*write file*/
			printf("receiveing file from server ......\n");
			if(fr==NULL)
			{
				printf("file %s cannot be opened\n",filename2);
			}
			else
			{
				bzero(buf,maxDataSize);
				int fr_block_sz=0;
	    			while((fr_block_sz=recv(sock_fd,buf,maxDataSize,0))>0)
	    			{
					int write_sz=fwrite(buf,sizeof(char),fr_block_sz,fr);
	        			if(write_sz<fr_block_sz)
					{
	            				error("file write failed\n");
	        			}
					bzero(buf,maxDataSize);
					if (fr_block_sz==0||fr_block_sz!=maxDataSize)
					{
						break;
					}
				}
				if(fr_block_sz<0)
        			{
					if(errno==EAGAIN)
					{
						printf("recv() timed out\n");
					}
					else
					{
						fprintf(stderr,"recv() failed due to errno=%d\n", errno);
					}
				}
	    			printf("received from server\n");
	    			fclose(fr);
			}
		}
		/*quit when you receive quit*/
		else if(strcmp(buf,"quit")==0)
		{
			printf("server is closed\n");
			close(sock_fd);
			exit(1);
		}
		/*print message*/
		else printf("error\n");
	}
}
int main(int argc,char *argv[])
{
	struct hostent* hostinfo;
	struct sockaddr_in serv_addr;
	/*tell you to enter ip and port*/
	if(argc<3)
	{
		fprintf(stderr,"usage:%s port host\n",argv[0]);
		exit(1);
	}
	/*get host ip*/
	if((hostinfo=gethostbyname(argv[2]))==NULL)
	{
		herror("gethostbyname");
		exit(1);
	}
	/*create socket*/
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		exit(1);
	}
	/*initialize sockaddr_in*/
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_port=htons(atoi(argv[1]));
	serv_addr.sin_addr=*((struct in_addr *)hostinfo->h_addr);
	bzero(&(serv_addr.sin_zero),8);
	/*send connect request to host*/
	if(connect(sock_fd,(struct sockaddr *)&serv_addr,sizeof(struct sockaddr))==-1)
	{
		perror("connect");
		exit(1);
	}
	/*create pthread*/
	if((pthread_create(&receiveThread,NULL,(void*)receiveText,NULL))!=0)
	{
		printf("create thread error\r\n");
		exit(1);
	}
	while(1)
	{
		char msg[maxDataSize];
		scanf("%s",msg);/*read message*/
		if(send(sock_fd,msg,strlen(msg),0)==-1)
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
			send(sock_fd,filename3,maxDataSize,0);/*send file name*/
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
		    		if(send(sock_fd,sdbuf,fs_block_sz,0)<0)
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
			close(sock_fd);
			exit(1);
		}
	}
	return 0;
}
