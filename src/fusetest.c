/**
 * Based on code at
 * http://sourceforge.net/apps/mediawiki/fuse/index.php?title=Hello_World
 */
#define FUSE_USE_VERSION 28

#include <unistd.h>

#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

const char *hello_str = "Hello, world!\n";
char bartext[1024];
int barexists = 0;
int barlength = 0;

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
  } else if( strcmp(path,"/bartest.txt") == 0 ) {
    if( barexists ) {
      stbuf->st_mode = S_IFREG | 0644;
      stbuf->st_nlink = 1;
      stbuf->st_size = barlength;
      return 0;
    } else {
      return -ENOENT;
    }
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
  if( barexists ) {
    filler(buf,"bar",NULL,0);
  }
  return 0;
}

static int fusetest_open( const char *path, struct fuse_file_info *fi ) {
  if( strcmp(path,"/foo") == 0 ) {
    if( (fi->flags &3) != O_RDONLY ) {
      return -EACCES;
    } else {
      return 0;
    }
  } else if( strcmp(path,"/bartest.txt") == 0 ) {
    if( barexists ) {
      return 0;
    } else {
      return -ENOENT;
    }
  } else {
    return -ENOENT;
  }
}

static int fusetest_read( const char *path, char *buf, size_t size,
			  off_t offset, struct fuse_file_info *fi ) {
  size_t len;
  if( strcmp(path,"/foo") == 0 ) {
    len = strlen(hello_str);
    if( offset < len ) {
      size = len-offset;
      memcpy(buf, hello_str+offset, size);
    } else {
      size = 0;
    }
    return size;
  } else if( strcmp(path,"/bartest.txt") == 0 ) {
    len = barlength;
    if( offset < len ) {
      size = len-offset;
      memcpy(buf, bartext+offset, size);
    } else {
      size = 0;
    }
    return size;
  } else {
    return -ENOENT;
  }
}

static int fusetest_create( const char *path, mode_t mode, struct fuse_file_info *fi ) {
  if( strcmp(path,"/bartest.txt") == 0 ) {
    barexists = 1;
    barlength = 0;
    return fusetest_open( path, fi );
  } else {
    return -EACCES;
  }
}

static int fusetest_truncate( const char *path, off_t offset, struct fuse_file_info *fi ) {
  if( strcmp(path,"/bartest.txt") == 0 ) {
    barlength = 0;
    return 0;
  } else {
    return -EACCES;
  }
}

static int fusetest_write( const char *path, char *buf, size_t size,
			   off_t offset, struct fuse_file_info *fi ) {
  size_t len;
  if( strcmp(path,"/bartest.txt") == 0 ) {
    if( barlength < sizeof bartext ) {
      if( barlength + size < sizeof bartext ) {
	len = size;
      } else {
	len = sizeof bartext - size;
      }
      memcpy( bartext+offset, buf, len );
    }
    barlength += size;
    return size;
  } else {
    return -EACCES;
  }
}

static struct fuse_operations fusetest_operations = {
  .getattr  = fusetest_getattr,
  .readdir  = fusetest_readdir,
  .create   = fusetest_create,
  .truncate = fusetest_truncate,
  .open     = fusetest_open,
  .read     = fusetest_read,
  .write    = fusetest_write,
};

int main( int argc, char **argv ) {
  return fuse_main( argc, argv, &fusetest_operations, NULL );
}
