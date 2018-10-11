/* testlongrec.c
 *
 * tests reading/writing/appending of long records of the bdio library
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

void compare(unsigned char *data, long sz)
{
   FILE *fp;
   long i;
   int fdat;
   
   fp = fopen("zebra-eye.tif","r");
   for(i=0; i<sz; i++)
   {
      fdat = fgetc(fp);
      if( fdat != (int) data[i] )
      {
         printf("Data got corrupted!\n");
         exit(EXIT_FAILURE);
      }
   }
   printf("%li bytes OK\n",sz);
}

int main(int argc, char *argv[])
{
   BDIO *fh;
   FILE *fp;
   unsigned char *data;
   long sz,sz2,i;

   /* set error stream to stderr */
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);


   /* load test data */
   fp = fopen("zebra-eye.tif","r");
   fseek(fp, 0L, SEEK_END);
   sz = ftell(fp);
   fseek(fp, 0L, SEEK_SET);
   data = malloc(sz);
   if( fread(data,1,sz,fp) != sz )
   {
      printf("Unexpected error while reading test-data. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   fclose(fp);
   
   /* open bdio file and write a single long record */
   printf("testing the reading & writing of a simple long-record.\n");
   if ((fh = bdio_open( "longrec.dat", "w", "file with long record"))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while starting record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_write(data, sz, fh)!=sz)
   {
      printf("Unexpected error while writing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }

   /* open bdio file and read in the data */
   if ((fh = bdio_open( "longrec.dat", "r", NULL))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_seek_record(fh)!=0)
   {
      printf("Unexpected error while seeking record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_read(data, sz, fh)!=sz)
   {
      printf("Unexpected error while reading. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   compare(data,sz);
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }

   /***************************************************************************/
   /* repeat test, but: write and read the file piece by piece                */
   /* open bdio file and write a single long record */
   printf("testing the reading & writing & appending of a long-record.\n");
   if ((fh = bdio_open( "longrec.dat", "w", "file with long record"))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while starting record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_write(data, 100, fh)!=100)
   {
      printf("Unexpected error while writing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if ((fh = bdio_open( "longrec.dat", "a", "file with long record"))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_append_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while appending record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   sz2 = bdio_write(&(data[100]), sz-100, fh);
   if(sz2!=(sz-100))
   {
      printf("expected to write %li, wrote %li\n",sz-100,sz2);
      printf("Unexpected error while writing (appending). testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if ((fh = bdio_open( "longrec.dat", "r", NULL))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_seek_record(fh)!=0)
   {
      printf("Unexpected error while seeking record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_read(data, 200, fh)!=200)
   {
      printf("Unexpected error while reading. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_read(&(data[200]), 2000000, fh)!=2000000)
   {
      printf("Unexpected error while reading. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_read(&(data[2000200]), sz-2000200, fh)!=sz-2000200)
   {
      printf("Unexpected error while reading. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   
   compare(data,sz);
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }

   /***************************************************************************/
   /* repeat test, but: append to an empty record                             */
   printf("testing the reading & writing & appending of a long-record II.\n");
   if ((fh = bdio_open( "longrec.dat", "w", "file with long record"))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_start_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while starting record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if ((fh = bdio_open( "longrec.dat", "a", "file with long record"))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_append_record(BDIO_BIN_GENERIC, 0, fh)!=0)
   {
      printf("Unexpected error while appending record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   sz2 = bdio_write(data, sz, fh);
   if(sz2!=(sz))
   {
      printf("expected to write %li, wrote %li\n",sz,sz2);
      printf("Unexpected error while writing (appending). testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if ((fh = bdio_open( "longrec.dat", "r", NULL))==NULL)
   {
      printf("Unexpected error while opening. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   if(bdio_seek_record(fh)!=0)
   {
      printf("Unexpected error while seeking record. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   for(i=0; i<sz; i++)
   {
      if(bdio_read(&(data[i]), 1, fh)!=1)
      {
         printf("Unexpected error while reading. testlongrec failed.\n");
         exit(EXIT_FAILURE);
      }
   }
   compare(data,sz);
   if(bdio_close(fh)==EOF)
   {
      printf("Unexpected error while closing. testlongrec failed.\n");
      exit(EXIT_FAILURE);
   }
   
   
   system("rm longrec.dat");
   exit(EXIT_SUCCESS);
}
