/**
 * Based on code at
 * http://sourceforge.net/apps/mediawiki/fuse/index.php?title=Hello_World
 */
#define FUSE_USE_VERSION 26

#include <unistd.h>

#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

const char *hello_str = "Hello, world!\n";

static int fusetest_getattr( const char *path, struct stat *stbuf ) {
  memset( stbuf, 0, sizeof(struct stat) );
  if( strcmp(path,"/") == 0 ){
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;
    return 0;
  } else if( strcmp(path,"/foo") == 0 ) {
    stbuf->st_mode = S_IFREG | 0444;
    stbuf->st_nlink = 2;
    stbuf->st_size = strlen(hello_str);
    return 0;
  } else {
    return -ENOENT;
  }
}

static int fusetest_readdir( const char *path, void *buf,
  fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi
) {
  if( strcmp(path,"/") != 0 ) {
    return -ENOENT;
  }
  filler(buf,".",NULL,0);
  filler(buf,"..",NULL,0);
  filler(buf,"foo",NULL,0);
  return 0;
}

static int fusetest_open( const char *path, struct fuse_file_info *fi ) {
  if( strcmp(path,"/foo") != 0 ) {
    return -ENOENT;
  }
  if( (fi->flags &3) != O_RDONLY ) {
    return -EACCES;
  }
  return 0;
}

static int fusetest_read( const char *path, char *buf, size_t size,
			  off_t offset, struct fuse_file_info *fi ) {
  puts("Serving\n");
  size_t len;
  sleep(5);
  if( strcmp(path,"/foo") != 0 ) {
    return -ENOENT;
  }
  len = strlen(hello_str);
  if( offset < len ) {
    size = len-offset;
    memcpy(buf, hello_str+offset, size);
  } else {
    size = 0;
  }
  puts("Done serving\n");
  return size;
}

static struct fuse_operations fusetest_operations = {
  .getattr = fusetest_getattr,
  .readdir = fusetest_readdir,
  .open    = fusetest_open,
  .read    = fusetest_read,
};

int main( int argc, char **argv ) {
  return fuse_main( argc, argv, &fusetest_operations, NULL );
}
