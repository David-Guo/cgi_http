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

    resultHtml += "<script><document.all['m";
    resultHtml += to_string(taskId);
    resultHtml += "'].innerHTML += \"";
    resultHtml += addStr;
    resultHtml += "<br>\";<script>\n";
}


void Client::init(string ip, string port, string file, int id) {
    IP = ip;
    Port = port;
    resultHtml = "";
    taskId = id;
    inputFile = file;
    batFile.open(inputFile.c_str(), ios::in);
}

void Client::reciveFromServ(bool &isSend) {
    char buff[4096];
    memset(buff, 0, sizeof(char) * 4096);

    if (recv(sockfd, buff, 4096, 0) > 0) {
        string recvStr(buff);
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
        for (int i = cmd.size() - 1; i > 0; i++)
            if (cmd[i] == 13) cmd.pop_back();

        string wrapCmd = "% ";
        wrapCmd += "<b>";
        wrapCmd += cmd;
        wrapCmd += "</b>";
        saveToResult(wrapCmd);
        cerr << taskId << " " << cmd << " " << cmd.size() << endl;
        cmd += '\n';
        write(sockfd, cmd.c_str(), cmd.size());

        if (cmd.find("exit") != string::npos) return false;
        else 
            return true;
    }
    else 
        return false;
}
