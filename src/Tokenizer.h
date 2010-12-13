#define TOKENLIST_BUFFER_SIZE 1024
#define TOKENLIST_MAX_TOKENS 32

struct TokenList {
  char buffer[TOKENLIST_BUFFER_SIZE];
  int token_count;
  char *tokens[TOKENLIST_MAX_TOKENS];
};

int Tokenizer_tokenize( const char *input, struct TokenList *tokenlist );
