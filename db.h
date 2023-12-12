#ifndef CONNECTIONS_H
#define CONNECTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>

#define DATABASE_FILE "connections.db"
#define MAX_CONNECTIONS 1000

struct ConnectionState {
    uint32_t source_ip;
    uint16_t source_port;
    uint32_t destination_ip;
    uint16_t destination_port;
    struct ConnectionState* next;
};

extern sqlite3* db;

void init_database();
void insert_connection(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port);
int connection_exists(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port);
void delete_connection(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port);
void cleanup_database();

#endif /* CONNECTIONS_H */