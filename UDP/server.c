#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>

/*define the biggest data size you can handle*/
#define maxDataSize 1024

#define ERR_EXIT(m) \
    do { \
        perror(m); \
        exit(EXIT_FAILURE); \
    } while (0)

void echo_ser(int sock)
{

	struct sockaddr_in peeraddr;
    	socklen_t peerlen;

/*    	char recvbuf[1024] = {0};
    	struct sockaddr_in peeraddr;
    	socklen_t peerlen;
	int n;

    	while (1)
    	{
        	peerlen = sizeof(peeraddr);
        	memset(recvbuf, 0, sizeof(recvbuf));
        	n = recvfrom(sock, recvbuf, sizeof(recvbuf), 0,(struct sockaddr *)&peeraddr, &peerlen);
        	if (n == -1)
        	{
            		if (errno == EINTR)
                	continue;
            		ERR_EXIT("recvfrom error");
        	}
        	else if(n > 0)
        	{
            		fputs(recvbuf, stdout);
            		sendto(sock, recvbuf, n, 0,(struct sockaddr *)&peeraddr, peerlen);
        	}
    	}
    	close(sock);*/

	while(1)
	{
		int numBytes;
		char buf[maxDataSize];
		if((numBytes=recvfrom(sock,buf,maxDataSize,0,(struct sockaddr *)&peeraddr, &peerlen))==-1)
		{
			perror("recv");
			exit(1);
		}
		buf[numBytes]='\0';
		/*send file when you receive file*/
		if(strcmp(buf,"file")==0)
		{
			char filename4[1024];
			recvfrom(sock,filename4,maxDataSize,0,(struct sockaddr *)&peeraddr, &peerlen);/*receive file name*/
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
	    			while((fr_block_sz=recvfrom(sock,buf,maxDataSize,0,(struct sockaddr *)&peeraddr, &peerlen))>0)
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
			close(sock);
			exit(1);
		}
		/*print message*/
		else printf("error\n");
	}

}

int main(int argc,char *argv[])
{
    	int sock;

	/*tell you to enter port*/
	if(argc<2)
	{
		fprintf(stderr,"usage:%s port\n",argv[0]);
		exit(1);
	}

    	if ((sock = socket(PF_INET, SOCK_DGRAM, 0)) < 0)
        ERR_EXIT("socket error");

    	struct sockaddr_in servaddr;
    	memset(&servaddr, 0, sizeof(servaddr));
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_port = htons(atoi(argv[1]));
    	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        ERR_EXIT("bind error");

    	echo_ser(sock);

    	return 0;
}
