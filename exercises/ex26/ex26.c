#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <wordexp.h>
#include <string.h>
#include <stdbool.h>

#include "dbg.h"

/*
 *  TODO:
 *  - args handling
 *    - words to grep for
 *    - optional -o flag
 *  √ file hadling function
 *    - given a file path, open file at path and return buffer with content
 *
 *
 *
 *
 *
 */


const int MAX_CONFIG = 10;

void release_file(char *file)
{
  if (file) free(file);
}

char *open_file(const char *path)
{
  FILE *file;
  int code;
  struct stat stat_res;
  wordexp_t expanded;

  code =  wordexp(path, &expanded, 0);
  check(code == 0, "error expanding path: %s\n", path)

  for(int i = 0; i < expanded.we_wordc; i++) {
    debug("expanded %d: %s\n", i, expanded.we_wordv[i]);
  }

  char *expanded_path = expanded.we_wordv[0];

  code = stat(expanded_path, &stat_res);
  check(code == 0, "error calling stat on path: %s\n", expanded_path)

  file = fopen(expanded_path, "r");

  debug("file size: %lld \n", stat_res.st_size);

  // null byte at the end so printf works?
  char *buf = malloc(stat_res.st_size * sizeof(char) + 1);
  check_mem(buf);

  buf[stat_res.st_size * sizeof(char)] = '\0';

  // ignoring return value from fread
  fread(buf, stat_res.st_size, 1, file);
  check(code == 0, "error reading from file: %s\n", expanded_path)

  code = fclose(file);
  check(code == 0, "error closing file: %s\n", expanded_path)

  wordfree(&expanded);


  return buf;

  error:
    printf("error! \n");
    return NULL;
}

void release_config(char **config)
{
  for (int i = 0; i < MAX_CONFIG; i++) {
    if (config[i]) {
      free(config[i]);
    }
  }
  free(config);
}

char **load_config(const char *path)
{
  char *raw_config = open_file(path);
  check(raw_config, "config file is null");

 
  char *parsed;
  char *to_parse = raw_config;

  char **config = malloc(MAX_CONFIG * sizeof(char *));
  check_mem(config);

  int count = 0;

  for (; count < MAX_CONFIG; count++) {
    
    // this is the wierdest function ever
    // apparently it has a hidden pointer it uses in subsequent calls
    // to keep parsing
    parsed = strtok(to_parse, "\n");

    if (!parsed) {
      break;
    }

    config[count] = strdup(parsed);

    to_parse = NULL;

  }

  if (count == MAX_CONFIG) {
    log_warn("max number of config values read is %d", MAX_CONFIG);
  }

  release_file(raw_config);

  return config;

error:
  printf("couldn't load config\n");
  release_file(raw_config);
  release_config(config);

  return NULL;
}

typedef struct {
  bool or;
  int wordc;
  char **words;
} arguments;

void release_args(arguments args)
{
  for (int i = 0; i < args.wordc; i++) {
    if (args.words[i]) {
      free(args.words[i]);
    }
  }
  free(args.words);
}

arguments setup_args(int argc, char *argv[])
{
  arguments args;

  args.or = false;
  args.wordc = 0;
  args.words = malloc(argc * sizeof(char **));

  for (int i = 1; i < argc; i++) {
    if (strcmp("-o", argv[i]) == 0) {
      debug("found -o flag");
      args.or = true;

      continue;
    }

    int len = strlen(argv[i]);
    char *str = malloc(len * sizeof(char *));
    strcpy(str, argv[i]);
    args.words[args.wordc] = str;

    args.wordc++;
  }


  return args;
}


int main(int argc, char *argv[])
{
  char **config = load_config("~/.logfind");
  check(config, "config is null");


  for (int i = 0; i < MAX_CONFIG && config[i]; i ++) {
    printf("config %d: %s\n", i, config[i]);
  }

  arguments args = setup_args(argc, argv);

  printf("count %d argc %d or %d\n", args.wordc, argc, args.or);

  for (int i = 0; i < args.wordc; i++) {
    printf("word %d %s \n", i, args.words[i]);
  }

  release_args(args);
  release_config(config);


  // char **content1 = open_file("test1.txt", 0, NULL);
  // check(content1, "content is null")


  return 0;

  error:
    // release_file(content);
    return -1;
}


