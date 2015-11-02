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
 *   http://linux.die.net/man/3/getaddrinfo
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
/* port we're listening on */
#define PORT "4458"

#include "ConnectionsDoublyLinkedList.h"

void LineLocator(struct ConnectionNode *conn) {
	/* look for the end of the line */
	char *str1, *saveptr1, *ntoken, *token;
	bool endswell = false;
	switch (conn->buf[conn->nbytes - 1]) {
	case '\r':
	case '\n':
		endswell = true;
		break;
	};

	for (str1 = conn->buf;; str1 = NULL ) {
		ntoken = strtok_r(str1, "\r\n", &saveptr1);
		/* Reverse this so we know the next result. */
		if (str1 == NULL ) {
			if (ntoken != NULL || endswell) {
				char *str2, *saveptr2, *subtoken, *t = NULL;
				char **argv = NULL;
				while (argv == NULL )
					argv = malloc(0);
				int argc = 0;

				for (str2 = token;; str2 = NULL ) {
					subtoken = strtok_r(str2, " \t", &saveptr2);
					if (subtoken == NULL )
						break;
					void *i = NULL;
					while (i == NULL)
						i = realloc(argv, sizeof(void*) * (argc + 1));
					argv = i;
					argv[argc++] = subtoken;
					if (t != NULL ) {
						size_t n = strlen(t) + strlen(subtoken) + 2;
						i = NULL;
						while (i == NULL )
							i = realloc(t, n);
						t = i;
						strncat(t, ",", n);
						strncat(t, subtoken, n);
					} else {
						{
							size_t n = strlen(subtoken) + 2;
							t = NULL;
							while (t == NULL )
								t = malloc(n);
							strncpy(t, subtoken, n);
						}
					}
				}
				strcat(t, "!\n");
				struct ConnectionNode *dest;
				for (dest = conn->next; dest != conn; dest = dest->next) {
					printf("socket send to %s on socket %d index %d\n",
							dest->host, dest->fd, dest->index);
					if (send(dest->fd, t, strlen(t), 0) == -1)
						perror("Negative send");
				}
				free(t);
				t = NULL;
				if (ntoken == NULL ) {
					free(conn->buf);
					conn->buf = NULL;
					conn->nbytes = 0;
					break;
				}
			} else {
				conn->nbytes = strlen(token);
				str1 = strdup(token);
				free(conn->buf);
				conn->buf = str1;
				break;
			}
		}
		token = ntoken;
	}
}

void ProccessInput(struct ConnectionNode *conn, char *buf, size_t nbytes) {
	/* we got some data from a client */
	conn->nbytes += nbytes;
	printf("socket recv from %s on socket %d index %d\n", conn->host, conn->fd,
			conn->index);
	if (strnlen(buf, nbytes) != nbytes) {
		/* this would be a crash */
		printf("socket to %s send non-string\n", conn->host);
		char e[42];
		sprintf(e, "551 Invalid string %4d bytes discarded\r\n",
				(unsigned int) conn->nbytes);
		write(conn->fd, &e, 41);
		if (conn->buf)
			free(conn->buf);
		conn->buf = NULL;
		conn->nbytes = 0;
	} else {
		if (conn->buf != NULL ) {
			char *t = NULL;
			while (t == NULL )
				t = malloc(conn->nbytes);
			strncpy(t, conn->buf, conn->nbytes);
			strncat(t, buf, conn->nbytes);
			free(conn->buf);
			conn->buf = t;
		} else
			conn->buf = strdup(buf);
		LineLocator(conn);
	}
}

int main(int argc, char *argv[]) {
	/* master file descriptor list */
	fd_set master;
	/* temp file descriptor list for select() */
	fd_set read_fds;
	/* server address */
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	/* maximum file descriptor number */
	int fdmax;
	/* listening socket descriptor */
	int listener;
	/* for setsockopt() SO_REUSEADDR, below */
	int yes = 1;
	int j;

	/* clear the master and temp sets */
	FD_ZERO(&master);
	FD_ZERO(&read_fds);

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC; /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
	hints.ai_flags = AI_PASSIVE;

	j = getaddrinfo(NULL, PORT, &hints, &result);
	if (j != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(j));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	 Try each address until we successfully bind(2).
	 If socket(2) (or bind(2)) fails, we (close the socket
	 and) try the next address. */

	for (rp = result; rp != NULL ; rp = rp->ai_next) {
		listener = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (listener == -1)
			continue;

		/*"address already in use" error message */
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
				== -1)
			goto tryagain;

#ifdef SO_REUSEPORT
		if (setsockopt(listener, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(int))
				== -1)
			goto tryagain;
#endif

		if (bind(listener, rp->ai_addr, rp->ai_addrlen) == 0)
			break; /* Success */

		tryagain: close(listener);
	}

	if (rp == NULL ) { /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}
	freeaddrinfo(result); /* No longer needed */

	/* listen */
	if (listen(listener, SOMAXCONN) == -1) {
		perror("Error opening listener");
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

		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL ) == -1) {
			perror("Error waiting for input");
			exit(1);
		}

		if (FD_ISSET(listener, &read_fds)) { /* we got a new one... */
			/* handle new connections */
			struct ConnectionNode *TempNode = GetNewConnection();
			TempNode->addr_len = sizeof(TempNode->addr);
			if ((TempNode->fd = accept(listener,
					(struct sockaddr *) &TempNode->addr, &TempNode->addr_len))
					== -1) {
				perror("Warning accepting one new connection");
				free(TempNode);
			} else {
				FD_SET(TempNode->fd, &master);
				/* add to master set */
				fdmax = TempNode->fd;
				fdmax = TempNode->fd;

				j = getnameinfo((struct sockaddr *) &TempNode->addr,
						TempNode->addr_len, TempNode->host, NI_MAXHOST, NULL, 0,
						0);

				InsertConnectionBefore(&connections_head, TempNode);
				printf("%s: New connection from %s on socket %d index %d\n",
						argv[0], TempNode->host, TempNode->fd, TempNode->index);
			}
		}

		/* run through the existing connections looking for data to be read */
		struct ConnectionNode *i = connections_head;
		if (connections_head != NULL )
			do {
				if (FD_ISSET(i->fd, &read_fds)) { /* we got one... */
					/* handle data from a client */
					printf("%s: New data from %s on socket %d index %d\n",
							argv[0], i->host, i->fd, i->index);
					/* buffer for client data */
					char buf[1024];
					int nbytes;
					if ((nbytes = recv(i->fd, buf, sizeof(buf) - 1, 0)) <= 0) {
						/* got error or connection closed by client */
						if (nbytes == 0)
							/* connection closed */
							printf(
									"%s: socket to %s hung up on socket %d index %d\n",
									argv[0], i->host, i->fd, i->index);
						else
							perror("Negative recv");
						/* close it... */
						close(i->fd);
						/* remove from master set */
						FD_CLR(i->fd, &master);
						/* step back and remove this connection */
						i = RemoveConnection(i);
						if (i == NULL )
							break;
					} else {
						/* Ensure this is an ansi string */
						buf[nbytes] = '\0';
						ProccessInput(i, buf, nbytes);
					}
				}
				i = i->next;
			} while (i != connections_head);
	}
	return 0;
}
