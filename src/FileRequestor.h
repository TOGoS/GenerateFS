#include "genfs-errors.h"

struct FileRequestor {
  const char *hostname;
  short int port;
};

int FileRequestor_parse_open_file_result( const char *result, char *outfilename, int outfilenamebufferlength );
int FileRequestor_parse_close_file_result( const char *result );
int FileRequestor_parse_dir_entries( FILE *stream, void *filler_dat, fuse_fill_dir_t filler );

int FileRequestor_init( struct FileRequestor *r, const char *hostname, short int port );
int FileRequestor_get_stat( struct FileRequestor *r, const char *infilename, struct stat *st );
int FileRequestor_open_read( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength );
int FileRequestor_close_read( struct FileRequestor *r, const char *infilename, const char *outfilename );
int FileRequestor_create_open_write( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength, int mode );
int FileRequestor_truncate( struct FileRequestor *r, const char *path );
int FileRequestor_open_write( struct FileRequestor *r, const char *infilename, char *outfilename, int outfilenamebufferlength );
int FileRequestor_close_write( struct FileRequestor *r, const char *infilename, const char *outfilename );
int FileRequestor_read_dir( struct FileRequestor *r, const char *infilename, void *filler_dat, fuse_fill_dir_t filler );
