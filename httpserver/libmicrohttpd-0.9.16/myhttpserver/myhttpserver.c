#include <sys/types.h>
#ifndef _WIN32
#include <sys/select.h>
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif
#include <microhttpd.h>
#include <time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>

#define PAGE "<html><head><title>Error</title></head><body>Bad data</body></html>"

#define PORT 8888
#define FILENAME "params.txt"
#define MIMETYPE "text/plain"

static int
answer_to_connection (void *cls, struct MHD_Connection *connection,
                      const char *url, const char *method,
                      const char *version, const char *upload_data,
                      size_t *upload_data_size, void **con_cls)
{
    struct MHD_Response *response;
    int fd;
    int ret;
    struct stat sbuf;

    printf("answer to connection\n");
    printf("url = %s \n",url);

    if (0 != strcmp (method, "GET"))
      return MHD_NO;

    if ( (-1 == (fd = open (FILENAME, O_RDONLY))) ||
         (0 != fstat (fd, &sbuf)) )
    {
        /* error accessing file */
        if (fd != -1)
	  (void) close (fd);
        const char *errorstr =
          "<html><body>An internal server error has occured!\
                              </body></html>";
        response =
	  MHD_create_response_from_buffer (strlen (errorstr),
					 (void *) errorstr,
					 MHD_RESPMEM_PERSISTENT);
        if (NULL != response)
        {
            ret =
              MHD_queue_response (connection, MHD_HTTP_INTERNAL_SERVER_ERROR,
                                response);
            MHD_destroy_response (response);

            return ret;
        }
        else
            return MHD_NO;
    }
    response =
      MHD_create_response_from_fd_at_offset (sbuf.st_size, fd, 0);
    MHD_add_response_header (response, "Content-Type", MIMETYPE);
    ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return ret;
}


int
main ()
{

    printf("In main\n");
    struct MHD_Daemon *daemon;

    daemon = MHD_start_daemon (MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL, PORT, NULL, NULL,
                             &answer_to_connection, (void *) PAGE, MHD_OPTION_END);
    if (NULL == daemon)
      return 1;

    (void) getchar ();

    MHD_stop_daemon (daemon);

    return 0;
}
