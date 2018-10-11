/* ex3_1.c
 *
 * demonstrate error handling.
 * 1) silent mode (default)
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   BDIO *fh;
   int i;
   char *c="some data";

   /*  create an empty bdio file */
   fh = bdio_open( "ex3.bdio", "w", "test-file" );
   bdio_close(fh);

   /* open it for reading and try to read in some data */
   bdio_open("ex3.bdio","r",NULL);
   bdio_seek_record(fh);
   i = bdio_read(c, 5, fh);
   bdio_write(c, 5, fh);
   
   /* if short write-count: print errors */
   if( i != 5 )
   {
      bdio_perror("Last error: ",fh);
      bdio_pferror("First error: ",fh);
   }

   bdio_close(fh);
   exit(EXIT_SUCCESS);
}
