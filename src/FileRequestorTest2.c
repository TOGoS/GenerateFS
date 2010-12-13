#define FUSE_USE_VERSION 26

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <err.h>
#include <sys/stat.h>
#include <string.h>
#include <fuse.h>
#include "genfs-errors.h"
#include "FileRequestor.h"

void test_open_file_result_parsing() {
  char buffer[23];
  int z;
  
  if( sizeof buffer != 23 ) {
    errx( 1, "Expected sizeof buffer to be %d, but was %d", 23, sizeof buffer );
  }

  z = FileRequestor_parse_open_file_result( "OK-ALIAS \"/home/junk/chambawamba\"", buffer, sizeof buffer );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }

  z = FileRequestor_parse_open_file_result( "OK-ALIAS \"/home/junk/chambawamba\" crumb", buffer, sizeof buffer );
  if( z != FILEREQUESTOR_RESULT_MALFORMED_RESPONSE ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MALFORMED_RESPONSE, z, __FILE__, __LINE__ );
  }

  z = FileRequestor_parse_open_file_result( "OK-ALIAS \"/home/junk/chambawamba\"", buffer, sizeof buffer - 1 );
  if( z != FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_open_file_result( "DOES-NOT-EXIST", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_DOES_NOT_EXIST ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_DOES_NOT_EXIST, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_open_file_result( "SERVER-ERROR", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_SERVER_ERROR ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_SERVER_ERROR, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_open_file_result( "DOES-NOT-EXIST \"askldmnaisd message asdnasndiunkjded\"", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MALFORMED_RESPONSE ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MALFORMED_RESPONSE, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_open_file_result( "SERVER-ERROR \"askldmnaisd message asdnasndiunkjded\"", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MALFORMED_RESPONSE ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MALFORMED_RESPONSE, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_open_file_result( "", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MALFORMED_RESPONSE ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MALFORMED_RESPONSE, z, __FILE__, __LINE__ );
  }
}

struct FillerTestResults {
  int cd_found;
  int pd_found;
  int test1_found;
  int subdir_found;
};

int null_trd_filler(void *buf, const char *name, const struct stat *stbuf, off_t off) { return 0; }

int trd_filler(void *buf, const char *name, const struct stat *stbuf, off_t off) {
  struct FillerTestResults *ftr = (struct FillerTestResults *)buf;
  
  if( strcmp(".",name) == 0 ) {
    ++ftr->cd_found;
  } else if( strcmp("..",name) == 0 ) {
    ++ftr->pd_found;
  } else if( strcmp("test1.txt",name) == 0 ) {
    ++ftr->test1_found;
    if( stbuf->st_size != 128 ) {
      errx( 1, "Expected test1.txt to be read as %d bytes, got %d, at %s:%d", 128, (int)stbuf->st_size, __FILE__, __LINE__ );
    } 
    if( stbuf->st_mode != 0100644 ) {
      errx( 1, "Expected test1.txt mode to be %d, got %d, at %s:%d", 0100644, stbuf->st_mode, __FILE__, __LINE__ );
    }
  } else if( strcmp("subdir",name) == 0 ) {
    ++ftr->subdir_found;
    if( stbuf->st_size != 0 ) {
      errx( 1, "Expected subdir to be read as %d bytes, got %d, at %s:%d", 0, (int)stbuf->st_size, __FILE__, __LINE__ );
    } 
    if( stbuf->st_mode != 0040755 ) {
      errx( 1, "Expected subdir mode to be %d, got %d, at %s:%d", 0040755, stbuf->st_mode, __FILE__, __LINE__ );
    }
  } else {
    errx( 1, "Unexpected filename in directory '%s', at %s:%d", name, __FILE__, __LINE__ );
  }
  
  return 0;
}

void test_read_directory( struct FileRequestor *fr ) {
  struct FillerTestResults tr = {
    .cd_found = 0,
    .pd_found = 0,
    .test1_found = 0,
    .subdir_found = 0
  };
  int z;
  
  z = FileRequestor_read_dir( fr, "/", &tr, trd_filler );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  
  if( tr.cd_found != 1 ) {
    errx( 1, "Expected . to have been found %d times, got %d, at %s:%d", 2, tr.cd_found, __FILE__, __LINE__ );
  }
  if( tr.test1_found != 1 ) {
    errx( 1, "Expected .. to have been found %d times, got %d, at %s:%d", 2, tr.pd_found, __FILE__, __LINE__ );
  }
  if( tr.test1_found != 1 ) {
    errx( 1, "Expected test1.txt to have been found %d times, got %d, at %s:%d", 2, tr.test1_found, __FILE__, __LINE__ );
  }
  if( tr.subdir_found != 1 ) {
    errx( 1, "Expected subdir to have been found %d times, got %d, at %s:%d", 2, tr.test1_found, __FILE__, __LINE__ );
  }
}

pid_t start_server() {
  pid_t cid;
  if( (cid = fork()) == 0 ) {
    if( execlp( "ruby", "testserv.rb", "testserv.rb", "-timeout", "5", "-port", "23823", NULL ) == -1 ) {
      err( 1, "Failed to start server" );
    }
    return -1; // should only get here on error
  } else {
    // Give it some time to start up because I can't
    // think of an easy way to wait for it...
    usleep(250000);
    return cid;
  }
}

int main( int argc, char **argv ) {
  struct FileRequestor fr;
  int z;
  pid_t server_pid = start_server();
  char outfilename[1024];
  struct stat stbuf;
  
  if( server_pid < 0 ) {
    err( 1, "Failed to start server (fork fail?)" );
  }
  
  /* Test connection error handling */
  
  FileRequestor_init( &fr, "127.0.0.1", 23820 );
  z = FileRequestor_read_dir( &fr, "/", NULL, null_trd_filler );
  if( z != GENFS_RESULT_IO_ERROR ) {
    errx( 1, "Expected %d, got %d, at %s:%d", GENFS_RESULT_IO_ERROR, z, __FILE__, __LINE__ );
  }
  if( errno != ECONNREFUSED ) {
    errx( 1, "Expected %d, got %d, at %s:%d", ECONNREFUSED, errno, __FILE__, __LINE__ );
  }
  
  /* Test read directories */
  
  FileRequestor_init( &fr, "127.0.0.1", 23823 );
  test_read_directory( &fr );
  
  z = FileRequestor_read_dir( &fr, "/doesnotexist", NULL, null_trd_filler );
  if( z != FILEREQUESTOR_RESULT_DOES_NOT_EXIST ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_DOES_NOT_EXIST, z, __FILE__, __LINE__ );
  }

  z = FileRequestor_read_dir( &fr, "/test1.txt", NULL, null_trd_filler );
  if( z != FILEREQUESTOR_RESULT_BAD_OPERATION ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_BAD_OPERATION, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_read_dir( &fr, "/server-error", NULL, null_trd_filler );
  if( z != FILEREQUESTOR_RESULT_SERVER_ERROR ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_SERVER_ERROR, z, __FILE__, __LINE__ );
  }
  
  /* Test getting file stats */
  
  z = FileRequestor_get_stat( &fr, "/doesnotexist.txt", &stbuf );
  if( z != FILEREQUESTOR_RESULT_DOES_NOT_EXIST ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_DOES_NOT_EXIST, z, __FILE__, __LINE__ );
  }  

  z = FileRequestor_get_stat( &fr, "/server-error", &stbuf );
  if( z != FILEREQUESTOR_RESULT_SERVER_ERROR ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_SERVER_ERROR, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_get_stat( &fr, "/secret.txt", &stbuf );
  if( z != FILEREQUESTOR_RESULT_PERMISSION_DENIED ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_PERMISSION_DENIED, z, __FILE__, __LINE__ );
  }

  z = FileRequestor_get_stat( &fr, "/test1.txt", &stbuf );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( stbuf.st_size != 128 ) {
    errx( 1, "Expected test1.txt to be %d bytes, got %d, at %s:%d", 128, (int)stbuf.st_size, __FILE__, __LINE__ );
  }
  if( stbuf.st_mode != 0100644 ) {
    errx( 1, "Expected test1.txt mode to be %d, got %d, at %s:%d", 0100644, stbuf.st_mode, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_get_stat( &fr, "/subdir", &stbuf );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( stbuf.st_size != 0 ) {
    errx( 1, "Expected subdir to be read as %d bytes, got %d, at %s:%d", 0, (int)stbuf.st_size, __FILE__, __LINE__ );
  } 
  if( stbuf.st_mode != 0040755 ) {
    errx( 1, "Expected subdir mode to be %d, got %d, at %s:%d", 0040755, stbuf.st_mode, __FILE__, __LINE__ );
  }
  
  /* Test open files for reading */
  
  z = FileRequestor_open_read( &fr, "/doesnotexist.txt", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_DOES_NOT_EXIST ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_DOES_NOT_EXIST, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_open_read( &fr, "/subdir", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_BAD_OPERATION ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_BAD_OPERATION, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_open_read( &fr, "/test1.txt", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( strcmp("test-data/test1.txt",outfilename) != 0 ) {
    errx( 1, "Expected '%s', got '%s', %s:%d", "test-data/test1.txt", outfilename, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_open_read( &fr, "/subdir/test2.txt", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( strcmp("test-data/test2.txt",outfilename) != 0 ) {
    errx( 1, "Expected '%s', got '%s', %s:%d", "test-data/test2.txt", outfilename, __FILE__, __LINE__ );
  }

  z = FileRequestor_close_read( &fr, "/subdir/test2.txt", outfilename );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    if( z == GENFS_RESULT_IO_ERROR ) warn( "IO Error" );
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }  
  
  z = FileRequestor_open_read( &fr, "/server-error", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_SERVER_ERROR ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_SERVER_ERROR, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_open_read( &fr, "/secret.txt", outfilename, sizeof outfilename );
  if( z != FILEREQUESTOR_RESULT_PERMISSION_DENIED ) {
    errx( 1, "Expected %d, got %d, %s:%d", FILEREQUESTOR_RESULT_PERMISSION_DENIED, z, __FILE__, __LINE__ );
  }
  
  /* Test open files for writing */
  
  kill( server_pid, SIGTERM );
  
  return 0;
}
