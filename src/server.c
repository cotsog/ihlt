/*  ihlt hopefully the last tracker: seed your network.
 *  Copyright (C) 5015  Michael Mestnik <cheako+github_com@mikemestnik.net>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* Example from:
 *   http://www.tenouk.com/Module41.html
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* port we're listening on phone coded msg */
#define PORT 4458

int main(int argc, char *argv[]) {
	/* master file descriptor list */
	fd_set master;
	/* temp file descriptor list for select() */
	fd_set read_fds;
	/* server address */
	struct sockaddr_in serveraddr;
	/* maximum file descriptor number */
	int fdmax;
	/* listening socket descriptor */
	int listener;
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	int addrlen;

	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	/* get the listener */
	if ((listener = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating listener");
		exit(1);
	}

	/* I.E. "address already in use" error message */
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1) {
		perror("Error setting address reusable");
		exit(1);
	}
#ifdef SO_REUSEPORT
	if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int))) {
		perror("Error setting port reusable");
		exit(1);
	}
#endif

	/* bind */
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = INADDR_ANY;
	serveraddr.sin_port = htons(PORT);
	memset(&(serveraddr.sin_zero), '\0', 8);

	if (bind(listener, (struct sockaddr *) &serveraddr, sizeof(serveraddr))
			== -1) {
		perror("Error opening listener");
		exit(1);
	}

	/* listen */
	if (listen(listener, 10) == -1) {
		perror("Error listening");
		exit(1);
	}

	/* add the listener to the master set */
	FD_SET(listener, &master);
	/* keep track of the biggest file descriptor */
	fdmax = listener; /* so far, it's this one */

	/* loop */
	for (;;) {
		/* copy it */
		read_fds = master;

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("Error waiting for input");
			exit(1);
		}

		/* run through the existing connections looking for data to be read */
		int i;
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { /* we got one... */
				if (i == listener) {
					/* handle new connections */
					/* client address */
					struct sockaddr_in clientaddr;
					/* newly accept()ed socket descriptor */
					int newfd;
					addrlen = sizeof(clientaddr);
					if ((newfd = accept(listener,
							(struct sockaddr *) &clientaddr, &addrlen)) == -1) {
						perror("Warning accepting one new connection");
					} else {
						FD_SET(newfd, &master);
						/* add to master set */
						if (newfd > fdmax) /* keep track of the maximum */
							fdmax = newfd;
						printf("%s: New connection from %s on socket %d\n",
								argv[0], inet_ntoa(clientaddr.sin_addr), newfd);
					}
				} else {
					/* handle data from a client */
					/* buffer for client data */
					char buf[1024];
					int nbytes;
					if ((nbytes = recv(i, buf, sizeof(buf), 0)) <= 0) {
						/* got error or connection closed by client */
						if (nbytes == 0)
							/* connection closed */
							printf("%s: socket %d hung up\n", argv[0], i);
						else
							perror("Negative recv");
						/* close it... */
						close(i);
						/* remove from master set */
						FD_CLR(i, &master);
					} else {
						/* we got some data from a client */
						int j;
						for (j = 0; j <= fdmax; j++) {
							/* send to everyone! */
							if (FD_ISSET(j, &master)) {
								/* except the listener and ourselves */
								if (j != listener && j != i)
									if (send(j, buf, nbytes, 0) == -1)
										perror("send() error lol!");
							}
						}
					}
				}
			}
		}
	}
	return 0;
}
