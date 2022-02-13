
/*
 * path.c
 * path manipulation utilities
 */

#include <kernel.h>
#include <kernel_shared.h>

// Skips the first component of a path and returns
// a pointer to the rst
// e.g. /foo/bar/baz.txt returns /bar/baz.txt
char* PathSkipFirstComponent(const char* path) {
  if (path == NULL) {
    return NULL;
  }

  if (*path == '/') {
    path++;
  }

  while (*path != '/' && *path != 0) {
    path++;
    Debug("Current: '%s'\n", path);
  }

  if (strlen(path) == 0) {
    return NULL;
  }

  return path;
}

// Returns how many levels there are in the path
// e.g. /foo/bar/baz.txt returns 3
int PathGetDepth(const char* path) {
  int level = 0;

  if (path == NULL) {
    return level;
  }

  while (*path != 0) {
    if (*path == '/') {
      level = level + 1;
    }
    path++;
  }
  return level;
}