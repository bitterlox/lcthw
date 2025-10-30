#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define MAX_ROWS 10

struct Address {
  int id;
  int set;
  char *name;
  char *email;
};


struct DatabaseHeader {
  int max_data;
  int max_rows;
};

struct Database {
  struct DatabaseHeader header;
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


struct DatabaseHeader Database_load_header(struct Connection *conn)
{
  struct DatabaseHeader header = {.max_data = 0, .max_rows = 0};

  int rc = fread(&header, sizeof(struct DatabaseHeader), 1, conn->file);
  if (rc != 1)
    die(conn,"failed to read db header from file");

  return header;
}

void Database_load_data(struct Connection *conn)
{
  int i = 0;

  for (i = 0; i < MAX_ROWS; i++) {
    int rc = fread(&conn->db->rows[i].id, sizeof(int), 1, conn->file);
    if (rc != 1)
      die(conn,"failed to read id from database");

    rc = fread(&conn->db->rows[i].set, sizeof(int), 1, conn->file);
    if (rc != 1)
      die(conn,"failed to read set from database");

    char *name = malloc(conn->db->header.max_data);

    rc = fread(name, conn->db->header.max_data, 1, conn->file);
    if (rc != 1) {
      printf("rc: %d\n", rc);
      die(conn,"failed to read name from database");
    }

    conn->db->rows[i].name = name;

    char *email = malloc(conn->db->header.max_data);

    rc = fread(email, conn->db->header.max_data, 1, conn->file);
    if (rc != 1)
      die(conn,"failed to read email from database");

    conn->db->rows[i].email = email;
  }
}

void Database_write(struct Connection *conn)
{
  rewind(conn->file);

  // maybe we should not keep writing this but we need to make sure that
  // we rewind to just after the header otherwise we'd overwrite it
  int rc = fwrite(&conn->db->header, sizeof(struct DatabaseHeader), 1,
      conn->file);
  if (rc != 1)
    die(conn,"failed to write db header to file");

  int i = 0;

  for (i = 0; i < MAX_ROWS; i++) {
    rc = fwrite(&conn->db->rows[i].id, sizeof(int), 1, conn->file);
    if (rc != 1)
      die(conn,"failed to write id to database");

    rc = fwrite(&conn->db->rows[i].set, sizeof(int), 1, conn->file);
    if (rc != 1)
      die(conn,"failed to write set to database");

    if (conn->db->rows[i].name && conn->db->rows[i].email) {
      rc = fwrite(conn->db->rows[i].name, conn->db->header.max_data, 1,
          conn->file);
      if (rc != 1)
        die(conn,"failed to write name to database");

      rc = fwrite(conn->db->rows[i].email, conn->db->header.max_data, 1,
          conn->file);
      if (rc != 1)
        die(conn,"failed to write email to database");
    }
  }

  rc = fflush(conn->file);
  if (rc == -1)
    die(conn,"cannot flush database");
}

void Database_allocate(struct Connection *conn, int max_data, int max_rows)
{
  int address_size = 2 * sizeof(int) + 2 * max_data;
  conn->db = malloc(sizeof(struct DatabaseHeader) +
      address_size * MAX_ROWS);
  if (!conn->db)
    die(conn,"memory error");
}

struct Connection *Database_open(const char *filename, char mode)
{
  struct Connection *conn = malloc(sizeof(struct Connection));
  if (!conn)
    die(conn,"memory error");

  if (mode == 'c') {
    conn->file = fopen(filename, "wb");
  } else {
    conn->file = fopen(filename, "rb+");
    if (conn->file) {
      
      struct DatabaseHeader header = Database_load_header(conn);
      
      printf("loaded header, max_data: %d, max_rows: %d\n", 
            header.max_data, header.max_rows);

      Database_allocate(conn, header.max_data, header.max_rows);

      conn->db->header = header;

      Database_load_data(conn);
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
    if (conn->db) {
      int i = 0;

      for (i = 0; i < MAX_ROWS; i++) {
        if (conn->db->rows[i].name)
          free(conn->db->rows[i].name);
        if (conn->db->rows[i].email)
          free(conn->db->rows[i].email);
      }
      free(conn->db);
    }
    free(conn);
  }
}


void Database_create(struct Connection *conn, int max_data, int max_rows)
{
  Database_allocate(conn, max_data, max_rows);

  struct DatabaseHeader header = {.max_data = max_data, .max_rows = max_rows};

  conn->db->header = header;

  int i = 0;

  for (i = 0; i < MAX_ROWS; i++) {
    char *name = malloc(header.max_data);
    char *email = malloc(header.max_data);

    struct Address addr = {.id = i, .set = 0, .name = name, .email = email};
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

  int max_data = conn->db->header.max_data;

  addr->set = 1;
  // problem here is if input > max_data strncpy just truncates the input
  // but at that point it doesn't put a null byte at the end of the array
  // and so printf reads in email as well when it goes to print it
  char *res = strncpy(addr->name, name, max_data);

  if (!res)
    die(conn,"name copy failed");

  // this should fix it
  // altho should probably check if it did overflow
  res[max_data-1] = '\0';

  res = strncpy(addr->email, email, max_data);
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
  struct Address prev = conn->db->rows[id];

  // should probably also 0 out the bytes of name and email
  struct Address addr = {.id = id, .set = 0, .name = prev.name, 
    .email = prev.email};

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

  switch (action) {
    case 'c':
      if (argc != 5)
        die(conn, "need max_data, max_rows");
        
      int max_data = atoi(argv[3]);
      int max_rows = atoi(argv[4]);

      // if it's less than 8 we get a double free in close db
      if (max_data < 8)
        die(conn, "minimum row size is 8 bytes");

      Database_create(conn, max_data, max_rows);
      Database_write(conn);
      break;

    case 'g':
      if (argc != 4)
        die(conn,"need an id to get");

      int id = 0;
      if (argc > 3) id = atoi(argv[3]);
      if (id > MAX_ROWS) die(conn,"there's not that many records");

      Database_get(conn, id);
      break;

    case 's':
      if (argc != 6)
        die(conn,"need id, name, email to set");
      
      id = 0;
      if (argc > 3) id = atoi(argv[3]);
      if (id > MAX_ROWS) die(conn,"there's not that many records");

      Database_set(conn, id, argv[4], argv[5]);
      Database_write(conn);
      break;

    case 'd':
      if (argc != 4)
        die(conn,"need id to delete");

      id = 0;
      if (argc > 3) id = atoi(argv[3]);
      if (id > MAX_ROWS) die(conn,"there's not that many records");

      Database_delete(conn, id);

      Database_write(conn);
      break;

    case 'l':
      Database_list(conn);
      break;
    default:
      die(conn,"invalid action: c=create, g=get, s=set, d=del, l=list");
  }

  Database_close(conn);

  return 0;
}
