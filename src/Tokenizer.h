#define TOKENIZER_RESULT_OK 0
#define TOKENIZER_RESULT_TOO_MANY_TOKENS -1
#define TOKENIZER_RESULT_TOO_MUCH_TOKEN -2
#define TOKENIZER_RESULT_MALFORMED_INPUT -3
#define TOKENIZER_RESULT_UNKNOWN_ERROR -3

#define TOKENLIST_BUFFER_SIZE 1024
#define TOKENLIST_MAX_TOKENS 32

struct TokenList {
  char buffer[TOKENLIST_BUFFER_SIZE];
  int token_count;
  char *tokens[TOKENLIST_MAX_TOKENS];
};

int Tokenizer_tokenize( const char *input, struct TokenList *tokenlist );
