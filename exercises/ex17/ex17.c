#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_DATA 512
#define MAX_ROWS 100

struct Address {
  int id;
  int set;
  char name[MAX_DATA];
  char email[MAX_DATA];
};

struct Database {
  struct Address rows[MAX_ROWS];
};

struct Connection {
  FILE *file;
  struct Database *db;
};


void Database_close(struct Connection *conn);

void die(struct Connection *conn, const char *message)
{
  if (errno) {
    perror(message);
  } else {
    printf("ERROR: %s\n", message);
  }

  if (conn != NULL) {
    Database_close(conn);
  }

  exit(1);
}

void Address_print(struct Address *addr)
{
  printf("%d %s %s\n", addr->id, addr->name, addr->email);
}

void Database_load(struct Connection *conn)
{

  int rc = fread(conn->db, sizeof(struct Database), 1, conn->file);
  if (rc != 1) {
    die(conn,"failed to load database");
  }
}

struct Connection *Database_open(const char *filename, char mode)
{
  struct Connection *conn = malloc(sizeof(struct Connection));
  if (!conn)
    die(conn,"memory error");

  conn->db = malloc(sizeof(struct Database));
  if (!conn->db)
    die(conn,"memory error");

  if (mode == 'c') {
    conn->file = fopen(filename, "wb");
  } else {
    conn->file = fopen(filename, "rb+");

    if (conn->file) {
      Database_load(conn);
    }
  }

  if (!conn->file)
    die(conn,"failed to open file");

  return conn;
}

void Database_close(struct Connection *conn)
{
  if (conn) {
    if (conn->file)
      fclose(conn->file);
    if (conn->db)
      free(conn->db);
    free(conn);
  }
}

void Database_write(struct Connection *conn)
{
  rewind(conn->file);

  int rc = fwrite(conn->db, sizeof(struct Database), 1, conn->file);
  if (rc != 1)
    die(conn,"failed to write database");

  rc = fflush(conn->file);
  if (rc == -1)
    die(conn,"cannot flush database");
}

void Database_create(struct Connection *conn)
{
  int i = 0;

  for (i = 0; i < MAX_ROWS; i++) {
    // make a prototype to initialize the db
    struct Address addr = {.id = i, .set = 0};
    // assign it
    conn->db->rows[i] = addr;
  }
}

void Database_set(struct Connection *conn, int id, const char *name,
    const char *email)
{
  struct Address *addr = &conn->db->rows[id];
  if (addr->set)
    die(conn,"already set, delete it first");

  addr->set = 1;
  // problem here is if input > MAX_DATA strncpy just truncates the input
  // but at that point it doesn't put a null byte at the end of the array
  // and so printf reads in email as well when it goes to print it
  char *res = strncpy(addr->name, name, MAX_DATA);

  if (!res)
    die(conn,"name copy failed");

  // this should fix it
  // altho should probably check if it did overflow
  res[MAX_DATA-1] = '\0';

  res = strncpy(addr->email, email, MAX_DATA);
  if (!res)
    die(conn,"email copy failed");
}


void Database_get(struct Connection *conn, int id)
{
  struct Address *addr = &conn->db->rows[id];

  if (addr->set) {
    Address_print(addr);
  } else {
    die(conn,"ID is not set");
  }
}


void Database_delete(struct Connection *conn, int id)
{
  struct Address addr = {.id = id, .set = 0};
  conn->db->rows[id] = addr;
}

void Database_list(struct Connection *conn)
{
  int i = 0;
  struct Database *db = conn->db;

  for (i = 0; i < MAX_ROWS; i++) {
    struct Address *cur = &db->rows[i];

    if (cur->set) {
      Address_print(cur);
    }
  }
}

int main(int argc, char *argv[])
{
  if (argc < 3)
    die(NULL,"USAGE: ex17 <dbfile> <action> [action params]");

  char *filename = argv[1];
  char action = argv[2][0];
  struct Connection *conn = Database_open(filename, action);
  int id = 0;

  if (argc > 3) id = atoi(argv[3]);
  if (id > MAX_ROWS) die(conn,"there's not that many records");

  switch (action) {
    case 'c':
      Database_create(conn);
      Database_write(conn);
      break;

    case 'g':
      if (argc != 4)
        die(conn,"need an id to get");

      Database_get(conn, id);
      break;

    case 's':
      if (argc != 6)
        die(conn,"need id, name, email to set");

      Database_set(conn, id, argv[4], argv[5]);
      Database_write(conn);
      break;

    case 'd':
      if (argc != 4)
        die(conn,"need id to delete");

      Database_delete(conn, id);
      Database_write(conn);
      break;

    case 'l':
      Database_list(conn);
      break;
    default:
      die(conn,"invalid action: c=create, g=get, d=del, l=list");
  }

  Database_close(conn);

  return 0;
}
