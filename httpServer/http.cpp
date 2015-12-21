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
#include <string>
#include <sstream>

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


void HttpServer::boost(int port) {
    struct sockaddr_in client_addr;
    socklen_t alen;
    alen = sizeof(client_addr);

    m_sock = passivesock(port);

    while(1) {
        int newsockfd = accept(m_sock, (struct sockaddr *) &client_addr, &alen);
        if (newsockfd < 0) {
            close(newsockfd);
            continue;
        }

        int childpid = fork();
        if (childpid  < 0)
            cerr << "fork failed" << endl;
        else if (childpid == 0) {
            // child process
            handleRequest(newsockfd);
            
            if(m_sock) close(m_sock);
            exit(0);
        }
        else {
            // parent process
            // do nothing
            close(newsockfd);
        }
    }
}


void HttpServer::handleRequest(int sockfd) {
    int tmpStdin = dup(0);
    int tmpStdout = dup(1);
    dup2(sockfd, 0);
    dup2(sockfd, 1);

    char buff[1024];

    // 解析协议 得到querystring 环境变量跟执行程序名
    string recvMsg;
    string queryString;
    string requestDoc;
    if (recv(sockfd, buff, 1023, 0) > 0) {
        recvMsg = string(buff);
        stringstream ss(recvMsg);
        string requestLine;
        string offset;
        // 直接在浏览器查看recvMsg cout << recvMsg << endl;
        // 直接在终端查看recvMsg cerr << recvMsg << endl;
        
        // 取出请求行
        getline(ss, requestLine, '\n');
        // cout << requestLine << endl;
        stringstream ssRqstLine(requestLine);

        if (requestLine.find(".cgi") != string::npos) {
            getline(ssRqstLine, offset, '/');
            getline(ssRqstLine, offset, '?');
            requestDoc = offset;
            getline(ssRqstLine, offset, ' ');
            queryString = offset;
            cerr << "requestDoc: " << requestDoc << endl;
            cerr << "queryString: " << queryString << endl;
            
        }

        // 响应信息
        cout << "HTTP/1.1 200 OK\r\n";
        cout << "response head\r\n";
        cout << "\r\n";
        if (requestLine.find(".cgi") != string::npos) {
            // 切换工作目录
            if (chdir("..") != 0) 
                error("Change working directory failed");

            // 改变环境变量！
            setenv("PATH", "./cgi:.", 1);
            setenv("QUERY_STRING", queryString.c_str(), 1);
            int status = execvp(requestDoc.c_str(), NULL);
            if (status == -1)
                error("execvp cgi failed");
        }
        else { //if (requestline.find(".htm") != string::npos) {
            // html 响应
            string htmlpage = string("<!doctype html>");
            htmlpage += "<html><head>404 not found : invalid web page requested.</head><body></body></html>";
        }    



    }
    else {
        cerr << "recv form sockfd failed" << endl;
        dup2(tmpStdin, 0);
        dup2(tmpStdout, 1);
        close(tmpStdin);
        close(tmpStdout);
        return;
    }

}
