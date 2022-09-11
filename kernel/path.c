
/*
 * path.c
 * path manipulation utilities
 */

#include <kernel.h>
#include <kernel_shared.h>

int PathCombine(const char* p1, const char* p2, char* output) {
  // Combines the two path segments

  if (!strcmp(p2, ".")) {
    // p2 is current directory, use p1 directly
    sprintf(255, output, "%s", p1);
    return 0;
  }

  if (!strcmp(p2, "..")) {
    // moving up a directory. Find count to last path segment.
    int len = 0;
    const char* s = p1 + strlen(p1) - 1;
    while (s != p1) {
      if (*s == '/') {
        len = s - p1;
        break;
      }
      s--;
    }

    if (len == 0) {
      // No directory to move to, use root
      sprintf(255, output, "/");
      return 0;
    } else {
      // Found directory to move to, use it
      Debug("LEN: %d\n", len);
      strcpy(output, p1, len);
      output[len] = 0;
      return 0;
    }
  }

  if (strlen(p2) > 0 && p2[0] == '/') {
    // Absolute path, just use p2 directly
    sprintf(255, output, "%s", p2);
    return 0;
  }

  if (!strcmp(p1, "/")) {
    // Currently on root dir, just use p2 directly
    sprintf(255, output, "/%s", p2);
    return 0;
  }

  // Not on root dir, not changing to absolute path
  // Do p1/p2
  sprintf(255, output, "%s/%s", p1, p2);
  return 0;
}

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