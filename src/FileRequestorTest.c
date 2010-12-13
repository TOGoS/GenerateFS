#define FUSE_USE_VERSION 26

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
  
  z = FileRequestor_parse_open_file_result( "", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MALFORMED_RESPONSE ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MALFORMED_RESPONSE, z, __FILE__, __LINE__ );
  }
}

struct FillerTestResults {
  int test1_found;
  int subdir_found;
};

int trd_filler(void *buf, const char *name, const struct stat *stbuf, off_t off) {
  struct FillerTestResults *ftr = (struct FillerTestResults *)buf;
  
  if( strcmp("test1.txt",name) == 0 ) {
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

void test_read_directory() {
  FILE *dirstream;
  char buf4skip[32];
  struct FillerTestResults tr = {
    .test1_found = 0,
    .subdir_found = 0
  };
  int z;
  
  if( (dirstream = fopen("test-dir.txt","r")) == NULL ) {
    err( 1, "Couldn't open test-dir.txt, at %s:%d", __FILE__, __LINE__ );
  }
  fgets( buf4skip, sizeof buf4skip, dirstream );
  z = FileRequestor_parse_dir_entries( dirstream, &tr, trd_filler );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }
  
  if( tr.test1_found != 1 ) {
    errx( 1, "Expected test1.txt to have been found %d times, got %d, at %s:%d", 2, tr.test1_found, __FILE__, __LINE__ );
  }
  if( tr.subdir_found != 1 ) {
    errx( 1, "Expected subdir to have been found %d times, got %d, at %s:%d", 2, tr.test1_found, __FILE__, __LINE__ );
  }
  
  fclose( dirstream );
}

int main( int argc, char **argv ) {
  test_open_file_result_parsing();
  test_read_directory();
  return 0;
}
