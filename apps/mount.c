
int main(int argc, char* argv[])
{
  if(argc != 3) {
    KPrint("Usage: mount <device> <mount_point>");
    return 1;
  }
  KPrint("Done mount");
}