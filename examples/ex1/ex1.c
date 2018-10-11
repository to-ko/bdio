/* ex1.c
 *
 * create a simple bdio file that contains x,y data in separate records
 * and an additional record with a script to create an eps figure from the
 * data.
 *
 * Tomasz Korzec 2014
 ******************************************************************************/


#include <bdio.h>
#include <stdlib.h>
#include <math.h>

int main(int argc, char *argv[])
{
   BDIO *fh;
   int i;
   int x[200];
   double y[200];
   char sh[] ="#!/bin/bash\n"
              "octave --persist --eval \"[d, x]=system('lsbdio -d 2 ex1.bdio');"
              "[d, y]=system('lsbdio -d 3 ex1.bdio');"
              "plot(str2num(x),str2num(y),'k.-');"
              "print -deps ex1.eps\"\n";

   bdio_set_dflt_verbose(1);

   /* prepare data */
   for( i=0; i<200; i++ )
   {
      x[i]  = i;
      y[i]  = sin(i*20)*sin(((double) i)/(31.415926535897931));
   }
   
   /* open a file for writing */
   if ((fh = bdio_open( "ex1.bdio", "w",
      "The first record contains a shell script to plot the data\n"
      ""))==NULL)
      exit(EXIT_FAILURE);

   /* write record with shell script */
   if( bdio_start_record(BDIO_ASC_EXEC, 0, fh) )
      exit(EXIT_FAILURE);

   if( bdio_write(sh, strlen(sh), fh) != strlen(sh) )
      exit(EXIT_FAILURE);

   /* write record with x data */
   if( bdio_start_record(BDIO_BIN_INT32, 0, fh) )
      exit(EXIT_FAILURE);

   if( bdio_write_int32(x, 200*4, fh) != 200*4 )
      exit(EXIT_FAILURE);

   /* write record with y data */
   if( bdio_start_record(BDIO_BIN_F64, 0, fh) )
      exit(EXIT_FAILURE);

   if( bdio_write_f64(y, 200*8, fh) != 200*8 )
      exit(EXIT_FAILURE);
   

   if( bdio_close(fh) )
      exit(EXIT_FAILURE);

   exit(EXIT_SUCCESS);
}
