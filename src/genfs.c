/**
 * Based on code at
 * http://sourceforge.net/apps/mediawiki/fuse/index.php?title=Hello_World
 */
#define FUSE_USE_VERSION 26

#include <unistd.h>
#include <err.h>
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include "genfs-errors.h"
#include "FileRequestor.h"

/* pread, pwrite missing from unistd.h; should look like this: */
ssize_t pread(int fildes, void *buf, size_t nbyte, off_t offset);
ssize_t pwrite(int fildes, const void *buf, size_t nbyte, off_t offset);

struct FDNameList {
  int fd;
  char *name;
  struct FDNameList *next;
};
static struct FDNameList namelist = {
  .fd = -1,
  .name = "",
  .next = NULL,
};

/*
 * TODO: How does FUSE do threading?
 * Need to make sure this is thread safe.
 */
static void add_fd_name( int fd, const char *name ) {
  struct FDNameList *newNode;
  char *newName = malloc(strlen(name)+1);
  strcpy(newName,name);
  newNode = malloc(sizeof(struct FDNameList));
  newNode->fd = fd;
  newNode->name = newName;
  newNode->next = namelist.next;
  namelist.next = newNode;
}

static const char *get_fd_name( int fd ) {
  struct FDNameList *nameNode;
  for( nameNode = &namelist; nameNode != NULL; nameNode = nameNode->next ) {
    if( nameNode->fd == fd ) {
      return nameNode->name;
    }
  }
  return NULL;
}

static void remove_fd_name( int fd ) {
  struct FDNameList *prevNode = NULL;
  struct FDNameList *nameNode = &namelist;
  if( fd < 0 ) {
    errx( 1, "Tried to remove %d from fd name list.", fd );
  }
  for( nameNode = &namelist; nameNode != NULL; prevNode = nameNode, nameNode = nameNode->next ) {
    if( nameNode->fd == fd ) {
      if( prevNode != NULL ) {
	prevNode->next = nameNode->next;
      } else {
	warn( "%d found in fd name list, but no previous node!", fd );
      }
      free( nameNode->name );
      free( nameNode );
      return;
    }
  }
  warnx( "%d not found in fd name list", fd );
}

static struct FileRequestor fr;

static int GeneratorFS_getattr( const char *path, struct stat *stbuf ) {
  int z = FileRequestor_get_stat( &fr, path, stbuf );
  
  warnx( "_getattr '%s'", path );
  
  switch( z ) {
  case( FILEREQUESTOR_RESULT_OK ):
    return 0;
  case( FILEREQUESTOR_RESULT_BAD_OPERATION ):
    return -ENOTSUP;
  case( FILEREQUESTOR_RESULT_DOES_NOT_EXIST ):
    return -ENOENT;
  case( FILEREQUESTOR_RESULT_PERMISSION_DENIED ):
    return -EACCES;
  case( GENFS_RESULT_IO_ERROR ):
    warn( "IO Error while statting '%s'", path );
    return -EIO;
  default:
    warnx( "Don't know how to turn result %d into an errno in getattr.", z );
    return -EBADMSG;
  }
}

static int GeneratorFS_setxattr( const char *path, const char *uhm, const char *what, size_t urr, int hmmm ) {
  warnx( "_setattr '%s'", path );
  
  return 0;
}

static int GeneratorFS_readdir( const char *path, void *buf,
  fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi
) {
  int z;
  
  warnx( "_readdir '%s'", path );
  
  z = FileRequestor_read_dir( &fr, path, buf, filler );
  switch( z ) {
  case( FILEREQUESTOR_RESULT_OK ):
    return 0;
  case( FILEREQUESTOR_RESULT_BAD_OPERATION ):
    return -ENOTSUP;
  case( FILEREQUESTOR_RESULT_DOES_NOT_EXIST ):
    return -ENOENT;
  case( FILEREQUESTOR_RESULT_PERMISSION_DENIED ):
    return -EACCES;
  case( GENFS_RESULT_IO_ERROR ):
    warn( "IO Error while reading dir '%s'", path );
    return -EIO;
  default:
    warnx( "Don't know how to turn result %d into an errno in readdir.", z );
    return -EBADMSG;
  }
}

static int GeneratorFS_handle_open_error( const char *path, int z ) {
  switch( z ) {
  case( FILEREQUESTOR_RESULT_BAD_OPERATION ):
    return -ENOTSUP;
  case( FILEREQUESTOR_RESULT_DOES_NOT_EXIST ):
    return -ENOENT;
  case( FILEREQUESTOR_RESULT_PERMISSION_DENIED ):
    return -EACCES;
  case( GENFS_RESULT_IO_ERROR ):
    warn( "IO Error while opening '%s'", path );
    return -EIO;
  default:
    warnx( "Don't know how to turn result %d into an errno in open.", z );
    return -EBADMSG;
  }
}

static int GeneratorFS__open( const char *path, struct fuse_file_info *fi, int accmode ) {
  char realname[1024];
  int z;
  
  warnx( "_open '%s' %d", path, accmode );
  
  if( accmode == O_RDONLY ) {
    z = FileRequestor_open_read( &fr, path, realname, sizeof realname );
  } else if( accmode == O_WRONLY ) {
    z = FileRequestor_open_write( &fr, path, realname, sizeof realname );
  } else {
    warn( "Tried to open '%s' with flags 0x%x; only O_RDONLY (0x%x), O_WRONLY (0x%x) supported.", path, fi->flags, O_RDONLY, O_WRONLY );
    return -ENOTSUP;
  }
  
  if( z == 0 ) {
    if( (int)(fi->fh = open( realname, fi->flags )) < 0 ) {
      warn( "Error opening real file '%s'", realname );
      return -errno;
    }
    add_fd_name( fi->fh, realname );
    return 0;
  } else {
    return GeneratorFS_handle_open_error( path, z );
  }
}

static int GeneratorFS_open( const char *path, struct fuse_file_info *fi ) {
  int accmode = (fi->flags & O_ACCMODE);
  return GeneratorFS__open( path, fi, accmode );
}

static int GeneratorFS_create( const char *path, mode_t mode, struct fuse_file_info *fi ) {
  return GeneratorFS__open( path, fi, O_WRONLY );
  /*
  int z;
  z = FileRequestor_create( &fr, path, mode );
  
  warnx( "_create '%s' 0%o", path, mode );
  
  switch( z ) {
  case( FILEREQUESTOR_RESULT_OK ):
    return 0;
  case( FILEREQUESTOR_RESULT_BAD_OPERATION ):
    return -ENOTSUP;
  case( FILEREQUESTOR_RESULT_DOES_NOT_EXIST ):
    return -ENOENT;
  case( FILEREQUESTOR_RESULT_PERMISSION_DENIED ):
    return -EACCES;
  case( GENFS_RESULT_IO_ERROR ):
    warn( "IO Error while creating '%s'", path );
    return -EIO;
  default:
    warnx( "Don't know how to turn result %d into an errno in create.", z );
    return -EBADMSG;
  }
  */
}

static int GeneratorFS_chmod( const char *path, mode_t mode ) {
  char realname[1024];
  int z;
  
  warnx( "_chmod '%s' 0%o", path, (int)mode );
  
  z = FileRequestor_open_write( &fr, path, realname, sizeof realname );
  if( z ) {
    return GeneratorFS_handle_open_error( path, z );
  }
  if( chmod( realname, mode ) ) {
    return -errno;
  }
  return 0;
}

static int GeneratorFS_chown( const char *path, uid_t uid, gid_t gid ) {
  char realname[1024];
  int z;
  
  warnx( "_chown '%s' %d %d", path, (int)uid, (int)gid );
  
  z = FileRequestor_open_write( &fr, path, realname, sizeof realname );
  if( z ) {
    return GeneratorFS_handle_open_error( path, z );
  }
  if( chown( realname, uid, gid ) ) {
    return -errno;
  }
  return 0;
}

static int GeneratorFS_utime( const char *path, struct utimbuf *utim ) {
  char realname[1024];
  int z;
  
  warnx( "_utime '%s'", path );
  
  z = FileRequestor_open_write( &fr, path, realname, sizeof realname );
  if( z ) {
    return GeneratorFS_handle_open_error( path, z );
  }
  if( utime( realname, utim ) ) {
    return -errno;
  }
  return 0;
}

static int GeneratorFS_read( const char *path, char *buf, size_t size,
			     off_t offset, struct fuse_file_info *fi ) {
  warnx( "_read '%s' %ld %d", path, (long)offset, (int)size );
  
  if( (int)fi->fh >= 0 ) {
    return (int)pread( fi->fh, buf, size, offset );
  } else {
    warnx( "Tried to read from '%s' with file descriptor = %d", path, (int)fi->fh );
    return -EBADF;
  }
}

static int GeneratorFS_write( const char *path, const char *buf, size_t size,
			      off_t offset, struct fuse_file_info *fi ) {
  int z;
  
  warnx( "_write '%s' %ld %d", path, (long)offset, (int)size );

  if( (int)fi->fh >= 0 ) {
    /* ignoring offset! */
    z = write( fi->fh, buf, size );
    if( z < 0 ) {
      warn( "Failed to write %d bytes to '%s', fd=%d", size, path, (int)fi->fh );
      return -errno;
    } else if( z < size ) {
      warn( "Only wrote %d of %d bytes to '%s', fd=%d", z, size, path, (int)fi->fh );
    }
    //warnx( "XX: write returning 0" );
    return 0;
  } else {
    warnx( "Tried to write to '%s' with file descriptor = %d", path, (int)fi->fh );
    return -EBADF;
  }
}

static int GeneratorFS_truncate( const char *path, off_t offset, struct fuse_file_info *fi ) {
  warnx( "_truncate '%s'", path );
  
  return 0;
}

static int GeneratorFS_fsync( const char *path, int sync, struct fuse_file_info *fi ) {
  warnx( "_fsync '%s' %d", path, (int)fi->fh );
  
  if( (int)fi->fh >= 0 ) {
    fsync( fi->fh );
    return 0;
  } else {
    return -EBADF;
  }
}

static int GeneratorFS_flush( const char *path, struct fuse_file_info *fi ) {
  warnx( "_flush '%s' %d", path, (int)fi->fh );
  
  return 0;
}

static int GeneratorFS_release( const char *path, struct fuse_file_info *fi ) {
  const char *realname;
  int accmode = (fi->flags & O_ACCMODE);
  int z;
  
  warnx( "_release '%s' %d", path, (int)fi->fh );
  
  if( (int)fi->fh >= 0 ) {
    realname = get_fd_name( fi->fh );
    close( fi->fh );
    if( realname == NULL ) {
      warnx( "Did not find real name for file descriptor %d", (int)fi->fh );
      z = -EBADFD;
    } else if( accmode == O_RDONLY ) {
      FileRequestor_close_read( &fr, path, realname );
      z = 0;
    } else if( accmode == O_WRONLY ) {
      FileRequestor_close_write( &fr, path, realname );
      z = 0;
    } else {
      warnx( "Somehow flags on '%s' became 0x%x, only O_RDONLY (0x%x) or O_WRONLY (0x%x) should be present.", path, fi->flags, O_RDONLY, O_WRONLY );
      z = -EBADFD;
    }
    
    remove_fd_name( fi->fh );
    fi->fh = -1;

    //warnx( "XX: release returning %d", z );
    return z;
  } else {
    return -EBADF;
  }
}

static struct fuse_operations GeneratorFS_operations = {
  .getattr   = GeneratorFS_getattr,
  .setxattr  = GeneratorFS_setxattr,
  .readdir   = GeneratorFS_readdir,
  .create    = GeneratorFS_create,
  .ftruncate = GeneratorFS_truncate,
  .open      = GeneratorFS_open,
  .read      = GeneratorFS_read,
  .write     = GeneratorFS_write,
  .chown     = GeneratorFS_chown,
  .chmod     = GeneratorFS_chmod,
  .utime     = GeneratorFS_utime,
  .fsync     = GeneratorFS_fsync,
  .flush     = GeneratorFS_flush,
  .release   = GeneratorFS_release,
};

int main( int argc, char **argv ) {
  FileRequestor_init( &fr, "127.0.0.1", 23823 );
  return fuse_main( argc, argv, &GeneratorFS_operations, NULL );
}
