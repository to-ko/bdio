/* testopen.c
 *
 * tests the opening and closing functions of bdio
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
   /*int i;*/

   /* set error stream to stderr */
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);

/******************************************************************************/
   printf("Tests that should succeed:\n\n");
   printf("----------------------------------------------------------------\n");
   printf("Open a file for writing\n");
   printf("Expecting: No output. Result:\n");
   fh = bdio_open( "test.dat", "w", "this is a test file" );
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Closing the file\n");
   printf("Expecting: No output. Result:\n");
   bdio_close(fh);
   printf("----------------------------------------------------------------\n\n");

   system("rm test.dat");
   printf("----------------------------------------------------------------\n");
   printf("Open an empty file for appending\n");
   printf("Expecting: No output. Result:\n");
   fh = bdio_open( "test.dat", "a", "this is a test file");
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Closing the file\n");
   printf("Expecting: No output. Result:\n");
   bdio_close(fh);
   printf("----------------------------------------------------------------\n\n");
   
   printf("----------------------------------------------------------------\n");
   printf("Open a file for appending\n");
   printf("Expecting: No output. Result:\n");
   fh = bdio_open( "test.dat", "a", NULL );
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Closing the file\n");
   printf("Expecting: No output. Result:\n");
   bdio_close(fh);
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Open a file for appending (with p-info string)\n");
   printf("Expecting: No output. Result:\n");
   fh = bdio_open( "test.dat", "a", "this is a test file" );
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Closing the file\n");
   printf("Expecting: No output. Result:\n");
   bdio_close(fh);
   printf("----------------------------------------------------------------\n\n");


/******************************************************************************/
   printf("\n\n\nTests that should fail:\n\n");
   printf("----------------------------------------------------------------\n");
   printf("Trying to open an non-existing file for reading\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "a_file_that_isnt.bin", "r", "" );
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open file in a non-existing mode:\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "a_file_that_isnt.bin", "x", "" );
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open a file with a non-matching p-info for reading\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "r", "wrong p-info");
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open a file with a non-matching p-info for appending\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "a", "wrong p-info");
   printf("----------------------------------------------------------------\n\n");
   
   system("rm test.dat");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open a file for writing without p-info\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "w", NULL);
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open an empty file for appending without p-info\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "a", NULL);
   printf("----------------------------------------------------------------\n\n");

   system("touch test.dat");
   system("chmod u-w test.dat");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open an empty file for writing without permissions\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "w", "this is a test file");
   printf("----------------------------------------------------------------\n\n");

   printf("----------------------------------------------------------------\n");
   printf("Trying to open an empty file for appending without permissions\n");
   printf("Expecting: error message. Result:\n");
   fh = bdio_open( "test.dat", "a", "this is a test file");
   printf("----------------------------------------------------------------\n\n");

   system("chmod u+w test.dat");
   system("rm test.dat");
   
   exit(0);
}
