/* testread.c
 *
 * tests the reading functionality of the bdio library
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
      printf("Unexpected error while opening. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   /* write a few bytes into the file */
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while starting record. testread failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_write(fdata, 11, fh)!=11)
   {
      printf("Unexpected error while writing. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   printf("----------------------------------------------------------------\n");
   printf("Trying to read from a file opened in write mode\n");
   printf("Expecting: error message. Result:\n");
   bdio_read(fdata, 1, fh);
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to seek_record in a file opened in write mode\n");
   printf("Expecting: error message. Result:\n");
   bdio_seek_record(fh);
   printf("----------------------------------------------------------------\n\n");

   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   if ((fh = bdio_open( "test.dat", "a", "This is a test file"))==NULL)
   {
      printf("Unexpected error while opening. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   printf("----------------------------------------------------------------\n");
   printf("Trying to read from a file opened in append mode\n");
   printf("Expecting: error message. Result:\n");
   bdio_read(fdata, 1, fh);
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to seek_record in a file opened in append mode\n");
   printf("Expecting: error message. Result:\n");
   bdio_seek_record(fh);
   printf("----------------------------------------------------------------\n\n");

   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   if ((fh = bdio_open( "test.dat", "r", "This is a test file"))==NULL)
   {
      printf("Unexpected error while opening. testread failed.\n");
      exit(EXIT_FAILURE);
   }

   printf("----------------------------------------------------------------\n");
   printf("Trying to read before seeking a record\n");
   printf("Expecting: error message. Result:\n");
   bdio_read(fdata, 1, fh);
   printf("----------------------------------------------------------------\n\n");

   if(bdio_seek_record(fh)==EOF)
   {
      printf("Unexpected error while seeking. testread failed.\n");
      exit(EXIT_FAILURE);
   }
   
   /*system("rm test.dat");*/
   printf("----------------------------------------------------------------\n");
   printf("Trying to read more than a record contains\n");
   printf("Expecting: error message. Result:\n");
   bdio_read(fdata, 15, fh);
   printf("----------------------------------------------------------------\n\n");
      
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testread failed.\n");
      exit(EXIT_FAILURE);
   }
   
   exit(EXIT_SUCCESS);
}
