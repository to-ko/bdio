/* ex0.c
 *
 * A minimal bdio example
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
   BDIO *fh;
   int x[5] = {1,2,3,4,5};

   /* open bdio file for writing */
   fh = bdio_open( "ex0.bdio", "w", "protocol info goes here" );

   /* start a integer record with user-info 0 and native endianness */
   bdio_start_record( BDIO_BIN_INT32, 0, fh );

   /* write 5 integers twice (5*32bit = 20 byte) and close the BDIO file */
   bdio_write_int32( x, 20, fh );
   bdio_write_int32( x, 20, fh );
   bdio_close( fh );

   /* open bdio file for reading */
   fh = bdio_open( "ex0.bdio", "r", NULL );

   /* move file pointer to the beginning of the first record */
   bdio_seek_record( fh );

   /* read two integers from the file, store them in x, and close the file */
   bdio_read_int32( x, 8, fh );
   bdio_close( fh );

   return( EXIT_SUCCESS );
}
