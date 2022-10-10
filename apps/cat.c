
int main(int argc, char* argv[]) {
  if (argc < 2) {
    write(1, "Usage: cat filename1 filename2 ...\n", 35);
    return 1;
  }
  for (int i = 1; i < argc; ++i) {
    int fd = open(argv[i], 0);
    if (fd == -1) {
      write(1, "Could not open file\n", 20);
      return 1;
    }

    while (1) {
      char buf[256];
      int nbytes = read(fd, buf, sizeof(buf));
      if (nbytes <= 0) {
        break;
      }
      write(1, buf, nbytes);
    }
  }

  return 0;
}