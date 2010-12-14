#include <err.h>
#include "genfs-errors.h"
#include "Tokenizer.h"

int Tokenizer_logging = 0;

int Tokenizer_tokenize( const char *input, struct TokenList *tokenlist ) {
  int i;
  char c;
  int quotemode = 0;
  int readingtoken = 0;
  int bufferoffset = 0;
  int bsmode = 0;
  
  tokenlist->token_count = 0;
  for( i=0;
       (c = input[i]) &&
       (bufferoffset < TOKENLIST_BUFFER_SIZE-1) &&
       (tokenlist->token_count < TOKENLIST_MAX_TOKENS);
       ++i
  ) {
    switch( c ) {
    case('"'):
      if( bsmode ) goto appendchar;
      
      quotemode = !quotemode;
      if( quotemode ) {
	if( readingtoken ) {
	  if( Tokenizer_logging ) warn("Found quote after token started in '%s'", input);
	  return TOKENIZER_RESULT_MALFORMED_INPUT;
	}
	tokenlist->tokens[tokenlist->token_count] = &tokenlist->buffer[bufferoffset];
	readingtoken = 1;
      }
      break;
    case('\\'):
      if( bsmode ) goto appendchar;
      if( !quotemode ) {
	if( Tokenizer_logging ) warn("Found backslash in non-quoted token '%s'", input);
	return TOKENIZER_RESULT_MALFORMED_INPUT;
      }
      bsmode = 1;
      break;
    case(' '): case('\t'): case('\r'): case('\n'):
      if( !quotemode ) {
	if( readingtoken ) {
	  tokenlist->buffer[bufferoffset++] = 0;
	  ++tokenlist->token_count;
	  readingtoken = 0;
	}
	break;
      }
    default:
    appendchar:
      if( bsmode ) {
	switch( c ) {
	case('\\'): case('"'): break;
	case('n'): c = '\n'; break;
	case('r'): c = '\r'; break;
	case('t'): c = '\t'; break;
	default:
	  if( Tokenizer_logging ) warn("Found invalid escape char '%c' in '%s'",c,input);
	  return TOKENIZER_RESULT_MALFORMED_INPUT;
	}
	bsmode = 0;
      }
      tokenlist->buffer[bufferoffset] = c;
      if( !readingtoken ) {
	tokenlist->tokens[tokenlist->token_count] = &tokenlist->buffer[bufferoffset];
	readingtoken = 1;
      }
      ++bufferoffset;
    }
  }
  if( input[i] ) {
    if( bufferoffset == TOKENLIST_BUFFER_SIZE-1 ) {
      return TOKENIZER_RESULT_TOO_MUCH_TOKEN;
    }
    if( tokenlist->token_count == TOKENLIST_MAX_TOKENS ) {
      return TOKENIZER_RESULT_TOO_MANY_TOKENS;
    }
    return TOKENIZER_RESULT_UNKNOWN_ERROR;
  }
  if( bsmode ) {
    if( Tokenizer_logging ) warn("Reached end of input before backslashed character in '%s'", input);
    return TOKENIZER_RESULT_MALFORMED_INPUT;
  }
  if( quotemode ) {
    if( Tokenizer_logging ) warn("Reached end of input before end of quoted token in '%s'", input);
    return TOKENIZER_RESULT_MALFORMED_INPUT;
  }
  if( readingtoken ) {
    tokenlist->buffer[bufferoffset++] = 0;
    ++tokenlist->token_count;
    readingtoken = 0;
  }
  return TOKENIZER_RESULT_OK;
}
