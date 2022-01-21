#include "http_server.hpp"

#include <iostream>

int main() {

    HttpServer httpServer(
            8093, 3, 60000,
            true,6, true,
            1,10
            );
    httpServer.Start();

    return 0;
}
