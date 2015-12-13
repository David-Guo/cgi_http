#include "http.h"
#include <stdlib.h>


int main(int argc, char* argv[]) {

    if (argc != 2) {
        cerr << "Usage : http [port num]" << endl;
        exit(1);
    }
    HttpServer httpServer;
    httpServer.boost(atoi(argv[1]));

    return 0;
}
