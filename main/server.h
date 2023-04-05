#ifndef SERVER_H
#define SERVER_H

#include <cstddef>
#define TX_BUFSZ 128
void run_server(std::size_t (*cb)(char[TX_BUFSZ], char*, size_t));

#endif // SERVER_H
