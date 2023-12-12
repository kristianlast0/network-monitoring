#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sqlite3.h>

char* DATABASE_FILE = "connections.db";
int32_t MAX_CONNECTIONS = 1000;
sqlite3* db;

void init_database() {
    int rc = sqlite3_open(DATABASE_FILE, &db);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    // Create the table if it doesn't exist
    const char* create_table_sql =
        "CREATE TABLE IF NOT EXISTS connections ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "source_ip INTEGER,"
        "source_port INTEGER,"
        "destination_ip INTEGER,"
        "destination_port INTEGER"
        ");";

    rc = sqlite3_exec(db, create_table_sql, 0, 0, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot create table: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

void insert_connection(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port) {
    const char* insert_sql =
        "INSERT INTO connections (source_ip, source_port, destination_ip, destination_port) "
        "VALUES (?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, insert_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_bind_int(stmt, 1, src_ip);
    sqlite3_bind_int(stmt, 2, src_port);
    sqlite3_bind_int(stmt, 3, dest_ip);
    sqlite3_bind_int(stmt, 4, dest_port);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Insert failed: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_finalize(stmt);
}

int connection_exists(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port) {
    const char* select_sql =
        "SELECT id FROM connections WHERE source_ip = ? AND source_port = ? AND destination_ip = ? AND destination_port = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, select_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_bind_int(stmt, 1, src_ip);
    sqlite3_bind_int(stmt, 2, src_port);
    sqlite3_bind_int(stmt, 3, dest_ip);
    sqlite3_bind_int(stmt, 4, dest_port);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Connection already exists
        sqlite3_finalize(stmt);
        return 1;
    } else if (rc == SQLITE_DONE) {
        // Connection does not exist
        sqlite3_finalize(stmt);
        return 0;
    } else {
        fprintf(stderr, "Select failed: %s\n", sqlite3_errmsg(db));
        exit(1);
    }
}

void delete_connection(uint32_t src_ip, uint16_t src_port, uint32_t dest_ip, uint16_t dest_port) {

    const char* delete_sql = "DELETE FROM connections WHERE source_ip = ? AND source_port = ? AND destination_ip = ? AND destination_port = ?;";

    sqlite3_stmt* stmt;
    int rc = sqlite3_prepare_v2(db, delete_sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot prepare statement: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_bind_int(stmt, 1, src_ip);
    sqlite3_bind_int(stmt, 2, src_port);
    sqlite3_bind_int(stmt, 3, dest_ip);
    sqlite3_bind_int(stmt, 4, dest_port);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "Delete failed: %s\n", sqlite3_errmsg(db));
        exit(1);
    }

    sqlite3_finalize(stmt);
}

void cleanup_database() {
    sqlite3_close(db);
}