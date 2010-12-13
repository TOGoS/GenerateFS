#include <string.h>
#include <stdio.h>
#include <err.h>
#include "genfs-errors.h"
#include "Tokenizer.h"

void dump_tokens( struct TokenList *tokenlist ) {
  int i;
  
  for( i=0; i<tokenlist->token_count; ++i ) {
    fprintf( stderr, "token: '%s'\n", tokenlist->tokens[i] );
  }
}

int main( int argc, char **argv ) {
  struct TokenList tokenlist;
  char longbuf[TOKENLIST_BUFFER_SIZE];
  int z;
  
  z = Tokenizer_tokenize( "hello", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 1 ) {
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 1, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("hello",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "hello", tokenlist.tokens[0], __FILE__, __LINE__ );
  }

  z = Tokenizer_tokenize( "\"hello\"", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 1 ) {
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 1, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("hello",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "hello", tokenlist.tokens[0], __FILE__, __LINE__ );
  }

  z = Tokenizer_tokenize( "hello goodbye", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 2 ) {
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 2, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("hello",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "hello", tokenlist.tokens[0], __FILE__, __LINE__ );
  }
  if( strcmp("goodbye",tokenlist.tokens[1]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "goodbye", tokenlist.tokens[1], __FILE__, __LINE__ );
  }

  z = Tokenizer_tokenize( "\"hello \\\"and goodbye\\\"\"", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 1 ) {
    dump_tokens( &tokenlist );
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 1, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("hello \"and goodbye\"",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "hello \"and goodbye\"", tokenlist.tokens[0], __FILE__, __LINE__ );
  }

  z = Tokenizer_tokenize( "\"\\r\\n\\t\\\\\\\"\"", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 1 ) {
    dump_tokens( &tokenlist );
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 1, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("\r\n\t\\\"",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "\r\n\t\\\"", tokenlist.tokens[0], __FILE__, __LINE__ );
  }

  z = Tokenizer_tokenize( "1 2 3 \"and 4\" \"and \\\"more\\\" :)\" 5 6 7", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 8 ) {
    dump_tokens( &tokenlist );
    errx( 1, "Expected %d token(s), got %d, at %s:%d", 8, tokenlist.token_count, __FILE__, __LINE__ );
  }
  if( strcmp("1",tokenlist.tokens[0]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "1", tokenlist.tokens[0], __FILE__, __LINE__ );
  }
  if( strcmp("2",tokenlist.tokens[1]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "2", tokenlist.tokens[1], __FILE__, __LINE__ );
  }
  if( strcmp("3",tokenlist.tokens[2]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "3", tokenlist.tokens[2], __FILE__, __LINE__ );
  }
  if( strcmp("and 4",tokenlist.tokens[3]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "and 4", tokenlist.tokens[3], __FILE__, __LINE__ );
  }
  if( strcmp("and \"more\" :)",tokenlist.tokens[4]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "and \"more\" :)", tokenlist.tokens[4], __FILE__, __LINE__ );
  }
  if( strcmp("5",tokenlist.tokens[5]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "5", tokenlist.tokens[5], __FILE__, __LINE__ );
  }
  if( strcmp("6",tokenlist.tokens[6]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "6", tokenlist.tokens[6], __FILE__, __LINE__ );
  }
  if( strcmp("7",tokenlist.tokens[7]) != 0 ) {
    errx( 1, "Expected '%s', got '%s', at %s:%d", "7", tokenlist.tokens[7], __FILE__, __LINE__ );
  }
  
  z = Tokenizer_tokenize( "1 2 3 \"", &tokenlist );
  if( z != TOKENIZER_RESULT_MALFORMED_INPUT ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_MALFORMED_INPUT, z, __FILE__, __LINE__ );
  }
  z = Tokenizer_tokenize( "1 2 3 \" 4 5", &tokenlist );
  if( z != TOKENIZER_RESULT_MALFORMED_INPUT ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_MALFORMED_INPUT, z, __FILE__, __LINE__ );
  }
  z = Tokenizer_tokenize( "1 2 3\\4 5", &tokenlist );
  if( z != TOKENIZER_RESULT_MALFORMED_INPUT ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_MALFORMED_INPUT, z, __FILE__, __LINE__ );
  }
  z = Tokenizer_tokenize( "1 2 3\"4 5", &tokenlist );
  if( z != TOKENIZER_RESULT_MALFORMED_INPUT ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_MALFORMED_INPUT, z, __FILE__, __LINE__ );
  }
  z = Tokenizer_tokenize( "\"foo\\g g is not valid escape!\"", &tokenlist );
  if( z != TOKENIZER_RESULT_MALFORMED_INPUT ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_MALFORMED_INPUT, z, __FILE__, __LINE__ );
  }
  
  z = Tokenizer_tokenize( "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31", &tokenlist );
  if( z != TOKENIZER_RESULT_OK ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_OK, z, __FILE__, __LINE__ );
  }
  if( tokenlist.token_count != 32 ) {
    errx( 1, "Expected %d token(s), but got %d, at %s:%d", 32, tokenlist.token_count, __FILE__, __LINE__ );
  }
  
  z = Tokenizer_tokenize( "0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19 20 21 22 23 24 25 26 27 28 29 30 31 32", &tokenlist );
  if( z != TOKENIZER_RESULT_TOO_MANY_TOKENS ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_TOO_MANY_TOKENS, z, __FILE__, __LINE__ );
  }
  
  memset( longbuf, 'a', sizeof longbuf );
  
  z = Tokenizer_tokenize( longbuf, &tokenlist );
  if( z != TOKENIZER_RESULT_TOO_MUCH_TOKEN ) {
    errx( 1, "Expected %d, got %d, at %s:%d", TOKENIZER_RESULT_TOO_MUCH_TOKEN, z, __FILE__, __LINE__ );
  }
  
  return 0;
}
