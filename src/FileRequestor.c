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
#include "FileRequestor.h"
#include "Tokenizer.h"

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port ) {
  r->hostname = hostname;
  r->port = port;
  return 0;
}

static int FileRequestor_open_control( struct FileRequestor *r ) {
  struct sockaddr_in sa;
  int socketfd;
  int z;
  
  memset( &sa, 0, sizeof sa );
  sa.sin_family = AF_INET;
  sa.sin_port = htons(r->port);
  
  if( !inet_pton( AF_INET, r->hostname, &sa.sin_addr ) ) return -1;
  
  socketfd = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP );
  if( socketfd < 0 ) return -1;
  
  z = connect( socketfd, (struct sockaddr *)&sa, sizeof sa );
  if( z < 0 ) return -1;
  
  return socketfd;
}

int FileRequestor_parse_open_file_result( const char *result, char *outfilename, int outfilenamelength ) {
  int z;
  struct TokenList rts;
  
  z = Tokenizer_tokenize( result, &rts );
  if( z != 0 ) return z;
  if( rts.token_count == 0 ) {
    return FILEREQUESTOR_RESULT_MESSAGE_MALFORMED;
  }
  
  if( rts.token_count == 2 && strcmp("OK-ALIAS",rts.tokens[0]) == 0 ) {
    if( strlen(rts.tokens[1]) > outfilenamelength-1 ) {
      return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
    } else {
      strcpy( outfilename, rts.tokens[1] );
      return FILEREQUESTOR_RESULT_OK;
    }
  } else if( rts.token_count == 1 && strcmp("DOES-NOT-EXIST",rts.tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_DOES_NOT_EXIST;
  } else if( rts.token_count == 1 && strcmp("SERVER-ERROR",rts.tokens[0]) == 0 ) {
    return FILEREQUESTOR_RESULT_SERVER_ERROR;
  } else {
    return FILEREQUESTOR_RESULT_MESSAGE_MALFORMED;
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
      warnx( "Bad nummer args or someth? %s, %d", rts.tokens[0], rts.token_count );
      return FILEREQUESTOR_RESULT_MESSAGE_MALFORMED;
    }
  }
  warnx( "Reached end of feiul" );
  return FILEREQUESTOR_RESULT_MESSAGE_MALFORMED;
}

int FileRequestor_open_file( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamelength ) {
  int controlsock;
  char buffer[1024];
  int written;
  int errstash;
  ssize_t readed;
  int i;
  
  written = snprintf( buffer, 1024, "%s \"%s\"\n", "OPEN-READ", infilename );
  if( written > 1024 ) {
    warnx( "Input filename is too long: %s", infilename );
    errno = ENAMETOOLONG;
    return -1;
  }

  controlsock = FileRequestor_open_control( r );
  if( controlsock < 0 ) return -1;
  
  write( controlsock, buffer, written );
  readed = read( controlsock, buffer, 1024-1 );
  if( readed < 0 ) {
    errstash = errno;
    warn( "Failed to read response line" );
    close( controlsock );
    errno = errstash;
    return -1;
  }
  if( readed >= 1024-1 ) {
    warnx( "Response line is too long: %s", buffer );
  }
  close( controlsock );
  
  for( i=0; buffer[i] != 0; ++i ) {
    if( buffer[i] == '\r' || buffer[i] == '\n' ) {
      buffer[i] = 0;
      break;
    }
  }
  return FileRequestor_parse_open_file_result( buffer, outfilename, outfilenamelength );
}

int FileRequestor_read_dir( struct FileRequestor *r, const char *infilename, fuse_fill_dir_t filler ) {
  return 0; //todo and stuf
}