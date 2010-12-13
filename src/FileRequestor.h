#include "genfs-errors.h"

struct FileRequestor {
  const char *hostname;
  short int port;
};

int FileRequestor_parse_open_file_result( const char *result, char *outfilename, int outfilenamebufferlength );
int FileRequestor_parse_dir_entries( FILE *stream, void *filler_dat, fuse_fill_dir_t filler );

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port );
int FileRequestor_open_file( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength );
int FileRequestor_read_dir( struct FileRequestor *r, const char *infilename, void *filler_dat, fuse_fill_dir_t filler );
