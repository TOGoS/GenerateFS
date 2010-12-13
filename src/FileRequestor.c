#include <err.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fuse.h>
#include <unistd.h>
#include "genfs-errors.h"
#include "FileRequestor.h"
#include "Tokenizer.h"

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port ) {
  r->hostname = hostname;
  r->port = port;
  return 0;
}

int FileRequestor_parse_error( struct TokenList *rts ) {
  if( rts->token_count >= 1 && strcmp("DOES-NOT-EXIST",rts->tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_DOES_NOT_EXIST;
  } else if( rts->token_count >= 1 && strcmp("SERVER-ERROR",rts->tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_SERVER_ERROR;
  } else if( rts->token_count >= 1 && strcmp("INVALID-OPERATION",rts->tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_BAD_OPERATION;
  } else if( rts->token_count >= 1 && strcmp("PERMISSION-DENIED",rts->tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_PERMISSION_DENIED;
  } else {
    return FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
  }
}

static void FileRequestor_chomp_line( char *l ) {
 check: switch( *l ) {
  case( '\r' ): case('\n'):
    *l = 0;
  case( 0 ):
    return;
  default:
    ++l;
    goto check;
  }
}

static int FileRequestor_open_control( struct FileRequestor *r ) {
  struct sockaddr_in sa;
  int socketfd;
  int z;
  
  memset( &sa, 0, sizeof sa );
  sa.sin_family = AF_INET;
  sa.sin_port = htons(r->port);
  
  if( !inet_pton( AF_INET, r->hostname, &sa.sin_addr ) ) {
    errno = EAFNOSUPPORT;
    return GENFS_RESULT_IO_ERROR;
  }
  
  // warn( "IP addy = %08lx, port = 0x%04hx, port support = %04hx", (unsigned long)sa.sin_addr.s_addr, sa.sin_port, r->port );
  
  socketfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if( socketfd < 0 ) return GENFS_RESULT_IO_ERROR;
  
  z = connect( socketfd, (struct sockaddr *)&sa, sizeof sa );
  if( z < 0 ) return GENFS_RESULT_IO_ERROR;
  
  return socketfd;
}

int FileRequestor_parse_open_file_result( const char *result, char *outfilename, int outfilenamelength ) {
  int z;
  struct TokenList rts;
  
  z = Tokenizer_tokenize( result, &rts );
  if( z != 0 ) return z;
  if( rts.token_count == 0 ) {
    return FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
  }
  
  if( rts.token_count == 2 && strcmp("OK-ALIAS",rts.tokens[0]) == 0 ) {
    if( strlen(rts.tokens[1]) > outfilenamelength-1 ) {
      return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
    } else {
      strcpy( outfilename, rts.tokens[1] );
      return FILEREQUESTOR_RESULT_OK;
    }
  } else {
    return FileRequestor_parse_error( &rts );
  }
}

int FileRequestor_parse_close_file_result( const char *result ) {
  int z;
  struct TokenList rts;
  
  z = Tokenizer_tokenize( result, &rts );
  if( z != 0 ) return z;
  if( rts.token_count == 0 ) {
    return FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
  }
  
  if( rts.token_count == 1 && strcmp("OK-CLOSED",rts.tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_OK;
  } else {
    return FileRequestor_parse_error( &rts );
  }
}

int FileRequestor_parse_dir_entries( FILE *stream, void *filler_dat, fuse_fill_dir_t filler ) {
  char linebuffer[1024];
  struct TokenList rts;
  struct stat st;
  int z;
  long size;
  
  memset( &st, 0, sizeof st );
  while( fgets( linebuffer, sizeof linebuffer, stream ) != NULL ) {
    z = Tokenizer_tokenize( linebuffer, &rts );
    if( z != 0 ) return z;
    if( rts.token_count == 4 && strcmp("DIR-ENTRY",rts.tokens[0]) == 0 ) {
      sscanf( rts.tokens[2], "%ld", &size );
      sscanf( rts.tokens[3], "%o", &st.st_mode );
      st.st_size = size;
      filler( filler_dat, rts.tokens[1], &st, 0 );
    } else if( rts.token_count >= 1 && strcmp("END-DIR-LIST",rts.tokens[0]) == 0 ) {
      return FILEREQUESTOR_RESULT_OK;
    } else {
      return FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
    }
  }
  return FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
}

static int FileRequestor_open_file( struct FileRequestor *r, const char *command, const char *infilename, char *outfilename, int outfilenamelength ) {
  int controlsock;
  char buffer[1024];
  int written;
  int errstash;
  ssize_t readed;
  
  written = snprintf( buffer, sizeof buffer, "%s \"%s\"\n", command, infilename );
  if( written >= sizeof buffer ) {
    return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
  }

  controlsock = FileRequestor_open_control( r );
  if( controlsock < 0 ) return GENFS_RESULT_IO_ERROR;
  
  write( controlsock, buffer, written );
  readed = read( controlsock, buffer, sizeof buffer - 1 );
  if( readed < 0 ) {
    errstash = errno;
    close( controlsock );
    errno = errstash;
    return GENFS_RESULT_IO_ERROR;
  }
  close( controlsock );
  if( readed >= sizeof buffer - 1 ) {
    return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
  }
  FileRequestor_chomp_line( buffer );
  return FileRequestor_parse_open_file_result( buffer, outfilename, outfilenamelength );
}

static int FileRequestor_close_file( struct FileRequestor *r, const char *command, const char *infilename, const char *outfilename ) {
  int controlsock;
  char buffer[1024];
  int written;
  int errstash;
  ssize_t readed;
  
  written = snprintf( buffer, sizeof buffer, "%s \"%s\" \"%s\"\n", command, infilename, outfilename );
  if( written >= sizeof buffer ) {
    return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
  }

  controlsock = FileRequestor_open_control( r );
  if( controlsock < 0 ) return GENFS_RESULT_IO_ERROR;
  
  write( controlsock, buffer, written );
  readed = read( controlsock, buffer, sizeof buffer - 1 );
  if( readed < 0 ) {
    errstash = errno;
    close( controlsock );
    errno = errstash;
    return GENFS_RESULT_IO_ERROR;
  }
  close( controlsock );
  if( readed >= sizeof buffer - 1 ) {
    return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
  }
  FileRequestor_chomp_line( buffer );
  return FileRequestor_parse_close_file_result( buffer );
}

int FileRequestor_open_read( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamelength ) {
  return FileRequestor_open_file( r, "OPEN-READ", infilename, outfilename, outfilenamelength );
}

int FileRequestor_close_read( struct FileRequestor *r, const char *infilename, const char *outfilename ) {
  return FileRequestor_close_file( r, "CLOSE-READ", infilename, outfilename );
}

int FileRequestor_open_write( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamelength ) {
  return FileRequestor_open_file( r, "OPEN-WRITE", infilename, outfilename, outfilenamelength );
}

int FileRequestor_close_write( struct FileRequestor *r, const char *infilename, const char *outfilename ) {
  return FileRequestor_close_file( r, "CLOSE-WRITE", infilename, outfilename );
}

int FileRequestor_read_dir( struct FileRequestor *r, const char *path, void *filler_dat, fuse_fill_dir_t filler ) {
  int controlsock;
  char buffer[1024];
  struct TokenList rts;
  int written;
  FILE *dirstream;
  int z;
  
  written = snprintf( buffer, sizeof buffer, "%s \"%s\"\n", "READ-DIR", path );
  if( written >= sizeof buffer ) {
    warnx( "Input filename is too long: %s", path );
    errno = ENAMETOOLONG;
    return -1;
  }
  
  controlsock = FileRequestor_open_control( r );
  if( controlsock < 0 ) {
    return GENFS_RESULT_IO_ERROR;
  }
  
  write( controlsock, buffer, written );
  
  dirstream = fdopen( controlsock, "r+" );
  if( dirstream == NULL ) {
    z = GENFS_RESULT_IO_ERROR;
    goto closefd;
  }
  
  if( fgets( buffer, sizeof buffer, dirstream ) == NULL ) {
    z = FILEREQUESTOR_RESULT_MALFORMED_RESPONSE;
    goto close;
  }
  
  if( (z = Tokenizer_tokenize( buffer, &rts )) < 0 ) {
    goto close;
  }
  
  if( rts.token_count == 1 && strcmp("OK-DIR-LIST",rts.tokens[0]) == 0 ) {
    z = FileRequestor_parse_dir_entries( dirstream, filler_dat, filler );
  } else {
    z = FileRequestor_parse_error( &rts );
  }
  
 close:
  fclose( dirstream );
 closefd:
  close( controlsock );
  return z;
}
