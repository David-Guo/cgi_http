#include <iostream>
#include <fstream>

using namespace std;

class Client {
    public:
        string IP;
        string Port;
        string inputFile;
        fstream batFile;
        int taskId;
        int sockfd;
        string resultHtml;

    public:
        void init(string ip, string port, string file, int id);
        void reciveFromServ(bool &isSend);
        bool sendToServ();
        void saveToResult(string addStr);
        string returnRsltHtml();
};
