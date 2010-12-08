#include <err.h>
#include "FileRequestor.h"

int main( int argc, char **argv ) {
  char buffer[24];
  int z;
  
  if( sizeof buffer != 24 ) {
    errx( 1, "Expected sizeof buffer to be %d, but was %d", 24, sizeof buffer );
  }

  z = FileRequestor_parse_result( "OK \"/home/junk/chambawamba\"", buffer, sizeof buffer );
  if( z != FILEREQUESTOR_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_OK, z, __FILE__, __LINE__ );
  }

  z = FileRequestor_parse_result( "OK \"/home/junk/chambawamba\"", buffer, sizeof buffer - 1 );
  if( z != FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_result( "DOES-NOT-EXIST", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_DOES_NOT_EXIST ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_DOES_NOT_EXIST, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_result( "SERVER-ERROR", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_SERVER_ERROR ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_SERVER_ERROR, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_result( "DOES-NOT-EXIST \"askldmnaisd message asdnasndiunkjded\"", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MESSAGE_MALFORMED ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MESSAGE_MALFORMED, z, __FILE__, __LINE__ );
  }
  
  z = FileRequestor_parse_result( "SERVER-ERROR \"askldmnaisd message asdnasndiunkjded\"", buffer, 22 );
  if( z != FILEREQUESTOR_RESULT_MESSAGE_MALFORMED ) {
    errx( 1, "Expected %d, got %d, at %s:%d", FILEREQUESTOR_RESULT_MESSAGE_MALFORMED, z, __FILE__, __LINE__ );
  }
  
  return 0;
}
