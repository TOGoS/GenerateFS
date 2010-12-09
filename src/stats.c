#include <sys/stat.h>
#include <stdio.h>

int main(int argc, char **argv) {
  printf("S_IFMT   = 0%06o\n", S_IFMT);
  printf("S_IFBLK  = 0%06o\n", S_IFBLK);
  printf("S_IFCHR  = 0%06o\n", S_IFCHR);
  printf("S_IFIFO  = 0%06o\n", S_IFIFO);
  printf("S_IFREG  = 0%06o\n", S_IFREG);
  printf("S_IFDIR  = 0%06o\n", S_IFDIR);
  printf("S_IFLNK  = 0%06o\n", S_IFLNK);
  printf("S_IFSOCK = 0%06o\n", S_IFSOCK);
  return 0;
}
