#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <pthread.h>

/*define the most connect request you can handle sumultaneuosly*/
#define backlog 10
/*define the biggest data size you can handle*/
#define maxDataSize 1024

int sock_fd,new_fd;
pthread_t acceptThread,receiveThread;

/*function receive text*/
void receiveText(void)
{
	while(1)
	{
		int numBytes;
		char buf[maxDataSize];
		if((numBytes=recv(new_fd,buf,maxDataSize,0))==-1)
		{
			perror("recv");
			exit(1);
		}
		buf[numBytes]='\0';
		/*send file when you receive file*/
		if(strcmp(buf,"file")==0)
		{
			char filename4[1024];
			recv(new_fd,filename4,maxDataSize,0);/*receive file name*/
			FILE *fr=fopen(filename4,"w");/*write file*/
			printf("receiveing file from client ......\n");
			if(fr==NULL)
			{
				printf("file %s cannot be opened\n",filename4);
			}
			else
			{
				bzero(buf,maxDataSize);
				int fr_block_sz=0;
	    			while((fr_block_sz=recv(new_fd,buf,maxDataSize,0))>0)
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
	    			printf("received from client\n");
	    			fclose(fr);
			}
		}
		/*quit when you receive quit*/
		else if(strcmp(buf,"quit")==0)
		{
			printf("Client is closed\n");
			close(new_fd);
			close(sock_fd);
			exit(1);
		}
		/*print message*/
		else printf("error\n");
	}
}
/*accept client connect*/
void acceptConnect(void)
{
	struct sockaddr_in other_addr;
	int sin_size;
	sin_size =sizeof(struct sockaddr_in);
	if((new_fd=accept(sock_fd,(struct sockaddr *)&other_addr,&sin_size))==-1)
	{
		perror("accept");
		exit(1);
	}
	printf("server:get connection from %s\n",inet_ntoa(other_addr.sin_addr));
	/*create pthread*/
	if((pthread_create(&receiveThread,NULL,(void*)receiveText,NULL))!=0)
	{
		printf("create thread error\r\n");
		exit(1);
	}
}
int main(int argc,char* argv[])
{
	struct sockaddr_in my_addr;
	/*tell you to enter port*/
	if(argc<2)
	{
		fprintf(stderr,"usage:%s port\n",argv[0]);
		exit(1);
	}
	/*create socket*/
	if((sock_fd=socket(AF_INET,SOCK_STREAM,0))==-1)
	{
		perror("socket");
		exit(1);
	}
	/*initialize sockaddr_in*/
	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(atoi(argv[1]));
	my_addr.sin_addr.s_addr=INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	/*binding*/
	if(bind(sock_fd,(struct sockaddr*)&my_addr,sizeof(struct sockaddr))==-1)
	{
		perror("bind");
		exit(1);
	}
	/*start listening*/
	if(listen(sock_fd,backlog)==-1)
	{
		perror("listen");
		exit(1);
	}
	printf("listening......\n");
	/*create pthread*/
	if((pthread_create(&acceptThread,NULL,(void*)acceptConnect,NULL))!=0)
	{
		printf("create thread error\n");
		exit(1);
	}
	while(1)
	{
		char msg[maxDataSize];
		scanf("%s",msg);/*read message*/
		if(send(new_fd,msg,strlen(msg),0)==-1)
		{
			perror("send");
			exit(1);
		}
		/*send file when you type file*/
		if(strcmp(msg,"file")==0)
		{
			char filename[1024];
			printf("filename:");/*enter file name*/
			scanf("%s",filename);/*read file name*/
			send(new_fd,filename,maxDataSize,0);/*send file name*/
			char sdbuf[maxDataSize];
			printf("sending %s to client......\n",filename);
			FILE *fs=fopen(filename,"r");/*read file*/
			/*can not find file*/
			if(fs==NULL)
			{
				printf("file %s not found\n",filename);
				exit(1);
			}
			bzero(sdbuf,maxDataSize);
			int fs_block_sz;
			while((fs_block_sz=fread(sdbuf,sizeof(char),maxDataSize,fs))>0)
			{
		    		if(send(new_fd,sdbuf,fs_block_sz,0)<0)
		    		{
		        		fprintf(stderr,"ERROR:Failed to send file %s.(errno = %d)\n",filename,errno);
		        		break;
		    		}
		    		else bzero(sdbuf,maxDataSize);
			}
			printf("file %s from server was sent\n",filename);
			fclose(fs);
		}
		/*quit when you send quit*/
		if(strcmp(msg,"quit")==0)
		{
			printf("goodbye\n");
			close(new_fd);
			close(sock_fd);
			exit(1);
		}
	}
	return 0;
}
