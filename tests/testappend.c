/* testappend.c
 *
 * tests the appending functionality of the bdio library
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

int main(int argc, char *argv[])
{
   BDIO *fh;
   char fdata[11]="some bytes";

   /* set error stream to stderr */
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);
      
   /* open a file for writing */
   if ((fh = bdio_open( "test.dat", "w", "This is a test file"))==NULL)
   {
      printf("Unexpected error while opening. testappend failed.\n");
      exit(EXIT_FAILURE);
   }

   /* write a few bytes into the file */
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while starting record. testappend failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_write(fdata, 11, fh)!=11)
   {
      printf("Unexpected error while writing. testappend failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testappend failed.\n");
      exit(EXIT_FAILURE);
   }   

   if ((fh = bdio_open( "test.dat", "a", "This is a test file"))==NULL)
   {
      printf("Unexpected error while opening. testappend failed.\n");
      exit(EXIT_FAILURE);
   }

   printf("----------------------------------------------------------------\n");
   printf("Trying to append to existing record\n");
   printf("Expecting: no output. Result:\n");
   bdio_append_record(BDIO_BIN_GENERIC, 0, fh);
   printf("----------------------------------------------------------------\n\n");

   
   printf("----------------------------------------------------------------\n");
   printf("Trying to write into appended record\n");
   printf("Expecting: no output. Result:\n");
   bdio_write(fdata, 11, fh);
   printf("----------------------------------------------------------------\n\n");

   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testappend failed.\n");
      exit(EXIT_FAILURE);
   }
   exit(EXIT_SUCCESS);
}
