/* This page contains a client program that can request a file from the server program
 * on the next page. The server responds by sending the whole file.
 */

#include "file-server.h"
#include <arpa/inet.h>  //for inet_ntoa()
#include <sys/fcntl.h>
#define DEBUG 1
/*change this to "yes"/"no" for the opposite effect in write mode
default is "yes" */
#define doesServerHaveFile "yes"
/* change the createFileName to something else to change the file */
#define MAKEFILENAME "ClientOutput.txt"
/* If you want to read a file from the server then save 
*it's contents to a file of the same name
leave this as 1, else leave it as 0 */
#define outputWrite 0


int main(int argc, char **argv)
{
	int c, s, g, bytes, fd;
  	FILE *fp; /*to create file*/
	int size=0;
	char buf[BUF_SIZE];		/* buffer for incoming file */
	char *startFlag = "-s";
	char *endFlag = "-e";
	char *writeFlag = "[-w]";
	char *writeNothing = "-1";
	struct hostent *h;		/* info about server */
	struct sockaddr_in channel;		/* holds IP address */
	char *createFileName = MAKEFILENAME;
	//char *ipaddress;              /* the ip of the client */

	/* User doesnt enter enough arguments*/
	if (argc < 3) 
	{
		printf("Usage: client server-name file-name\n");
		exit(0);
	}
	
	/* User didn't put in the write flag correctly */
	else if (argc == 4 && strcmp(argv[2],writeFlag) != 0)
	{
		printf("Usage: client server-name [-w] file-name\n");
		exit(0);
	}
	/* User didn't put the -s or -e flag correctly */
	else if (argc >= 6 && (strcmp (argv[2],startFlag) != 0 || strcmp (argv[4],endFlag) ) )
	{
		printf("Usage: client server-name -s byte1 -e byte2 file-name\n");
		exit(0);
	}
	
	/*Go ahead and try to connect to the server*/
	else
	{
		h = gethostbyname(argv[1]);		/* look up host's IP address */
		if (!h)
			{
			printf("Could not get the host's IP address...\n %s is an incorrect server.",argv[1]);
			exit(0);
			}

		s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (s <0)
		{
			printf("Unable to make the socket, exiting...\n");
		}
  
		memset(&channel, 0, sizeof(channel));
		channel.sin_family= AF_INET;
		memcpy(&channel.sin_addr.s_addr, h->h_addr, h->h_length);
		channel.sin_port= htons(SERVER_PORT);
		c = connect(s, (struct sockaddr *) &channel, sizeof(channel));
		if (c < 0)
		{
			printf("Could not connect to the server...\n");
			exit(0);
		}
		
		/* Check if the [-w] flag is present */
		if (strcmp(argv[2],writeFlag) == 0)
		{
			//printf("We got a write flag!\nSending it to server now...\n");
			write(s,argv[2],strlen(argv[2])+1);
			//printf("Now we gotta send the file name...which is %s\n",argv[3]);
			write(s,argv[3],strlen(argv[3])+1);
			read(s, buf, BUF_SIZE);
			//printf("Does the server have that file already? %s\n",buf);
			if (strcmp(buf,doesServerHaveFile) == 0) //change to yes/no for opposite. default = yes
			{
				if (DEBUG)
					printf("The server already has the <%s> file.\nWriting is not possible.\n",argv[3]);
				exit(0);
			}
			/* If the file isn't on the server, we can create one and write to it */
			else
			{
				//printf("We can write now...\n");
				fd = open(argv[3], O_RDONLY);
				if (fd < 0) 
				{
					if (DEBUG)
						printf("We could not open %s because it doesn't exist in the client's directory.\n",argv[3]);
					exit(0);
				}
				else
				{
					bytes = read(fd, buf, BUF_SIZE);	/* read from file */
					//printf("We got our bytes of %d\n",bytes);
					if (bytes <= 0)
					{
						//printf("Writing is done. Exiting now. \n");
						close(fd);
						exit(0);
					}
					/* Writes the file's bytes to the server */
					write(s, buf, bytes);
				}
			}
		}
		/* Write flag not found, we're reading files from server */
		else
		{
			/* Connection is now established. Send file name including 0 byte at end. */
			if (argc >= 6)
			{
				write(s, argv[6], strlen(argv[6]) + 1);
				if (outputWrite == 1)
					createFileName = argv[6];
			}
			else
			{
				write(s, argv[2], strlen(argv[2]) + 1);
				if (outputWrite == 1)
					createFileName = argv[2];
			}
			/* Get the Ok from the server */
			read(s, buf, BUF_SIZE);
			
			if (strcmp(buf,"File does not exist on server.") == 0)
			{
				if (DEBUG)
					printf("Error with file.\n%s\nExiting...\n",buf);
				exit(0);
			}
			/* This should only happen with start/end byte flags */
			if (argc >= 6)
			{	
				if (atoi(argv[3]) >= atoi(argv[5]))
				{
					if (DEBUG)
						printf("Your start byte %d can't be greater than or equal to your ending byte %d.\nExiting...\n",atoi(argv[3]),atoi(argv[5]));
					exit(0);
				}
				if (atoi(argv[3]) <= 0 || atoi(argv[5]) <= 0)
				{
					if (DEBUG)
						printf("Your start byte %d and end byte %d must be > 0\n",atoi(argv[3]),atoi(argv[5]));
					exit(0);
				}	
				/*Open file in Read Mode*/
				fp=fopen(argv[6],"r");
				/*Move file point at the end of file.*/
				fseek(fp,0,SEEK_END);
				/*Get the current position of the file pointer.*/
				size=ftell(fp);
				if (atoi(argv[3]) > size || atoi(argv[5]) > size)
				{
					if (DEBUG)
						printf("Your start byte %d or end byte %d are greater than the file size %d bytes.\n",atoi(argv[3]),atoi(argv[5]),size);
					exit(0);
				}
				//if (strcmp(argv[3],startFlag) == 0) printf("The start flag %s is found!\n",argv[3]);
				if (argv[3] != NULL)
				{
					//printf("We sent %s as the start byte to the server\n",argv[4]);
					write(s, argv[3], strlen(argv[3])+1);
				}
					
				//if (strcmp(argv[5],endFlag) == 0) printf("The end flag %s is found!\n",argv[5]);
				
				if (argv[5] != NULL)
				{
					//printf("We sent %s as the end byte to the server\n",argv[6]);
					write(s, argv[5], strlen(argv[5])+1);
				}  
			}
			/* There are no starting and ending flags, just send -1 to the server */
			else
			{
				write(s, writeNothing, strlen(writeNothing)+1);  
				write(s, writeNothing, strlen(writeNothing)+1); 
			}
			/* Open the file to write to, if it's not there, make one */
			fd = open(createFileName, O_WRONLY | O_CREAT, 0644); 
			/* Go get the file and write it to the file specified in define. */
			while (1) 
			{
				bytes = read(s, buf, BUF_SIZE);	/* read from socket */
				//if (bytes > 0 ) printf("The file's bytes are %d and the buff size is %d\n",bytes,BUF_SIZE);
				if (bytes <= 0)/* check for end of file */
				{ 
					//printf("\n");
					exit(0);
				}		
				write(fd, buf, bytes);		/* write to the file */
			}
		}
	}
}

