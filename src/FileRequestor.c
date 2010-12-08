#include <err.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "FileRequestor.h"

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

int FileRequestor_parse_result( const char *result, char *outfilename, int outfilenamelength ) {
  char format[128];
  
  sprintf( format, "OK \"%%%d[^\\\r\n\"]\"", outfilenamelength-1 );
  if( sscanf(result, format, outfilename) ) {
    if( strlen(outfilename) == outfilenamelength-1 ) {
      return FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG;
    } else {
      return FILEREQUESTOR_RESULT_OK;
    }
  } else if( strcmp("DOES-NOT-EXIST",result) == 0 ) {
    return FILEREQUESTOR_RESULT_DOES_NOT_EXIST;
  } else if( strcmp("SERVER-ERROR",result) == 0 ) {
    return FILEREQUESTOR_RESULT_SERVER_ERROR;
  } else {
    return FILEREQUESTOR_RESULT_MESSAGE_MALFORMED;
  }
}

int FileRequestor_open_file( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamelength ) {
  int controlsock;
  char buffer[1024];
  int written;
  int errstash;
  ssize_t readed;
  int res;
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
  return FileRequestor_parse_result( buffer, outfilename, outfilenamelength );
}
