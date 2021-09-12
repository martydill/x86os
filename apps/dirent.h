#ifndef DIRENT_H
#define DIRENT_H

DIR *opendir(const char *name);
struct dirent *readdir(DIR *dirp);

#endif
