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

#define BDIO_MAX_RECORD_LENGTH 1048575        /* 2^20-1 */
#define BDIO_MAX_LONG_RECORD_LENGTH 268435455 /* 2^28-1 */

void print_status(BDIO *fh)
{
   if( fh==NULL)
   {
      printf("\n");
   }else
   {
      printf("   fh=%p\n",(void *) fh);
      printf("   opened in mode %i\n",fh->mode);
      printf("   headers encountered so far:  %i\n",bdio_get_hcnt(fh));
      printf("   last header's version:       %i\n",bdio_get_hversion(fh));
      printf("   last header's creator:       %s\n",bdio_get_hcuser(fh));
      printf("   last header's modifier:      %s\n",bdio_get_hmuser(fh));
      printf("   last header's creation host: %s\n",bdio_get_hchost(fh));
      printf("   last header's mod. host:     %s\n",bdio_get_hmhost(fh));
      printf("   last header's prot.info:     %s\n",bdio_get_hpinfo(fh));
      printf("   last header's dir1:          %i\n",fh->hdirinfo1);
      printf("   last header's dir2:          %i\n",fh->hdirinfo2);
      printf("   records encountered so far:  %i\n",bdio_get_rcnt(fh));
      printf("   currently in state:          %i\n",fh->state);
      if(fh->state==BDIO_R_STATE)
      {
         printf("   length of current record:    %li\n",bdio_get_rlen(fh));
         printf("   format of current record:    %i\n",bdio_get_rfmt(fh));
         printf("   dsize %i swap %i long %i\n",fh->rdsize, fh->rswap, fh->rlongrec);
      }
      printf("\n");
   }
}

int main(int argc, char *argv[])
{
   BDIO *fh;
   double data[5] = {0.1, 0.2, 0.3, 0.4, 0.5};
   float  fdata[4]= {0.6, 0.7, 0.8, 0.9};
   int32_t    idata[2]= {1, 2};
   int64_t    ldata[6]= {3, 4, 5, 6, 7, 8};
   unsigned char   bdata[17]={0xbd,0x10,0x00,0xbd,0x10,0x00,0x01,0x23,0x45,0x67,0x89,0xab,0xcd,0xef,0x00,0xbd,0x10};
   char   cdata[100]="There is\nonly one way to avoid criticism: do nothing, say nothing, and be nothing. –Aristotle\0";
   char   xdata[200]="<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root>\n  <title>\n    My First XML Document\n  </title>\n</root>\n\0";
   char   edata[100]="#!/bin/sh\nlsbdio concat.dat\n\0";
   unsigned int udata[2] = {4294967295u, 1};
   double data2[10];
   double *data3;
   int rd;
   int i;

   printf("sizeof(int)    = %lu\n",(long) sizeof(int));
   printf("sizeof(long)   = %lu\n",(long) sizeof(long));
   printf("sizeof(size_t) = %lu\n",(long) sizeof(size_t));

   /* set error stream to stderr */
   bdio_set_dflt_msg(stderr);
   bdio_set_dflt_verbose(1);

/*   bdio_set_user("tom");*/
   bdio_set_host("shmo");

   /* create a bdio file with a single header only */
   fh = bdio_open("justaheader.dat","w","");
   bdio_close(fh);
   

   /* Now tests that should not generate any errors */
   /* write one empty record */
   /* open a file for writing */
   printf("Opening a file for writing:\n");
   fh = bdio_open( "test.dat", "w", "This is a test file");
   bdio_hash_auto(fh);
   printf("Writing an empty int32 record:\n");
   if(bdio_start_record(BDIO_BIN_INT32, 1, fh)!=0)
      exit(1);
   if(bdio_flush_record(fh)!=0)
      exit(1);
   print_status(fh);

   printf("Writing an ASCII record:\n");
   if(bdio_start_record(BDIO_ASC_GENERIC, 2, fh)!=0)
      exit(1);
   if(bdio_write(cdata, strlen(cdata)+1, fh)!=strlen(cdata)+1)
      exit(1);
   print_status(fh);

   printf("Writing a generic binary record:\n");
   if(bdio_start_record(BDIO_BIN_GENERIC, 3, fh)!=0)
      exit(1);
   if(bdio_write(bdata, 17, fh)!=17)
      exit(1);
   print_status(fh);

   printf("Writing an XML record:\n");
   if(bdio_start_record(BDIO_ASC_XML, 7, fh)!=0)
      exit(1);
   if(bdio_write(xdata, strlen(xdata)+1, fh)!=strlen(xdata)+1)
      exit(1);
   print_status(fh);

   printf("Writing an exec record:\n");
   if(bdio_start_record(BDIO_ASC_EXEC, 0, fh)!=0)
      exit(1);
   if(bdio_write(edata, strlen(edata)+1, fh)!=strlen(edata)+1)
      exit(1);
   print_status(fh);
   
   printf("Writing an f64 record:\n");
   if(bdio_start_record(BDIO_BIN_F64, 0, fh)!=0)
      exit(1);
   if(bdio_write_f64(data, 5*sizeof(double), fh)!=5*sizeof(double))
      exit(1);
   print_status(fh);

   /* write a record with 10 doubles */
   printf("Writing an f64 record with 10 doubles:\n");
   if(bdio_start_record(BDIO_BIN_F64, 0, fh)!=0)
      exit(1);
   if(bdio_write_f64(data, 5*sizeof(double), fh)!=5*sizeof(double))
      exit(1);
   if(bdio_write_f64(data, 5*sizeof(double), fh)!=5*sizeof(double))
      exit(1);
   print_status(fh);

   /* write a record with 5 doubles in big endian*/
   printf("Writing an f64BE record with 5 doubles:\n");
   if(bdio_start_record(BDIO_BIN_F64BE, 0, fh)!=0)
      exit(1);
   if(bdio_write_f64(data, 5*sizeof(double), fh)!=5*sizeof(double))
      exit(1);
   print_status(fh);

   /* write a record with 4 floats in big endian*/
   printf("Writing an f32BE record with 4 floats:\n");
   if(bdio_start_record(BDIO_BIN_F32BE, 0, fh)!=0)
      exit(1);
   if(bdio_write_f32(fdata, 4*sizeof(float), fh)!=4*sizeof(float))
      exit(1);
   print_status(fh);

   /* write a record with 2 int in little endian*/
   printf("Writing an int32LE record with 2 ints:\n");
   if(bdio_start_record(BDIO_BIN_INT32LE, 0, fh)!=0)
      exit(1);
   if(bdio_write_int32(idata, 2*sizeof(int), fh)!=2*sizeof(int))
      exit(1);
   print_status(fh);

   /* write a record with 6 long in big endian*/
   printf("Writing an int64BE record with 6 longs:\n");
   if(bdio_start_record(BDIO_BIN_INT64BE, 0, fh)!=0)
      exit(1);
   if(bdio_write_int64(ldata, 6*sizeof(long), fh)!=6*sizeof(long))
      exit(1);
   print_status(fh);

/* write a record with 2 uint in big endian*/
   printf("Writing an int32BE record with 2 uints:\n");
   if(bdio_start_record(BDIO_BIN_INT32BE, 0, fh)!=0)
      exit(1);
   if(bdio_write_int32((int *) udata, 2*sizeof(int), fh)!=2*sizeof(int))
      exit(1);
   print_status(fh);
   
   printf("Closing:\n");
   if(bdio_close(fh)==EOF)
      exit(1);

   /*
   printf("Closing again:\n");
   if(bdio_close(fh)==EOF)
      exit(1)
   */




   /***************************************************************************/
   /* open the same file in read mode */
   printf("Opening the file for reading:\n");
   fh = bdio_open( "test.dat", "r","This is a test file");
   print_status(fh);
   /* seek all the headers */
   printf("Seeking all records---------------------------------------------\n");
   if(fh!=NULL)
   while((fh->state == BDIO_R_STATE) || (fh->state == BDIO_H_STATE))
   {
      printf(" seeking next record\n");
      if(bdio_seek_record(fh)==EOF)
      {
         exit(1);
      }else 
         print_status(fh);
      if ( fh->state == BDIO_R_STATE )
      {
         if( fh->rfmt==BDIO_BIN_F64BE || fh->rfmt==BDIO_BIN_F64LE )
         {
            printf(" reading f64 record\n");
            printf("t %i size %i mendi %i sw %i\n",
                                   fh->rfmt, fh->rdsize, fh->endian, fh->rswap);
            rd = bdio_read_f64(data2, bdio_get_rlen(fh), fh);
            printf("  read %i bytes:",rd);
            if( rd>=sizeof(double) )
            {
               for( i=0; i<rd/sizeof(double); i++ )
               {
                  printf("%f ",data2[i]);
               }
            }
            printf("\n\n");
         }
         if( fh->rfmt==BDIO_BIN_F32BE || fh->rfmt==BDIO_BIN_F32LE )
         {
            printf(" reading f32 record\n");
            rd = bdio_read_f32(fdata, bdio_get_rlen(fh), fh);
            printf("  read %i bytes:",rd);
            if( rd>=sizeof(float) )
            {
               for( i=0; i<rd/sizeof(float); i++ )
               {
                  printf("%f ",fdata[i]);
               }
            }
            printf("\n");
         }
         if( fh->rfmt==BDIO_BIN_INT32BE || fh->rfmt==BDIO_BIN_INT32LE )
         {
            printf(" reading int32 record\n");
            rd = bdio_read_int32((int*)udata, bdio_get_rlen(fh), fh);
            printf("  read %i bytes:",rd);
            if( rd>=sizeof(int) )
            {
               for( i=0; i<rd/sizeof(int); i++ )
               {
                  printf("%u ",udata[i]);
               }
            }
            printf("\n\n");
         }
         if( fh->rfmt==BDIO_BIN_INT64BE || fh->rfmt==BDIO_BIN_INT64LE )
         {
            printf(" reading int64 record\n");
            rd = bdio_read_int64(ldata, bdio_get_rlen(fh), fh);
            printf("  read %i bytes:",rd);
            if( rd>=sizeof(long) )
            {
               for( i=0; i<rd/sizeof(long); i++ )
               {
                  printf("%li ",(long) ldata[i]);
               }
            }
            printf("\n\n");
         }
      }      
   }
   if(bdio_close(fh)==EOF)
      exit(1);

   /* test long records */
   printf("Creating a long record:\n");
   fh = bdio_open( "test2.dat", "w", "This file contains a long record");
   if(bdio_start_record(BDIO_BIN_F64BE, 0, fh)!=0)
      exit(1);
   
   /* create data to write into long record */
   data3 = (double*) malloc(BDIO_MAX_RECORD_LENGTH+123*8);
   for(i=0; i<BDIO_MAX_RECORD_LENGTH/8+113; i++)
   {
      data3[i] = data2[i%10];  
   }
   rd=bdio_write_f64(data2, 10*sizeof(double), fh);
   rd=bdio_write_f64(data3, (BDIO_MAX_RECORD_LENGTH/8+113)*8, fh);
   
   /* write a long record */
   /*
   recsize=0;
   while( bdio_get_rcnt(fh) < 3)
   {
      rd=bdio_write_f64(data2, 10*sizeof(double), fh);
      recsize += rd;
   }
   print_status(fh);
   if(bdio_close(fh)==EOF)
      exit(1);
   */
   if(bdio_close(fh)==EOF)
      exit(1);

   /* open the same file in read mode */
   printf("Opening the file for reading:\n");
   fh = bdio_open( "test2.dat", "r",NULL);
   /* seek all the headers */
   printf("Seeking all records---------------------------------------------\n");
   if(fh!=NULL)
   while((fh->state == BDIO_R_STATE) || (fh->state == BDIO_H_STATE))
   {
      printf(" seeking next record\n");
      if(bdio_seek_record(fh)==EOF)
      {
         exit(1);
      }else
         print_status(fh);
   }
   if(bdio_close(fh)==EOF)
      exit(1);

   /* open again and try to read the continuation record into a single array */
   printf("Opening the file for reading and read in all data:\n");
   
   fh = bdio_open( "test2.dat", "r",NULL);
   if(bdio_seek_record(fh)==EOF)
   {
      exit(1);
   }
   if(bdio_read_f64(data3,(BDIO_MAX_RECORD_LENGTH/8+123)*8,fh)
      !=(BDIO_MAX_RECORD_LENGTH/8+123)*8)
   {
      exit(1);
   }else
      print_status(fh);
   if(bdio_close(fh)==EOF)
      exit(1);
   for( i=0; i<(BDIO_MAX_RECORD_LENGTH/8+123); i++)
   {
      if(data3[i] != data2[i%10])
         printf("something went wrong\n");
   }
   free(data3);
   exit(0);
}
