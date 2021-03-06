#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "shared_map.h"

static shared_map_t map;

int main(int argc, char **argv) {
  struct stat st = {0};
  char *moddb_dir = "/var/tmp/shared_map";
  if (stat(moddb_dir, &st) == -1) {
    mkdir(moddb_dir, 0777);
  } 

  void *value;
  size_t length;

  map = shared_map_new(getpid());
  if (map == NULL) {
    fprintf(stderr, "Error: Failed to initialize Netlink\n");
    return 1;
  }

  if ((argc == 3) && !strcmp("get", argv[1])) {
    if (!shared_map_lookup(map, argv[2], &value, &length)) {
      printf("%s\n", (char *) value);
      free(value);
    } else {
      fprintf(stderr, "Key not found\n");
    }
  } else if ((argc == 4) && !strcmp("set", argv[1])) {
    if (shared_map_insert(map, argv[2], argv[3], strlen(argv[3]) + 1))
      fprintf(stderr, "Failed to insert element.\n");
  } else {
    fprintf(stderr, "usage: <set|get> <key> [value]\n");
    return 1;
  }

  shared_map_free(map);
  return 0;
}
