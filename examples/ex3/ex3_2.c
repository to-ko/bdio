/* ex3_1.c
 *
 * demonstrate error handling.
 * 2) verbose mode (recommended)
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   BDIO *fh;
   char *c="some data";

   bdio_set_dflt_verbose(1);   /* at startup: 0      */
   bdio_set_dflt_msg(stdout);  /* at startup: stderr */
   
   /*  create an empty bdio file */
   fh = bdio_open( "ex3.bdio", "w", "test-file" );
   bdio_close(fh);

   /* open it for reading and try to read in some data */
   bdio_open("ex3.bdio","r",NULL);
   bdio_seek_record(fh);
   bdio_read(c, 5, fh);
   bdio_write(c, 5, fh);
   
   bdio_close(fh);
   exit(EXIT_SUCCESS);
}
