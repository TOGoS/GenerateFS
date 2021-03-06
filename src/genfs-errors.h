#ifndef GENFS_ERRORS_H
#define GENFS_ERRORS_H 1

#define GENFS_RESULT_OK 0
/** See errno for details */
#define GENFS_RESULT_SYSTEM_ERROR -1
/** See errno for details */
#define GENFS_RESULT_IO_ERROR -2

/*
 * Tokenizer errors
 */

#define TOKENIZER_RESULT_OK 0

#define TOKENIZER_RESULT_TOO_MANY_TOKENS -11
#define TOKENIZER_RESULT_TOO_MUCH_TOKEN -12
#define TOKENIZER_RESULT_MALFORMED_INPUT -13
#define TOKENIZER_RESULT_UNKNOWN_ERROR -14

/*
 * FileRequestor errors
 */

#define FILEREQUESTOR_RESULT_OK 0

/* Possibly user errors */
#define FILEREQUESTOR_RESULT_BAD_OPERATION -21
#define FILEREQUESTOR_RESULT_DOES_NOT_EXIST -22
#define FILEREQUESTOR_RESULT_PERMISSION_DENIED -23

/* Indicate bugs in client or server */
#define FILEREQUESTOR_RESULT_CLIENT_ERROR -31
#define FILEREQUESTOR_RESULT_SERVER_ERROR -32
#define FILEREQUESTOR_RESULT_MALFORMED_REQUEST -33
#define FILEREQUESTOR_RESULT_MALFORMED_RESPONSE -34

/* When running into client limitations? */
#define FILEREQUESTOR_RESULT_MESSAGE_TOO_LONG -44

#endif
