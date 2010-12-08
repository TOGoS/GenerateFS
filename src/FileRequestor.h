#define FILEREQUESTOR_RESULT_OK 0
#define FILEREQUESTOR_RESULT_MESSAGE_MALFORMED -1
#define FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG -2
#define FILEREQUESTOR_RESULT_DOES_NOT_EXIST -10
#define FILEREQUESTOR_RESULT_SERVER_ERROR -11

struct FileRequestor {
  const char *hostname;
  short int port;
};

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port );
int FileRequestor_parse_result( const char *result, char *outfilename, int outfilenamebufferlength );
int FileRequestor_open_file( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength );
