/* This is the server code */
#include "file-server.h"
#include <sys/fcntl.h>
#include <arpa/inet.h>  //for inet_ntoa()

#define QUEUE_SIZE 10
#define DEBUG 1
#define MAKEFILENAME "ServerOutput.txt"


void getHere();


int main(int argc, char *argv[])
{	
	int s, b, l, fd, sa, bytes, on = 1;
	char buf[BUF_SIZE];		/* buffer for outgoing file */
	struct sockaddr_in channel;		/* holds IP address */
	struct sockaddr_in clientIP;		/* holds IP address */
	char *ipaddress;              /* the ip of the client */
	int startByte = -1;
	int endByte = -1;
	int writeMode = -1;
	char *writeFlag="[-w]";
	char *createFileName = MAKEFILENAME;
	int filePercent = 1;
	char fileName[1024];
	char *fileOpeningError = "File does not exist on server.";

	/* Build address structure to bind to socket. */
	memset(&channel, 0, sizeof(channel));	/* zero channel */
	channel.sin_family = AF_INET;
	channel.sin_addr.s_addr = htonl(INADDR_ANY);
	channel.sin_port = htons(SERVER_PORT);

	/* Passive open. Wait for connection. */
	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);    /* create socket */
	if (s < 0) fatal("socket failed");
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));

	b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
	if (b < 0) fatal("bind failed");

	l = listen(s, QUEUE_SIZE);		/* specify queue size */
	if (l < 0) fatal("listen failed");
  

	/* Socket is now set up and bound. Wait for connection and process it. */
	while (1) 
	{
		//printf("----------INCOMING REQUEST-------\n\n");
        int sizeofChannel = sizeof(channel);
        sa = accept(s, (struct sockaddr *)&channel, &sizeofChannel );		/* block for connection request */
        if (sa < 0)
		{
			printf("accept failed\n");
			close(sa);
			continue;
		}

        read(sa, buf, BUF_SIZE);		/* read file name or flag from socket */
        //printf("We have read %s!\n",buf);
		int writeMode = !strcmp(buf,writeFlag);
		/* Enter the write mode only if we get the write flag from the client */
        if (writeMode)
        {
			//printf("We writing and reading bois!\n");
			read(sa, buf, BUF_SIZE);		/* read file name socket */
			strcpy(fileName,buf);
			//printf("We have the file %s now\n",fileName);
			fd = open(fileName, O_RDONLY); 
			if (fd < 0)
			{
				printf("We don't have the file. %s does not exist\n",fileName);
				write(sa, "no", strlen("no") + 1);
			}
			else
			{
				//printf("We have the file.\n");
				write(sa, "yes", strlen("yes") + 1);
			}
			/*change fileName to a string have it write to a different file */
			fd = open("yeye", O_WRONLY | O_CREAT, 0644);
			/* This writes to the file specified in the define */
		  	while(1)
			{
				bytes = read(sa, buf, BUF_SIZE);	/* read from socket */
				//if (bytes > 0 ) printf("The file's bytes are %d and the buff size is %d\n",bytes,BUF_SIZE);
				if (bytes <= 0)/* check for end of file */
				{
				//printf("\n");
				break;
				}		
				write(fd, buf, bytes);		/* write to file */
			}
        }
		/* We're doing a read from the server */
		else
		{
			/* Get and return the file. */
			fd = open(buf, O_RDONLY);	/* open the file to be sent back */
			if (fd < 0)
			{
				//printf("%s\n",fileOpeningError);
				write(sa, fileOpeningError, strlen(fileOpeningError) + 1);	
				continue;
			}
			else
			{
				//printf("%s was able to open!\n",buf);
				write(sa, "ok", strlen("ok") + 1);
			}
			ipaddress = inet_ntoa(channel.sin_addr); /*store the ip*/
			if (DEBUG == 1)
				printf("Sending <%s> to <%s> \n", buf,ipaddress);  
			/* Store the buf in the fileName */
			strcpy(fileName,buf);
			
			read(sa, buf, BUF_SIZE);		/* read byte start from socket */
			/*Set our start and end bytes that we read in from client */
			startByte = atoi(buf);
			//printf("We got start byte as %d\n",startByte);
			
			read(sa, buf, BUF_SIZE);		/* read byte end from socket */
			endByte = atoi(buf);
			//printf("We got end byte as %d\n",endByte);
        
			/* This basically skips ahead in the file, setting the pointer to the start of where we want it */
			if (startByte >= 1)
				read(fd, buf, startByte-1);	/* read from file */ 
			
			while (1) 
			{
                bytes = read(fd, buf, BUF_SIZE);	/* read from file */
                //printf("We have read a total of %d bytes\n",bytes);
                if (bytes <= 0) break;		/* check for end of file */
                if (DEBUG == 1)
                {
					if (bytes - BUF_SIZE >= 0 && filePercent <= 9)
					{
						printf("Sent %d%% of <%s>\n",filePercent*10,fileName);
						filePercent++;
					}
					if (bytes - BUF_SIZE <= 0)
					{
						while (filePercent <= 9)
						{
							printf("Sent %d\%% of <%s>\n",filePercent*10,fileName);
							filePercent++;
						}
					printf("Finished sending <%s> to <%s>\n",fileName,ipaddress);   
					}
                }
				/* only do this if we have our endByte enabled, 
				just keep writing to the file with a size limit of (endByte-1) */
				if (endByte >= 1)
					write(sa, buf, endByte-1);		/* write bytes to socket */
				else 
					write(sa, buf, bytes);		/* write bytes to socket */
        }
        /*printf("We got to the end of the file and connection close\n");*/
        /* reset the filePercent, startByte, writeMode, bytes, and endByte */
        filePercent = 1; 
        startByte = -1;
        endByte = -1;
		writeMode = -1;
		bytes = 0;
        close(fd);			/* close file */
        close(sa);			/* close connection */
	}
  }
}

void getHere()
{
  /*This is just a test print statement I have*/
  printf("Did we ever get here?\n");
  return;
}  