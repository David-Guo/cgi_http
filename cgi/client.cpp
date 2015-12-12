#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sstream>
#include <unistd.h>
#include <string>
#include "client.h"

std::string substitute(std::string str, const std::string& src, const std::string& dest) {
    size_t start = 0;
    while ((start = str.find(src, start) != std::string::npos)) {
        str.replace(start, src.length(), dest);
        start += src.length();
    }
    return str;
}

void Client::saveToResult(string addStr) {
    while(addStr != "" && addStr[addStr.size() - 1] == 13) 
        addStr.pop_back();

    resultHtml += "<script>document.all['m";
    resultHtml += to_string(taskId);
    resultHtml += "'].innerHTML += \"";
    resultHtml += addStr;
    resultHtml += "<br>\";</script>\n";
}


void Client::init(string ip, string port, string file, int id) {
    IP = ip;
    Port = port;
    resultHtml = "";
    taskId = id;
    inputFile = file;
    batFile.open(inputFile.c_str(), ios::in);
}


string Client::returnRsltHtml() {
    string retrunStr = resultHtml;
    resultHtml = "";
    return retrunStr;
}


void Client::reciveFromServ(bool &isSend) {
    char buff[4096];
    memset(buff, 0, sizeof(char) * 4096);

    if (recv(sockfd, buff, 4096, 0) > 0) {
        string recvStr(buff);
        // 从server 取到输出结果，遇到 % 证明，可以向server 发送信息了
        if (recvStr.find('%') != string::npos) isSend = true;

        stringstream ss(recvStr);
        string tempStr;

        // 以换行为分隔符
        while (getline(ss, tempStr, '\n')) {
            tempStr = substitute(tempStr, "\"", "&quot");
            tempStr = substitute(tempStr, ">", "&qt");
            tempStr = substitute(tempStr, "<", "&lt");
            tempStr = substitute(tempStr, "%", "");
            tempStr = substitute(tempStr, "\r", "");
            tempStr = substitute(tempStr, "\n", "");

            if (tempStr != " ") {
                saveToResult(tempStr);
            }
        }

    }
}

bool Client::sendToServ() {
    string cmd = "";

    if (getline(batFile, cmd)) {
        for (int i = cmd.size() - 1; i > 0; i--)
            if (cmd[i] == 13) cmd.pop_back();
        
        // 粗体打包好从batFile读取的指令
        string wrapCmd = "% ";
        wrapCmd += "<b>";
        wrapCmd += cmd;
        wrapCmd += "</b>";
        saveToResult(wrapCmd);
        // 终端查看读取的文件信息
        cerr << taskId << " " << cmd << " " << cmd.size() << endl;
        cmd += '\n';
        // 将指令通过socket 发送到server
        write(sockfd, cmd.c_str(), cmd.size());

        if (cmd.find("exit") != string::npos) return false;
        else 
            return true;
    }
    else 
        return false;
}
