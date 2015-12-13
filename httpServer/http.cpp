#include "http.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

int HttpServer::passivesock(int port) {
    struct protoent *ppe;
    struct sockaddr_in serv_addr;
    if ((ppe = getprotobyname("tcp")) == 0)
        error("can't get prtocal entry");

    /* Open a TCP socker (an Internet stream socket). */
    int msock = socket(PF_INET, SOCK_STREAM, ppe->p_proto);
    if (msock == -1)
        error("Can't open socket");

    /* Bind our local address so that the client can send to us. */
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(port);

    if (bind(msock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
        error("Can't bind");
    printf("\t[Info] Binding...\n");

    if (listen(msock, QLEN) < 0)
        error("Can't listen");
    printf("\t[Info] Listening...\n");

    return msock;
}

void HttpServer::error(const char *eroMsg) {
    cerr << eroMsg << ":" << strerror(errno) << endl;
    exit(1);
}
