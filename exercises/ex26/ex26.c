#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

#include "dbg.h"

/*
 *  TODO:
 *  - args handling
 *    - word to grep for
 *    - optional -o flag
 *  √ file hadling function
 *    - given a file path, open file at path and return buffer with content
 *
 *
 *
 *
 *
 */

char *open_file(const char *path, int max_size, char *ptr)
{
  FILE *file;
  int code;
  struct stat stat_res;

  code = stat(path, &stat_res);
  check(code == 0, "error calling stat on path: %s\n", path)

  file = fopen(path, "r");

  printf("file size: %lld \n", stat_res.st_size);

  // null byte at the end so printf works?
  char *buf = malloc(stat_res.st_size * sizeof(char) + 1);
  check_mem(buf);

  buf[stat_res.st_size * sizeof(char)] = '\0';

  int read = fread(buf, stat_res.st_size, 1, file);
  check(code == 0, "error reading from file: %s\n", path)

  code = fclose(file);
  check(code == 0, "error closing file: %s\n", path)

  return buf;

  error:
    printf("error! \n");
    return NULL;
}

void release_file(char *file)
{
  if (file) free(file);
}

int main(int argc, char *argv[])
{
  printf("%s\n", "hello world!");

  char *content = open_file("test.txt", 0, NULL);
  check(content, "content is null")

  printf("%s\n", content);

  // char **content1 = open_file("test1.txt", 0, NULL);
  // check(content1, "content is null")


  release_file(content);

  return 0;

  error:
    release_file(content);
    return -1;
}


