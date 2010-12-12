#define FILEREQUESTOR_RESULT_OK 0

/* Possibly user errors */
#define FILEREQUESTOR_RESULT_DOES_NOT_EXIST -11
#define FILEREQUESTOR_RESULT_BAD_OPERATION -12

/* Indicate bugs in client or server */
#define FILEREQUESTOR_RESULT_CLIENT_ERROR -21
#define FILEREQUESTOR_RESULT_SERVER_ERROR -22

/* When running into client limitations? */
#define FILEREQUESTOR_RESULT_MESSAGE_MALFORMED -23
#define FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG -24

struct FileRequestor {
  const char *hostname;
  short int port;
};

int FileRequestor_parse_open_file_result( const char *result, char *outfilename, int outfilenamebufferlength );
int FileRequestor_parse_dir_entries( FILE *stream, void *filler_dat, fuse_fill_dir_t filler );

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port );
int FileRequestor_open_file( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength );
int FileRequestor_read_dir( struct FileRequestor *r, const char *infilename, fuse_fill_dir_t filler );
