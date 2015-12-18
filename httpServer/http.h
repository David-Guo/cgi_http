#include <iostream>

using namespace std;
#define QLEN 30

class HttpServer {
    public :
        void boost(int port);
        int passivesock(int port);
        void error(const char *eroMsg);
        void handleRequest(int sockfd);

    private:
        int m_sock;
};
