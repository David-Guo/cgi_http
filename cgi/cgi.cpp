#include <iostream>
#include <stdlib.h>
#include <assert.h>
#include <vector>
#include <sstream>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "client.h"
using namespace std;
#define MAXUSER 5


void decodeEnv(string sourceString, vector<string>& vDest) {
    assert(sourceString != "");
    stringstream source(sourceString);
    string splited;
    while(getline(source, splited, '&'))
        vDest.push_back(splited);
}

int main() {
    int clieFd[MAXUSER] = {-1, -1, -1, -1, -1};
    struct hostent *host[MAXUSER];
    int hostPort[MAXUSER];
    struct sockaddr_in serv_sin[MAXUSER];
    fd_set rfds;
    fd_set afds;


    setenv("QUERY_STRING", "h1=127.0.0.1&p1=12345&f1=t1.txt", 1);
    string queryString(getenv("QUERY_STRING"));

    vector<string> vDecodeEnv;
    decodeEnv(queryString, vDecodeEnv);

    Client * pClient = new Client[MAXUSER];

    // 初始化pClient
    for(size_t i = 0; i < vDecodeEnv.size(); i += 3) {
        int num = i/3;

        if(vDecodeEnv[i].find_first_of('=') == vDecodeEnv[i].size() - 1){
            pClient[num].init("", "", "", num);
            clieFd[num] = -1;
            continue;
        }
        string serverIP = vDecodeEnv[i].substr(vDecodeEnv[i].find_first_of('=') + 1);
        string serverPort = vDecodeEnv[i + 1].substr(vDecodeEnv[i + 1].find_first_of('=') + 1);
        string serverFile = vDecodeEnv[i + 2].substr(vDecodeEnv[i + 2].find_first_of('=') + 1);
        clieFd[num] = socket(AF_INET, SOCK_STREAM, 0);
        host[num] = gethostbyname(serverIP.c_str());
        hostPort[num] = atoi(serverPort.c_str());

        // 初始化client[num] 的成员变量
        pClient[num].init(serverIP, serverPort, serverFile, num);
        pClient[num].sockfd = clieFd[num];
    }

    // 初始化 serv_sin
    for(int i = 0; i < MAXUSER; i++) {
        if(clieFd[i] != -1) {
            memset(&serv_sin[i], 0, sizeof(serv_sin[i]));
            serv_sin[i].sin_family = AF_INET;
            serv_sin[i].sin_addr = *((struct in_addr *)host[i]->h_addr);
            serv_sin[i].sin_port = htons(hostPort[i]);
        }
    }

    // 输出 html head
    cout << "<html>" << endl
        << "<head>" << endl
        << "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=big5\" />" << endl
        << "<title>Network Programming Homework 3</title>" << endl
        << "</head>" << endl
        << "<body bgcolor=#336699>" << endl
        << "<font face=\"Courier New\" size=2 color=#FFFF99>" << endl
        << "<table width=\"800\" border=\"1\">" << endl
        << "<tr>";

    // 通过JavaScript利用`id` 属性来改变表格内容
    for (int i = 0; i < MAXUSER; i++) {
        if (pClient[i].IP == "" || pClient[i].Port == "")
            continue;
        else
            cout << "<td valign=\"top\" id =\"m" << i << "\"></td>";
    }
    cout << "</tr></table>" << endl;

    FD_ZERO(&rfds);
    FD_ZERO(&afds);
    int maxFd = 0;

    for (int i = 0; i < MAXUSER; i++) {
        if (clieFd[i] != -1) {
            int flags;
            // 设置 sockfd 为 nonblock
            if ((flags = fcntl(clieFd[i], F_GETFL, 0)) == -1)
                flags = 0;
            fcntl(clieFd[i], F_SETFL, flags | O_NONBLOCK);

            if (clieFd[i] > maxFd) maxFd = clieFd[i];
            // 与ras 服务建立 socket 链接
            while(1) {
                if (connect(clieFd[i], (struct sockaddr *)&serv_sin[i], sizeof(serv_sin[i])) == -1) 
                    ;
                else 
                    break;
            }
            FD_SET(clieFd[i], &afds);
        }
    }

    bool isSend[5] = {false, false, false, false, false};

    while(1) {
        memcpy(&rfds, &afds, sizeof(fd_set));
        if (select(maxFd + 1, &rfds, NULL, NULL, NULL) < 0) {
            cerr << "select failed" << endl;
            return 0;
        }
        // 轮询各个serv
        for (int i = 0; i < MAXUSER; i++) {
            if (clieFd[i] == -1) 
                continue;
            if (FD_ISSET(clieFd[i], &rfds)) {
                // recive 到 % 号说明可以向serv 发送指令了，isSend = ture
                pClient[i].reciveFromServ(isSend[i]);
                cout << pClient[i].returnRsltHtml() << endl;
            }

            if (isSend[i]) {
                bool isExit = pClient[i].sendToServ() == false;
                sleep(1);
                if (isExit) {
                    sleep(4);
                    pClient[i].reciveFromServ(isSend[i]);
                    cout << pClient[i].resultHtml << endl;

                    // close sockfd，将不会再被select
                    close(clieFd[i]);
                    FD_CLR(clieFd[i], &afds);
                    clieFd[i] = -1;
                }
            }
        }
        bool isFinish = true;
        for (int i = 0; i < MAXUSER; i++)
            if (clieFd[i] != -1) {
                isFinish = false;
                break;
            }
        if (isFinish) goto finished;

    }
finished:
    cout << "</font>"
        << "</body>"
        << "</html>" << endl;
    delete[] pClient;
    return 0;
}
