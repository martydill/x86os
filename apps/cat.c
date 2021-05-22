
int main(int argc, char* argv[]) {
  if (argc != 2) {
    write(1, "Usage: cat <filename>\n", 22);
    return 1;
  }

  int fd = open(argv[1], 0);
  if (fd == -1) {
    write(1, "Could not open file\n", 20);
    return 1;
  }

  while (1) {
    char buf[256];
    int nbytes = read(fd, buf, sizeof(buf));
    if (nbytes <= 0) {
      return 0;
    }
    write(1, buf, nbytes);
  }
}