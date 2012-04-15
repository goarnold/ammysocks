#include <stdio.h>
#include <stdlib.h>	/* needed for os x*/
#include <string.h>	/* for strlen */
#include <netdb.h>      /* for gethostbyname() */
#include <sys/socket.h>
#include <netinet/in.h>

#include "port.h"       /* defines default port */

int conn(char *host, int port);	/* connect to host,port; return socket */
void disconn(fd);	/* close a socket connection */
int debug = 1;

main(int argc, char **argv)
{
	extern char *optarg;
	extern int optind;
	int c, err = 0; 
	char *prompt = 0;
	int port = SERVICE_PORT;	/* default: whatever is in port.h */
	char *host = "localhost";	
	int fd;				/* file descriptor for socket */
	int MAXBUF = 30;
	int nbytes;
	static char usage[] = 
	"usage: %s [-d] [-h serverhost] [-p port]\n";
	
	while ((c = getopt(argc, argv, "dh:p:")) != -1)
		switch (c) {
			case 'h':  /* hostname */
				host = optarg;
				break;
			case 'p':  /* port number */
				port = atoi(optarg);
				if (port < 0 || port > 65535) {
					fprintf(stderr, "invalid port number: %s\n", optarg);
					err = 1;
				}
				break;
			case '?':
				err = 1;
				break;
		}
	if (err || (optind < argc)) {	/* error or extra arguments? */
		fprintf(stderr, usage, argv[0]);
		exit(1);
	}
	
	printf("connecting to %s, port %d\n", host, port);
	
	if ((fd = conn(host, port)) < 0)    /* connect */
		exit(1);   /* something went wrong */
	
	/* in a useful program, we would do something here involving reads and writes on fd */
	char buffer[MAXBUF];
	char prev[MAXBUF];
	char *stock = "aapl";
	char *endmessage = "&f=snl1 HTTP/1.0\n\n";
	char *frontmessage = "GET /d/quotes.csv?s=";
	char* stockmessage = malloc(strlen(stock) + strlen(endmessage) + strlen(frontmessage)+1);
	strcpy(stockmessage,frontmessage);
	strcat(stockmessage, stock);
	strcat(stockmessage, endmessage);
	printf(stockmessage);
	nbytes = write(fd, stockmessage, 42);	
	printf("write complete\n");
	//nbytes=read(fd,buffer,MAXBUF);
	while(read(fd,buffer,MAXBUF)>0)
	{
		printf("%s\n",buffer);
	}
	/*nbytes = read(fd, buffer, MAXBUF);
	printf("read complete\n");
	printf("%s\n",buffer);
	nbytes = read(fd, buffer, MAXBUF);
	printf("read complete\n");
	printf("%s\n",buffer);*/
		 /*write(2, "An error occurred in the read.\n", 31); *//* read up to MAXBUF bytes */
	disconn(fd);    /* disconnect */
	return 0;
}


/* conn: connect to the service running on host:port */
/* return -1 on failure, file descriptor for the socket on success */
int
conn(char *host, int port)
{
	struct hostent *hp;	/* host information */
	unsigned int alen;	/* address length when we get the port number */
	struct sockaddr_in myaddr;	/* our address */
	struct sockaddr_in servaddr;	/* server address */
	int fd;  /* fd is the file descriptor for the connected socket */
	
	if (debug) printf("conn(host=\"%s\", port=\"%d\")\n", host, port);
	
	/* get a tcp/ip socket */
	/* We do this as we did it for the server */
	/* request the Internet address protocol */
	/* and a reliable 2-way byte stream */
	
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("cannot create socket");
		return -1;
	}
	
	/* bind to an arbitrary return address */
	/* because this is the client side, we don't care about the */
	/* address since no application will connect here  --- */
	/* INADDR_ANY is the IP address and 0 is the socket */
	/* htonl converts a long integer (e.g. address) to a network */
	/* representation (agreed-upon byte ordering */
	
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(0);
	
	if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
		perror("bind failed");
		close(fd);
		return -1;
	}
	
	/* this part is for debugging only - get the port # that the operating */
	/* system allocated for us. */
	alen = sizeof(myaddr);
	if (getsockname(fd, (struct sockaddr *)&myaddr, &alen) < 0) {
		perror("getsockname failed");
		close(fd);
		return -1;
	}
	if (debug) printf("local port number = %d\n", ntohs(myaddr.sin_port));
	
	/* fill in the server's address and data */
	/* htons() converts a short integer to a network representation */
	
	memset((char*)&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	
	/* look up the address of the server given its name */
	hp = gethostbyname(host);
	if (!hp) {
		fprintf(stderr, "could not obtain address of %s\n", host);
		close(fd);
		return -1;
	}
	
	/* put the host's address into the server address structure */
	memcpy((void *)&servaddr.sin_addr, hp->h_addr_list[0], hp->h_length);
	
	/* connect to server */
	if (connect(fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("connect failed");
		close(fd);
		return -1;
	}
	if (debug) printf("connected socket = %d\n", fd);
	return fd;
}

/* disconnect from the service */
/* lame: we can just as easily do a shutdown() or close() ourselves */

void
disconn(int fd)
{
	if (debug) printf("disconn(%d)\n", fd);
	shutdown(fd, 2);    /* 2 means future sends & receives are disallowed */
}
