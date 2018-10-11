/* testbdio.c
 *
 * tests the bdio library
 *
 * Tomasz Korzec 2013
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


int main(int argc, char *argv[])
{
   BDIO *fh;
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);
   const char str[] = "Far out in the uncharted backwaters of the unfashionable end of the western spiral arm of the Galaxy lies a small, unregarded yellow sun.";


   /* create a bdio file */
   fh = bdio_open("hash_s.dat","w","Test file with hashes in s-mode");
   bdio_hash_auto(fh);
   if(bdio_start_record(BDIO_ASC_GENERIC, 1, fh)!=0)
      exit(1);
   if(bdio_write((void*) str, strlen(str)+1, fh)!=strlen(str)+1)
      exit(1);
   if(bdio_start_record(BDIO_ASC_GENERIC, 1, fh)!=0)
      exit(1);
   if(bdio_write((void*) str, strlen(str)+1, fh)!=strlen(str)+1)
      exit(1);
   bdio_close(fh);
  


   exit(0);
}
