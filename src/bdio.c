/** @file bdio.c
 *  @brief Main file for bdio-library
 *  @details Details & license
 *  @version 1.0
 *  @author Tomasz Korzec, Alberto Ramos, Hubert Simma
 *  @date 2013-2018
 *  @copyright GNU Lesser General Public License v3.
 */

/*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU Lesser General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



/* includes */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef _NO_POSIX_LIBS

   /* for unix host names */
   #include <unistd.h>
   #include <sys/types.h>
   #include <sys/socket.h>
   #include <netdb.h>

   /* for unix user ids */
   #include <sys/types.h>
   #include <pwd.h>

#endif

/* for time stamps */
#include <time.h>

#include <bdio.h>

/******************************************************************************/
/* private preprocessor scripts                                               */
/******************************************************************************/

/* magic number for file header */
#define BDIO_MAGIC 0x7ffbd07e

/*bdio version number */
#define BDIO_VERSION 1

/* maximal size of a record's data payload */
#define BDIO_MAX_RECORD_LENGTH 1048575        /* 2^20-1 */
#define BDIO_MAX_LONG_RECORD_LENGTH 268435455 /* 2^28-1 */

/* buffer size - must be at least 4095+8 */
#define BDIO_BUF_SIZE 1048576

/* maximal length of the host-name string incl. 0-terminator */
#define BDIO_MAX_HOST_LENGTH 256

/* maximal length of the user-name string incl. 0-terminator */
#define BDIO_MAX_USER_LENGTH 33

/* maximal length of the protocol-info string incl. 0-terminator
 * == 4095-2*BDIO_MAX_USER_LENGTH-2*BDIO_MAX_HOST_LENGTH-12
 */
#define BDIO_MAX_PINFO_LENGTH 3505

#define HEADER_INT(fmt, uinfo, len) \
  (        0x00000001                                 /* magic=1       */ \
         | ((fmt) << 4)                               /* format        */ \
         | ((uinfo) << 8)                             /* user info     */ \
         | (((len)-4) <<12)                           /* record length */ \
  )

#define HEADER_INT_LONG(fmt, uinfo, len) \
  (        0x0000000000000001                         /* magic=1       */ \
         | 0x0000000000000008                         /* long rec      */ \
         | ((fmt) << 4)                               /* format        */ \
         | ((uinfo) << 8)                             /* user info     */ \
         | (((uint64_t)(len)-8) <<12)                 /* record length */ \
  )

/******************************************************************************/
/* private global variables                                                   */
/******************************************************************************/
static FILE *default_msg = NULL;
static int default_verbosity = 0;

static char user_str[BDIO_MAX_USER_LENGTH]="\0";
static char host_str[BDIO_MAX_HOST_LENGTH]="\0";


/******************************************************************************/
/* private functions                                                          */
/******************************************************************************/
static char isBigEndian()
{
   union{
     char C[4];
     int  R   ;
        }word;
   word.R=1;
   if(word.C[3]==1) return 1;
   if(word.C[0]==1) return 0;
   return -1;
}

static void swap64(void *R, long N)
{
   register unsigned char *j,*k;
   unsigned char swap;
   unsigned char *max;

   max = (unsigned char*)R+N-7;
   for(j=R;j<max;)
   {
      k=j+7;
      swap = *j; *j = *k;  *k = swap;
      j++; k--;
      swap = *j; *j = *k;  *k = swap;
      j++; k--;
      swap = *j; *j = *k;  *k = swap;
      j++; k--;
      swap = *j; *j = *k;  *k = swap;
      j+=5;
   }
}


static void swap32(void *R, long N)
{
  register unsigned char *j,*k;
  unsigned char swap;
  unsigned char *max;

  max = (unsigned char*)R+N-3;
  for(j=R;j<max;)
  {
    k=j+3;
    swap = *j; *j = *k;  *k = swap;
    j++; k--;
    swap = *j; *j = *k;  *k = swap;
    j+=3;
  }
}

static int is_valid_bdio(const char *caller, BDIO *fh)
{
   /* returns  1 if fh is a pointer to a valid bdio structure 
    * returns  0 if fh is a NULL pointer
    *            or if fh is in an error state
    */
   if( fh==NULL )
   {
      return 0;
   }
   if( fh->state==BDIO_E_STATE )
   {
      return 0;
   }
   if( (fh->state!=BDIO_R_STATE) &&
       (fh->state!=BDIO_H_STATE) &&
       (fh->state!=BDIO_N_STATE) )
   {
      return 0;
   }
   return 1;
}


#ifndef _NO_POSIX_LIBS
static int user2buf(unsigned char *buf)
{
   int len;
   uid_t uid;
   struct passwd *pwd;
   if(user_str[0]=='\0')
   {
      uid = getuid();
      pwd = getpwuid(uid);
      if( pwd==NULL )
      {
         buf[0]='\0';
         return 1;
      }else
      {
         len = strlen(pwd->pw_name)+1;
         if( len < BDIO_MAX_USER_LENGTH )
         {
            strcpy((char*)buf, pwd->pw_name);
            strcpy(user_str, pwd->pw_name);
            return len;
         }else
         {
            /* truncate to maxLen */
            strncpy((char*)buf, pwd->pw_name, BDIO_MAX_USER_LENGTH-1);
            buf[BDIO_MAX_USER_LENGTH-1]='\0';
            strcpy(user_str,(char*) buf);
            return BDIO_MAX_USER_LENGTH;
         }
      }
   }else
   {
      strcpy((char*)buf, user_str);
      return strlen(user_str)+1;
   }
}
#else
static int user2buf(unsigned char *buf)
{
   if(user_str[0]=='\0')
   {
      strcpy((char*)buf, "unknown");
      return 8;
   }else
   {
      strcpy((char*)buf, user_str);
      return strlen(user_str)+1;   
   }
}
#endif

#ifndef _NO_POSIX_LIBS
static int host2buf(unsigned char *buf)
{
   struct addrinfo hints, *info;
   int gai_result;
   char hostname[BDIO_MAX_HOST_LENGTH];
   int len;
   
   if(host_str[0]=='\0')
   {
      if( gethostname(hostname, BDIO_MAX_HOST_LENGTH)==-1 )
      {
          buf[0]='\0';
          return 1;
      }

      memset(&hints, 0, sizeof hints);
      hints.ai_family = AF_UNSPEC; /*either IPV4 or IPV6*/
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_CANONNAME;

      if ((gai_result = getaddrinfo(hostname, "http", &hints, &info)) != 0)
      {
          buf[0]='\0';
          freeaddrinfo(info);
          return 1;
      }
      len = strlen(info->ai_canonname);
      if( len<BDIO_MAX_HOST_LENGTH-1 )
      {
         strcpy((char*)buf, info->ai_canonname);
         strcpy(host_str, info->ai_canonname);
         freeaddrinfo(info);
         return len+1;
      }
      else
      {
         strncpy((char*)buf, info->ai_canonname,BDIO_MAX_HOST_LENGTH-1);
         buf[BDIO_MAX_HOST_LENGTH-1]='\0';
         strcpy(host_str,(char*)buf);
         freeaddrinfo(info);
         return BDIO_MAX_HOST_LENGTH;
      }
   }else
   {
      strcpy((char*)buf, host_str);
      return strlen(host_str)+1;   
   }
}
#else
static int host2buf(unsigned char *buf)
{
   if(host_str[0]=='\0')
   {
      strcpy((char*)buf, "unknown");
      return 8;
   }else
   {
      strcpy((char*)buf, host_str);
      return strlen(host_str)+1;   
   }
}
#endif

static void bdio_error(int syserr, char *errmsg, BDIO *fh)
{
   if( syserr != 0)
   {
      sprintf(fh->error,"%s\n  %.128s",errmsg,strerror(errno));
   }else
   {
      strcpy(fh->error,errmsg);
   }
   if(fh->nerror==0)
      strcpy(fh->ferror,fh->error);
   fh->nerror++;

   if( fh->verbose )
   {
      fprintf(fh->msg,"%s\n",fh->error);
   }
}


static void buf2head(BDIO *fh, unsigned char *buf)
{
   int bufpos=0;
   int len,totlen;
   int maxLen=128;
   uint32_t hdr[5];
   memcpy(hdr,fh->buf,20);
   if( fh->endian==BDIO_BEND )
      swap32(hdr,20);
   fh->hmagic = hdr[0];
   fh->hversion  = (hdr[1] & 0xffff0000) >> 16;
   fh->hdirinfo1 = (hdr[2] & 0xffc00000) >> 22;
   fh->hdirinfo2 = (hdr[2] & 0x003fffff);
   fh->hcdate = hdr[3];
   fh->hmdate = hdr[4];
   bufpos=20;
   len = strlen((char*)buf+bufpos)+1;
   totlen=len;
   if( len>maxLen || totlen>fh->hssize )
   {
      bdio_error(0,"Error in buf2head. String buffers too short.",fh);
      fh->state = BDIO_E_STATE;
      return;
   }
   strcpy(fh->hcuser, (char*)buf+bufpos);
   bufpos += len;
   fh->hmuser = &(fh->hcuser[totlen]);
   len = strlen((char*)buf+bufpos)+1;
   totlen+=len;
   if( len>maxLen || totlen>fh->hssize )
   {
      bdio_error(0,"Error in buf2head. String buffers too short.",fh);
      fh->state = BDIO_E_STATE;
      return;
   }
   strcpy(fh->hmuser, (char*)buf+bufpos);
   bufpos += len;
   fh->hchost = &(fh->hcuser[totlen]);
   len = strlen((char*)buf+bufpos)+1;
   totlen += len;
   if( len>maxLen || totlen>fh->hssize )
   {
      bdio_error(0,"Error in buf2head. String buffers too short.",fh);
      fh->state = BDIO_E_STATE;
      return;
   }
   strcpy(fh->hchost, (char*)buf+bufpos);
   bufpos += len;
   fh->hmhost = &(fh->hcuser[totlen]);
   len = strlen((char*)buf+bufpos)+1;
   totlen += len;
   if( len>maxLen || totlen>fh->hssize )
   {
      bdio_error(0,"Error in buf2head. String buffers too short.",fh);
      fh->state = BDIO_E_STATE;
      return;
   }
   strcpy(fh->hmhost, (char*)buf+bufpos);
   bufpos += len;
   fh->hpinfo = &(fh->hcuser[totlen]);
   len = strlen((char*)buf+bufpos)+1;
   totlen=len;
   if( len>maxLen || totlen>fh->hssize )
   {
      bdio_error(0,"Error in buf2head. String buffers too short.",fh);
      fh->state = BDIO_E_STATE;
      return;
   }
   strcpy(fh->hpinfo, (char*)buf+bufpos);
}


static int write_header(BDIO *fh,char *protocol_info)
{
   int len,dir1,dir2,minpadding;
   int wr;
   uint32_t hdr[5];
   time_t ttime;
   
   /* set creation user */
   fh->bufidx=20;
   fh->bufidx += user2buf(&(fh->buf[fh->bufidx]));

   /* set last-modification user */
   fh->bufidx += user2buf(&(fh->buf[fh->bufidx]));

   /* set creation host */
   fh->bufidx += host2buf(&(fh->buf[fh->bufidx]));

   /* set last-modification host */
   fh->bufidx += host2buf(&(fh->buf[fh->bufidx]));

   minpadding = 2*BDIO_MAX_USER_LENGTH+2*BDIO_MAX_HOST_LENGTH+12-fh->bufidx;

   /* set protocol info */
   len = strlen( protocol_info )+1; /* length including 0-byte */
   if( len>BDIO_MAX_PINFO_LENGTH )
   {
      len = BDIO_MAX_PINFO_LENGTH;
   }
   strncpy((char*)&(fh->buf[fh->bufidx]),protocol_info,len);
   fh->bufidx+=len;
   fh->buf[fh->bufidx-1]='\0'; /* make sure it's 0-terminated */
   minpadding += (4-(fh->bufidx+minpadding)%4)%4;  /* header ends at 4b bndry*/
   for( len=0; len<minpadding; len++ )
   {
      fh->buf[fh->bufidx++]='\0';
   }

   /* fill out first 20 bytes */
   hdr[0] = BDIO_MAGIC;
   hdr[1] = ( (BDIO_VERSION & 0xffff) << 16 )
           |( (fh->bufidx-8) & 0x00000fff );
   dir1=0;
   dir2=0;
   hdr[2] =  ( (dir1 & 0x3ff) << 22 )
            |(  dir2 & 0x3fffff );

   /* set creation and modification date */
   ttime = time(NULL);
   if( ttime == ((time_t) - 1))
   {
      bdio_error(1,"Error in write_header. time fails with",fh);
      hdr[3] = 0;
      hdr[4] = 0;
   }else
   {
      hdr[3] = (uint32_t) ttime;
      hdr[4] = (uint32_t) ttime;
   }

   if(fh->endian == BDIO_BEND)
      swap32(hdr,20);

   memcpy(fh->buf,hdr,20);
   
   /* allocate memory for strings with header information*/
   fh->hcuser = (char*) malloc((fh->bufidx-20) * sizeof(char));
   if( fh->hcuser == NULL)
   {
      bdio_error(1,"Error in write_header. malloc fails with",fh);
      return EOF;
   }
   fh->hssize=fh->bufidx-20;

   /* fill out header-information from info in the buffer */
   buf2head(fh,fh->buf);
   /* set the remaining  header fields */
   fh->hcnt++;
   fh->state = BDIO_H_STATE;
   fh->ridx = fh->bufidx;
   fh->rlen = fh->bufidx;
   fh->rlongrec = 0;

   /* write to file */
   wr=fwrite(fh->buf,1,fh->bufidx,fh->fp);
   if( wr != fh->bufidx )
   {
      bdio_error(1,"Error in write_header. fwrite fails with",fh);
      free(fh->hcuser);
      return EOF;
   }
   return 0;
}

static int update_header(BDIO *fh)
{
   int len,dir1,dir2,minpadding;
   int wr;
   uint32_t hdr[5];
   time_t ttime;

   /* set creation user */
   fh->bufidx=20;
   len=strlen(fh->hcuser)+1;
   memcpy(&(fh->buf[fh->bufidx]),fh->hcuser,len);
   fh->bufidx += len;

   /* set last-modification user */
   fh->bufidx += user2buf(&(fh->buf[fh->bufidx]));

   /* set creation host */
   len=strlen(fh->hchost)+1;
   memcpy(&(fh->buf[fh->bufidx]),fh->hchost,len);
   fh->bufidx += len;

   /* set last-modification host */
   fh->bufidx += host2buf(&(fh->buf[fh->bufidx]));

   minpadding = 2*BDIO_MAX_USER_LENGTH+2*BDIO_MAX_HOST_LENGTH+12-fh->bufidx;

   /* set protocol info */
   len = strlen( fh->hpinfo )+1; /* length including 0-byte */
   if( len>BDIO_MAX_PINFO_LENGTH )
   {
      len = BDIO_MAX_PINFO_LENGTH;
   }
   memcpy(&(fh->buf[fh->bufidx]),fh->hpinfo,len);
   fh->bufidx+=len;
   fh->buf[fh->bufidx-1]='\0'; /* make sure it's 0-terminated */
   minpadding += (4-(fh->bufidx+minpadding)%4)%4;  /* header ends at 4b bndry*/
   for( len=0; len<minpadding; len++ )
   {
      fh->buf[fh->bufidx++]='\0';
   }

   /* fill out first 20 bytes */
   hdr[0] = BDIO_MAGIC;
   hdr[1] = ( (BDIO_VERSION & 0xffff) << 16 )
           |( (fh->bufidx-8) & 0x00000fff );
   dir1=0;
   dir2=0;
   hdr[2] =  ( (dir1 & 0x3ff) << 22 )
            |(  dir2 & 0x3fffff );

   /* set creation and modification date */
   ttime = time(NULL);
   if( ttime == ((time_t) - 1))
   {
      bdio_error(1,"Error in write_header. time fails with",fh);
      hdr[3] = fh->hcdate;
      hdr[4] = 0;
   }else
   {
      hdr[3] = fh->hcdate;
      hdr[4] = (uint32_t) ttime;
   }

   if(fh->endian == BDIO_BEND)
      swap32(hdr,20);

   memcpy(fh->buf,hdr,20);

   /* re-allocate memory for strings with header information if necessary*/
   if( fh->hssize < fh->bufidx-20 )
   {
      fh->hcuser = (char*) realloc(fh->hcuser, (fh->bufidx-20) * sizeof(char));
      if( fh->hcuser == NULL)
      {
         bdio_error(1,"Error in write_header. realloc fails with",fh);
         return EOF;
      }
   }
   fh->hssize=fh->bufidx-20;

   /* fill out header-information from info in the buffer */
   buf2head(fh,fh->buf);

   /* write to file */
   wr=fwrite(fh->buf,1,fh->bufidx,fh->fp);
   if( wr != fh->bufidx )
   {
      bdio_error(1,"Error in write_header. fwrite fails with",fh);
      return EOF;
   }
   return 0;
}


static int read_header(BDIO *fh)
{
   /* assumes that fh->rstart is already set correctly */
   int wr;
   int len;
   uint32_t hdr[2];

   /* read header up to length entry */
   if( fh->ridx>4 )
   {
      bdio_error(0,"Error in read_header. Not at the beginning of header.",fh);
      return EOF;
   }
   wr = fread( &(fh->buf[fh->ridx]), 1, 8-fh->ridx, fh->fp );
   if ( wr != 8-fh->ridx )
   {
      if( feof(fh->fp) )
         bdio_error(0,"Error in read_header. Unexpected EOF.",fh);
      else
         bdio_error(1,"Error in read_header. fread fails with",fh);
      return EOF;
   }

   memcpy(hdr,fh->buf,8);
   if(fh->endian==BDIO_BEND)
      swap32(hdr,8);
   
   if(hdr[0] != BDIO_MAGIC)
   {
      bdio_error(0,"Error in read_header. Not a valid bdio file.",fh);
      return EOF;
   }

   fh->hversion = (hdr[1] & 0xffff0000)>>16;
   len          =  hdr[1] & 0x00000fff;

   wr = fread( fh->buf+8, 1, len, fh->fp );
   if ( wr != len )
   {
      if( feof( fh->fp ) )
         bdio_error(0,"Error in read_header. Unexpected EOF.",fh);
      else
         bdio_error(1,"Error in read_header. fread fails with",fh);
      return EOF;
   }

   /* allocate memory for header information*/
   if( fh->hcuser==NULL )
   {
      fh->hcuser = (char*) malloc((len-12) * sizeof(char));
      if( fh->hcuser == NULL)
      {
         bdio_error(1,"Error in read header. malloc fails with",fh);
         return EOF;
      }
      fh->hssize=len-12;
   }
   else if( fh->hssize < len-12 )
   {
      fh->hcuser = (char*) realloc(fh->hcuser, (len-12) * sizeof(char));
      if( fh->hcuser == NULL)
      {
         bdio_error(1,"Error in read header. realloc fails with",fh);
         return EOF;
      }
      fh->hssize=len-12;
   }

   buf2head(fh,fh->buf);

   /* set the remaining  header fields */
   fh->hcnt++;
   fh->state = BDIO_H_STATE;
   fh->hstart = fh->rstart;
   fh->rlen = len+8;
   fh->ridx = len+8;
   fh->rlongrec = 0;
   fh->bufstart = 0;
   fh->bufidx = fh->ridx;
   return 0;
}

static int flush_buf(BDIO *fh)
{
   int w;
   /* write contents of buffer to file and reset buffer */
   /* TODO: it would be good to update the record-length at this point */
   w=fwrite(fh->buf, 1, fh->bufidx, fh->fp);
   fh->bufstart += fh->bufidx;
   fh->bufidx=0;
   return w;
}

static int min(int a, int b)
{
   return (a>b)?b:a;
}

static int buf_write(unsigned char *dat, int n, BDIO *fh)
{
   int nn;
   int res=0;
   int nf;
   /* write n bytes from dat to bdio file fh, use buffer
      return number of bytes written, or a short-count on failure */
   do{
      nn = min(n, BDIO_BUF_SIZE-fh->bufidx);
      memcpy(&(fh->buf[fh->bufidx]),dat,nn);
      n -= nn;
      res+=nn;
      dat+=nn;
      fh->bufidx += nn;
      fh->ridx += nn;
      fh->rlen += nn;
      if( n>0 )
      {
         nn = fh->bufidx;
         if ((nf=flush_buf(fh)) != nn)
         {
            res -= (nn-nf);
            bdio_error(1,"Error in buf_write. fwrite fails with",fh);
            return res;
         }
      }
   }while( n!=0 );
   return res;
}


static size_t bdio_write_hash(BDIO *fh)
{
   int nb, i;
   uint32_t magic[1];
   unsigned char digest[16];

   if (fh->hash_auto==BDIO_NO_HASH)
   {
      bdio_error(1,"Error in bdio_write_hash: HASH_AUTO not set",fh);
      fh->state=BDIO_E_STATE;
      return -1;
   }
   
   printf("fh->state=%i\n",fh->state);
   MD5_Final(digest,fh->hash);
   for (i=0;i<16;i++)
      fh->prev_digest[i] = digest[i];

   if (fh->hash_mode==BDIO_HASH_CHAIN)
   {
      magic[0] = BDIO_HASH_MAGIC_C;
   } else {
      magic[0] = BDIO_HASH_MAGIC_S;
   }
   
   fh->hash_auto=BDIO_NO_HASH; /* no hash records of hash records of hash records... */
   bdio_start_record(BDIO_BIN_GENERIC, 7, fh);

   nb  = bdio_write(magic, 4, fh);
   nb += bdio_write(digest, 16, fh);

   
   bdio_flush_record(fh);
   fh->hash_auto=BDIO_AUTO_HASH;

   return nb;
}

/******************************************************************************/
/* public functions                                                           */
/******************************************************************************/

void bdio_hash_auto(BDIO *fh)
{
   fh->hash_auto = BDIO_AUTO_HASH;
   fh->hash_mode = BDIO_HASH_SINGL;
   if( (fh->hash = malloc(sizeof(MD5_CTX)))==NULL )
   {
      fh->state=BDIO_E_STATE;
      bdio_error(1,"Error in bdio_hash_auto. Out of memory",fh);
   }
}


void bdio_hash_chain(BDIO *fh)
{
   int i;
  
   if (!fh->hash_auto)
      bdio_error(0,"Error in bdio_hash_chain. BDIO_HASH_AUTO no set (maybe call bdio_hash_auto first?)",fh);

   fh->hash_mode = BDIO_HASH_CHAIN;
   for (i=0;i<16;i++)
   {
      fh->prev_digest[i] = (unsigned char)0;
   }
}


int bdio_is_hash_record(unsigned char digest[16], BDIO *fh)
{
   unsigned char d[4];
   int rb, is_mg;
   long fpos;
   
   if (bdio_get_rlen(fh)!=20)
      return 0;

   fpos = ftell(fh->fp);
   rb = fread(d,1,4,fh->fp);
   is_mg = d[3];
   is_mg <<=8;
   is_mg |= d[2];
   is_mg <<=8;
   is_mg |= d[1];
   is_mg <<=8;
   is_mg |= d[0];
   
   rb=0;
   if ( (is_mg==BDIO_HASH_MAGIC_S)||(is_mg==BDIO_HASH_MAGIC_C) )
   {
      rb=fread(digest,1,16,fh->fp);
   }
   fseek(fh->fp,fpos,SEEK_SET);
   if(rb==16)
      return 1;
   else
      return 0;
}


void bdio_perror(const char *s, BDIO *fh)
{
   if( fh==NULL || fh->nerror==0)
      return;
   if( s != NULL )
      fprintf(fh->msg,"%s%s\n",s,fh->error);
   else
      fprintf(fh->msg,"%s\n",fh->error);
   return;
}

void bdio_pferror(const char *s, BDIO *fh)
{
   if( fh==NULL || fh->nerror==0)
      return;
   if( s != NULL )
      fprintf(fh->msg,"%s%s\n",s,fh->ferror);
   else
      fprintf(fh->msg,"%s\n",fh->ferror);
   return;
}

BDIO *bdio_open(const char* file, const char* mode, char* protocol_info)
{
   BDIO *fh;
   int wr;
   char errormsg[256];
   int last_hcnt;
   long fpos;

   if( default_msg==NULL )
      default_msg = stderr;
   
   fh = (BDIO*) malloc(sizeof(BDIO));
   if( fh==NULL )
   {
      return NULL;
   }


   fh->msg = default_msg;
   fh->verbose = default_verbosity;
   fh->nerror = 0;
   fh->error[0] = 0;
   fh->ferror[0]= 0;
   fh->hash_auto=BDIO_NO_HASH;
   fh->hash_mode=BDIO_HASH_SINGL;

   /* test the machine for compatibility */
   if( sizeof(int32_t) != 4 )
   {
      bdio_error(0,"Error in bdio_open. sizeof(int32_t) is not 4 bytes.",fh);
      free(fh);
      return NULL;
   }

   if( sizeof(int64_t) != 8 )
   {
      bdio_error(0,"Error in bdio_open. sizeof(int64_t) is not 8 bytes.",fh);
      free(fh);
      return NULL;
   }

   if( sizeof(float) != 4 )
   {
      bdio_error(0,"Error in bdio_open. sizeof(float) is not 4 bytes.",fh);
      free(fh);
      return NULL;
   }

   if( sizeof(double) != 8 )
   {
      bdio_error(0,"Error in bdio_open. sizeof(double) is not 8 bytes.",fh);
      free(fh);
      return NULL;
   }
   
   switch( *mode )
   {
      case 'r': fh->mode = BDIO_R_MODE;
                break;
      case 'w': fh->mode = BDIO_W_MODE;
                break;
      case 'a': fh->mode = BDIO_A_MODE;
                break;
      default:  bdio_error(0,"Error in bdio_open. Unknown mode.",fh);
                free(fh);
                return NULL;
   }

   if( mode[1] != 0 )
   {
      /* r+ w+ and a+ modes do not exist in bdio */
      bdio_error(0,"Error in bdio_open. Unknown mode",fh);
      free(fh);
      return NULL;
   }

   /* determine machine endiannes */
   switch( isBigEndian() )
   {
      case 0: fh->endian = BDIO_LEND;
              break;
      case 1: fh->endian = BDIO_BEND;
              break;
      default:bdio_error(0,"Error in bdio_open. Machine has unknown endianess.",
                         fh);
                 free(fh);
                 return NULL;
   }

   fh->buf = (unsigned char*) malloc(sizeof(char)*(BDIO_BUF_SIZE));
   if( fh==NULL)
   {
      bdio_error(1,"Error in bdio_open. malloc fails with",fh);
      free(fh);
      return NULL;
   }

   if( fh->mode == BDIO_W_MODE )
   {
      if( protocol_info==NULL )
      {
         bdio_error(0,"Error in bdio_open. No protocol info supplied.",fh);
         free(fh->buf);
         free(fh);
         return NULL;
      }
      if( (fh->fp = fopen(file, "w+"))==NULL )
      {
         sprintf(errormsg,
              "Error in bdio_open. Cannot open %s for writing. fopen fails with"
                                                                    ,file);
         bdio_error(1,errormsg,fh);
         free(fh->buf);
         free(fh);
         return NULL;
      }

      fh->hcnt = 0;
      fh->rcnt = 0;
      fh->hstart = 0;
      fh->rstart = 0;
      fh->bufstart= 0;
      fh->bufidx = 0;
      if( write_header(fh, protocol_info) != 0 )
      {
         fclose(fh->fp);
         free(fh->buf);
         free(fh);
         return NULL;
      }
      return fh;
   }
   if (fh->mode == BDIO_R_MODE )
   {
      if( (fh->fp = fopen(file, "r"))==NULL )
      {
         sprintf(errormsg,
              "Error in bdio_open. Cannot open %s for reading. fopen fails with"
                                                                         ,file);
         bdio_error(1,errormsg,fh);
         free(fh->buf);
         free(fh);
         return NULL;
      }
      /* initialize some  header fields */
      fh->hcnt = 0;
      fh->rcnt = 0;
      fh->rstart= 0;
      fh->rlen = 0;
      fh->ridx = 0;
      fh->bufstart = 0;
      fh->bufidx = 0;
      fh->hcuser = NULL;
      if( read_header(fh) != 0 )
      {
         bdio_error(0,"Error in bdio_open. Could not read header.",fh);
         free(fh->buf);
         fclose(fh->fp);
         free(fh);
         return NULL;
      }
      if( protocol_info != NULL )
      {
         if(strcmp(protocol_info,fh->hpinfo)!=0)
         {
            bdio_error(0,"Error in bdio_open. protocol_info does not match"
                          " first header's.",fh);
            free(fh->buf);
            free(fh->hcuser);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }
      }
      return fh;
   }
   if (fh->mode == BDIO_A_MODE )
   {
      /* apend mode: open file scroll to the end, update last header.
       *             if the last entry was a record: fh->state= BDIO_R_STATE
       *             else fh->state = BDIO_H_STATE
       */
      /* initialize some  header fields */
      fh->hcnt = 0;
      fh->rcnt = 0;
      fh->rstart= 0;
      fh->rlen = 0;
      fh->ridx = 0;
      fh->bufstart = 0;
      fh->bufidx = 0;
      fh->hcuser = NULL;

      if( (fh->fp = fopen(file, "r+"))==NULL )
      {
         if( errno!=ENOENT )
         {
            sprintf(errormsg,
              "Error in bdio_open. Cannot open %s in r+ mode. fopen fails with"
                                                                         ,file);
            bdio_error(1,errormsg,fh);
            free(fh->buf);
            free(fh);
            return NULL;
         }else
         {
            if( (fh->fp = fopen(file, "w+"))==NULL )
            {
               sprintf(errormsg,
                "Error in bdio_open. Cannot open %s in w mode. fopen fails with"
                                                                         ,file);
               bdio_error(1,errormsg,fh);
               free(fh->buf);
               free(fh);
               return NULL;
            }
         }
      }
      /* read in first 4 bytes to decide whether file is empty or not */
      wr = fread( fh->buf, 1, 4, fh->fp );
      if ( wr != 4 )
      {
         if( feof(fh->fp))
         {
            if( wr!=0 )
            {
               bdio_error(0,"Error in bdio_open. Unexpected EOF.",fh);
               free(fh->buf);
               fclose(fh->fp);
               free(fh);
               return NULL;
            }
            /* empty file - start new bdio file */
            /* write new header */
            if( protocol_info==NULL )
            {
               bdio_error(0,"Error in bdio_open. No protocol info supplied."
                                                                           ,fh);
               fclose(fh->fp);
               free(fh->buf);
               free(fh);
               return NULL;
            }
            if( write_header(fh, protocol_info) != 0 )
            {
               fclose(fh->fp);
               free(fh->buf);
               free(fh);
               return NULL;
            }
            return fh;
         }
         else
         {
            bdio_error(1,"Error in bdio_open. fread fails with",fh);
            free(fh->buf);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }
      }else
      {
         /* file is not empty */
         fh->bufidx = 4;
         fh->ridx = 4;
         if( read_header(fh) != 0 )
         {
            bdio_error(0,"Error in bdio_open. Could not read header.",fh);
            free(fh->buf);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }

         fh->mode = BDIO_R_MODE;
         last_hcnt = fh->hcnt;
         while( (fh->state == BDIO_R_STATE) || (fh->state == BDIO_H_STATE) )
         {
            last_hcnt = fh->hcnt;
            if( bdio_seek_record(fh)==EOF )
            if( fh->state == BDIO_E_STATE )
            {
               free(fh->hcuser);
               free(fh->buf);
               fclose(fh->fp);
               free(fh);
               return NULL;
            }
         }
         if( protocol_info != NULL)
         {
            if(strcmp(protocol_info,fh->hpinfo)!=0)
            {
               bdio_error(0,"Error in bdio_open. protocol_info does not match"
                                                          " last header's.",fh);
               free(fh->buf);
               free(fh->hcuser);
               fclose(fh->fp);
               free(fh);
               return NULL;
            }
         }
         if( last_hcnt < fh->hcnt )
            fh->state=BDIO_H_STATE;
         else
            fh->state=BDIO_N_STATE;

         /* update last header */
         fpos=fh->rstart+fh->rlen;
         if( fseek(fh->fp, fh->hstart, SEEK_SET)!=0 )
         {
            bdio_error(1,"Error in bdio_open. fseek fails with",fh);
            free(fh->buf);
            free(fh->hcuser);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }

         if( update_header(fh) != 0 )
         {
            free(fh->buf);
            free(fh->hcuser);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }
         
         if( fseek(fh->fp, fpos, SEEK_SET)!=0 )
         {
            bdio_error(1,"Error in bdio_open. fseek fails with",fh);
            free(fh->buf);
            free(fh->hcuser);
            fclose(fh->fp);
            free(fh);
            return NULL;
         }
         fh->mode = BDIO_A_MODE;
         return fh;
      }
   }
   return fh;
}


int bdio_close(BDIO *fh)
{
   int ret;
   if( !is_valid_bdio("bdio_close", fh) )
   {
      if( fh==NULL )
         return EOF;
      else
      {
         bdio_error(0,"Error in bdio_close. Stream is in error state.",fh);
         ret = fclose( fh->fp );
         if( ret==EOF )
            bdio_error(1,"Error in bdio_close. fclose fails with",fh);
         if( fh->hcuser!=0 )
            free( fh->hcuser );
         if( fh->buf!=0 )
            free( fh->buf );
         fh->state = -1;
         free( fh );
         return EOF;
      }
   }
   if( (fh->mode == BDIO_W_MODE)  || (fh->mode == BDIO_A_MODE) )
   {
      if( bdio_flush_record( fh )!=0)
      {
         bdio_error(0,"Error in bdio_close. Could not flush.",fh);
         ret = fclose( fh->fp );
         if( ret==EOF )
            bdio_error(1,"Error in bdio_close. fclose fails with",fh);
         free( fh->hcuser );
         free( fh->buf );
         fh->state = -1;
         free( fh );
         return EOF;
      }
   }
   ret = fclose( fh->fp );
   if( ret==EOF )
   {
      bdio_error(1,"Error in bdio_close. fclose fails with",fh);
      free( fh->hcuser );
      free( fh->buf );
      fh->state = -1;
      free( fh );
      return EOF;
   }
   free( fh->hcuser );
   free( fh->buf );
   fh->state = -1;
   free( fh );
   return ret;
}

void bdio_set_user(char* u)
{
   int len;
   len = strlen(u)+1;
   if( len < BDIO_MAX_USER_LENGTH )
   {
      strcpy(user_str,u);
   }
   else
   {
      strncpy(user_str,u,BDIO_MAX_USER_LENGTH-1);
      user_str[BDIO_MAX_USER_LENGTH-1]='\0';
   }
}

void bdio_set_host(char* u)
{
   int len;
   len = strlen(u)+1;
   if( len < BDIO_MAX_HOST_LENGTH )
   {
      strcpy(host_str,u);
   }
   else
   {
      strncpy(host_str,u,BDIO_MAX_HOST_LENGTH-1);
      host_str[BDIO_MAX_HOST_LENGTH-1]='\0';
   }
}


void bdio_set_dflt_msg(FILE *stream)
{
   if( stream != NULL )
      default_msg = stream;
}

void bdio_set_dflt_verbose(int v)
{
   if( v==0 )
   {
      default_verbosity=0;
   } else
      default_verbosity=1;
   return;
}

int bdio_set_verbose(int v, BDIO *fh)
{
   if( !is_valid_bdio("bdio_set_verbose", fh) )
   {
      return EOF;
   }
   if( v==0 )
   {
      fh->verbose = 0;
      return 0;
   }
   if( v>0 )
   {
      fh->verbose = 1;
      return 0;
   }
   bdio_error(0,"Error in bdio_set_verbose. Argument out of range",fh);
   return EOF;
}

int bdio_set_msg(FILE *stream, BDIO *fh)
{
   if( !is_valid_bdio("bdio_set_verbose", fh) )
   {
      return EOF;
   }
   if( stream != NULL)
   {
      fh->msg = stream;
      return 0;
   }else
      bdio_error(0,"Error in bdio_set_msg. NULL is an invalid stream.\n"
                   "Use set_verbose to turn off error output.",fh);
   return EOF;
}

char* bdio_get_hchost(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return NULL;
   }
   return fh->hchost;
}

char* bdio_get_hcuser(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return NULL;
   }
   return fh->hcuser;
}

char* bdio_get_hmhost(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return NULL;
   }
   return fh->hmhost;
}

char* bdio_get_hmuser(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return NULL;
   }
   return fh->hmuser;
}

char* bdio_get_hpinfo(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return NULL;
   }
   return fh->hpinfo;
}

int bdio_get_hversion(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hversion", fh) )
   {
      return EOF;
   }
   return fh->hversion;
}
   
int bdio_get_hcnt(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hcnt", fh) )
   {
      return EOF;
   }
   return fh->hcnt;
}

int bdio_get_hcdate(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hcdate", fh) )
   {
      return EOF;
   }
   return fh->hcdate;
}

int bdio_get_hmdate(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_hmdate", fh) )
   {
      return EOF;
   }
   return fh->hmdate;
}

int bdio_get_ruinfo(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_ruinfo", fh) )
   {
      return EOF;
   }
   if( fh->state == BDIO_R_STATE )
      return fh->ruinfo;
   bdio_error(0, "Error in bdio_get_ruinfo. Currently not in a data record.",fh);
   return EOF;
}

int bdio_get_rfmt(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_rtype", fh) )
   {
      return EOF;
   }
   if( fh->state == BDIO_R_STATE )
      return fh->rfmt;
   bdio_error(0, "Error in bdio_get_rfmt. Currently not in a data record.",fh);
   return EOF;
}

uint64_t bdio_get_rlen(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_rlen", fh) )
   {
      return EOF;
   }
   if( fh->state == BDIO_R_STATE )
   {
      if ( fh->rlongrec )
         return fh->rlen-8;
      else
         return fh->rlen-4;
   }
   return 0;
}
   
int bdio_get_rcnt(BDIO *fh)
{
   if( !is_valid_bdio("bdio_get_rcnt", fh) )
   {
      return EOF;
   }
   return fh->rcnt;
}

int bdio_is_in_record(BDIO *fh)
{
   if( !is_valid_bdio("bdio_is_in_record", fh) )
   {
      return 0;
   }
   if( fh->state == BDIO_R_STATE )
      return 1;
   else
      return 0;
}

int bdio_is_in_header(BDIO *fh)
{
   if( !is_valid_bdio("bdio_is_in_header", fh) )
   {
      return 0;
   }
   if( fh->state == BDIO_H_STATE )
      return 1;
   else
      return 0;
}




int bdio_append_record(int fmt, int uinfo, BDIO *fh)
{
   if( !is_valid_bdio("bdio_append_record", fh) )
   {
      return EOF;
   }

   if( (fh->mode != BDIO_W_MODE)  && (fh->mode != BDIO_A_MODE) )
   {
      bdio_error(0,
                "Error in bdio_append_record. Not in write or append mode.",fh);
      return EOF;
   }
   
   if( fh->state != BDIO_N_STATE )
   {
      bdio_error(0,"Error in bdio_append_record. Not at the end of file.",fh);
      return EOF;
   }

   if( fh->rstart == fh->hstart )
   {
      bdio_error(0,"Error in bdio_append_record. Last entry in file is not a"
                   " record.",fh);
      return EOF;
   }

   /* assume that meta-data of last record are still up to date despite of
    * being in N-state
    */
   if( fh->ruinfo != uinfo )
   {
      bdio_error(0,"Error in bdio_append_record. uinfo does not "
                   "match previous record's.",fh);
      return EOF;
   }
   /* TODO: typen ohne endianness: check ob sie zur maschine passen */
   if( fmt != fh->rfmt )
   {
      if( ! (  (fmt==BDIO_BIN_INT32 &&
               ((fh->rfmt==BDIO_BIN_INT32BE)||(fh->rfmt==BDIO_BIN_INT32LE)) )
             ||(fmt==BDIO_BIN_INT64 &&
               ((fh->rfmt==BDIO_BIN_INT64BE)||(fh->rfmt==BDIO_BIN_INT64LE)) )
             ||(fmt==BDIO_BIN_F32 &&
               ((fh->rfmt==BDIO_BIN_F32BE)||(fh->rfmt==BDIO_BIN_F32LE)) )
             ||(fmt==BDIO_BIN_F64 &&
               ((fh->rfmt==BDIO_BIN_F64BE)||(fh->rfmt==BDIO_BIN_F64LE)) ) )
        )
      {
         bdio_error(0,"Error in bdio_append_record. fmt does not match"
                      " previous record's.",fh);
         return EOF;
      }
   }
   fh->state = BDIO_R_STATE;
   fh->bufstart = fh->rlen;
   fh->bufidx = 0;
   return 0;
}


int bdio_seek_record(BDIO *fh)
{
   int rd;
   uint32_t hdr;
   uint64_t lhdr;
   if( !is_valid_bdio("bdio_seek_record", fh) )
   {
      return EOF;
   }

   if( fh->mode != BDIO_R_MODE )
   {
      bdio_error(0, "Error in bdio_seek_record. Not in read mode.",fh);
      return EOF;
   }
   
   if( (fh->state == BDIO_R_STATE) || (fh->state == BDIO_H_STATE))
   {
      if( fseek(fh->fp, fh->rlen-fh->ridx, SEEK_CUR)==-1 )
      {
         bdio_error(1,"Error in bdio_seek_record. fseek fails with",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      fh->ridx = fh->rlen;
   }

   
   /* read the header of the following record*/
   rd = fread( fh->buf, 1, 4, fh->fp);
   if ( feof( fh->fp ) )
   {
      /* clean EOF reached */
      clearerr( fh->fp );
      fh->state = BDIO_N_STATE;
      return EOF;
   }
   if ( rd != 4 )
   {
      bdio_error(1,"Error in bdio_seek_record. fread fails with.",fh);
      fh->state = BDIO_E_STATE;
      return EOF;
   }
   fh->rstart = fh->rstart+fh->rlen;
   fh->ridx=4;
   memcpy(&hdr,fh->buf,4);
   if (fh->endian == BDIO_BEND)
         swap32(&hdr,4);
   while( !(hdr & 0x00000001) )
   {
      /* must be a header */
      if( read_header(fh) != 0 )
      {
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      /* seek next record */
      if( fseek(fh->fp, fh->rlen-fh->ridx, SEEK_CUR)==-1 )
      {
         bdio_error(1,"Error in bdio_seek_record. fseek failed with",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      fh->ridx=fh->rlen;
      /* read the header of the following record (version 2 in the notes) */
      rd = fread( fh->buf, 1, 4, fh->fp);
      if ( feof( fh->fp ) )
      {
         /* clean EOF reached */
         clearerr( fh->fp );
         fh->state = BDIO_N_STATE;
         return 0;
      }
      if ( rd != 4 )
      {
         bdio_error(1,"Error in bdio_seek_record. fread fails with",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      memcpy(&hdr,fh->buf,4);
      if (fh->endian == BDIO_BEND)
         swap32(&hdr,4);
      fh->rstart = fh->rstart+fh->rlen;
      fh->ridx = 4;
   }
   /* can only be a data record */
   fh->rcnt++;
   fh->rlongrec = (hdr & 0x00000008)>>3;
   if (fh->rlongrec )
   {
      /* need next 4 bytes to determine length */
      rd = fread( &(fh->buf[4]), 1, 4, fh->fp);
      if ( feof( fh->fp ) )
      {
         bdio_error(1,"Error in bdio_seek_record. Unexpected EOF.",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      if ( rd != 4 )
      {
         bdio_error(1,"Error in bdio_seek_record. fread fails with",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      memcpy(&lhdr, fh->buf, 8);
      if (fh->endian == BDIO_BEND)
         swap64(&lhdr,8);
      fh->rfmt     =  (int) ((lhdr & 0x00000000000000f0)>>4);
      fh->ruinfo   =  (int) ((lhdr & 0x0000000000000f00)>>8);
      fh->rlen     = ((lhdr & 0xfffffffffffff000)>>12) + 8;
      fh->ridx     = 8;
   }else
   {
      fh->rfmt     =  (hdr & 0x000000f0)>>4;
      fh->ruinfo   =  (hdr & 0x00000f00)>>8;
      fh->rlen     = ((hdr & 0xfffff000)>>12) + 4;
   }

   /* find out whether on this machine swapping of the byte order will be
    * necessary after reading from disk
    */
   fh->rswap=0;
   if(  ((fh->rfmt==BDIO_BIN_INT32LE) && (fh->endian==BDIO_BEND))
      ||((fh->rfmt==BDIO_BIN_INT32BE) && (fh->endian==BDIO_LEND))
      ||((fh->rfmt==BDIO_BIN_F32LE)   && (fh->endian==BDIO_BEND))
      ||((fh->rfmt==BDIO_BIN_F32BE)   && (fh->endian==BDIO_LEND))
      ||((fh->rfmt==BDIO_BIN_INT64LE) && (fh->endian==BDIO_BEND))
      ||((fh->rfmt==BDIO_BIN_INT64BE) && (fh->endian==BDIO_LEND))
      ||((fh->rfmt==BDIO_BIN_F64LE)   && (fh->endian==BDIO_BEND))
      ||((fh->rfmt==BDIO_BIN_F64BE)   && (fh->endian==BDIO_LEND)) )
   {
      fh->rswap=1;
   }
   /* find out whether data items will have 1, 4 or 8 bytes */
   fh->rdsize=1;
   if(  (fh->rfmt==BDIO_BIN_INT32LE) || (fh->rfmt==BDIO_BIN_INT32BE)
      ||(fh->rfmt==BDIO_BIN_F32LE)   || (fh->rfmt==BDIO_BIN_F32BE) )
   {
      fh->rdsize=4;
   }
   if(  (fh->rfmt==BDIO_BIN_INT64LE) || (fh->rfmt==BDIO_BIN_INT64BE)
      ||(fh->rfmt==BDIO_BIN_F64LE)   || (fh->rfmt==BDIO_BIN_F64BE) )
   {
      fh->rdsize=8;
   }
   fh->state  = BDIO_R_STATE;
   return 0;
}


size_t bdio_read(void *buf, size_t nb, BDIO *fh)
{
   size_t rd=0;
   
   if( !is_valid_bdio("bdio_read", fh) )
   {
      return 0;
   }
   if( fh->state != BDIO_R_STATE )
   {
      bdio_error(0, "Error in bdio_read. No record seeked.",fh);
      return 0;
   }
   if( fh->mode != BDIO_R_MODE )
   {
      bdio_error(0, "Error in bdio_read. Not in read mode.",fh);
      return 0;
   }

   if(nb%fh->rdsize!=0)
   {
      bdio_error(0, "Error in bdio_read. nb is not multiple of data size.",fh);
      return 0;
   }

   if( nb > (fh->rlen-fh->ridx) )
   {
      bdio_error(0,"Error in bdio_read. nb is larger than remaining data in"
                   " the record.",fh);
      return 0;
      /* TODO: maybe better: read as much as possible? */
   }
   
   rd =  fread(buf, 1, nb, fh->fp);
   if( rd<nb )
   {
      if ( feof( fh->fp ) )
         bdio_error(0, "Error in bdio_read. Unexpected EOF.",fh);
      else
         bdio_error(1, "Error in bdio_read. fread fails with",fh);
      /*TODO set error state? */
   }
   /* update fh */
   fh->ridx += rd;
   return( rd );
}

size_t bdio_read_f32(float *buf, size_t nb, BDIO *fh)
{
   size_t rd;
   if(   (fh->rfmt != BDIO_BIN_F32BE) && (fh->rfmt != BDIO_BIN_F32LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_read_f32. Record has incompatible format",fh);
      return(0);
   }
   rd=bdio_read((void*) buf, nb, fh);
   if( fh->rswap )
      swap32(buf, rd);
   return rd;
}

size_t bdio_read_f64(double *buf, size_t nb, BDIO *fh)
{
   size_t rd;
   if(   (fh->rfmt != BDIO_BIN_F64BE) && (fh->rfmt != BDIO_BIN_F64LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_read_f64. Record has incompatible format",fh);
      return(0);
   }
   rd=bdio_read((void*) buf, nb, fh);
   if( fh->rswap )
      swap64(buf, rd);
   return rd;
}

size_t bdio_read_int32(int32_t *buf, size_t nb, BDIO *fh)
{
   size_t rd;
   if(   (fh->rfmt != BDIO_BIN_INT32BE) && (fh->rfmt != BDIO_BIN_INT32LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_read_i32. Record has incompatible format",fh);
      return(0);
   }
   rd=bdio_read((void*) buf, nb, fh);
   if( fh->rswap )
      swap32(buf, rd);
   return rd;
}

size_t bdio_read_int64(int64_t *buf, size_t nb, BDIO *fh)
{
   size_t rd;
   if(   (fh->rfmt != BDIO_BIN_INT64BE) && (fh->rfmt != BDIO_BIN_INT64LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_read_i64. Record has incompatible format",fh);
      return(0);
   }
   rd=bdio_read((void*) buf, nb, fh);
   if( fh->rswap )
      swap64(buf, rd);
   return rd;
}

int bdio_start_record(int fmt, int uinfo, BDIO *fh)
{
   uint32_t hdr;
   if( !is_valid_bdio("bdio_start_record", fh) )
   {
      return EOF;
   }

   if( fmt != BDIO_BIN_GENERIC &&
       fmt != BDIO_ASC_EXEC    &&
       fmt != BDIO_BIN_INT32BE &&
       fmt != BDIO_BIN_INT32LE &&
       fmt != BDIO_BIN_INT64BE &&
       fmt != BDIO_BIN_INT64LE &&
       fmt != BDIO_BIN_F32BE   &&
       fmt != BDIO_BIN_F32LE   &&
       fmt != BDIO_BIN_F64BE   &&
       fmt != BDIO_BIN_F64LE   &&
       fmt != BDIO_ASC_GENERIC &&
       fmt != BDIO_ASC_XML     &&
       fmt != BDIO_BIN_INT32   &&
       fmt != BDIO_BIN_INT64   &&
       fmt != BDIO_BIN_F32     &&
       fmt != BDIO_BIN_F64)
   {
      bdio_error(0,"Error in bdio_start_record. Unknown format.",fh);
      return EOF;
   }

   if( ((unsigned int) uinfo) > 15 )
   {
      bdio_error(0,"Error in bdio_start_record. Info out of range.",fh);
      return EOF;
   }

   if( (fh->mode != BDIO_W_MODE)  && (fh->mode != BDIO_A_MODE) )
   {
      bdio_error(0,
                 "Error in bdio_start_record. Not in write or append mode.",fh);
      /*fh->state = BDIO_E_STATE; */ /*TODO: stay in wrong state? */
      return EOF;
   }
   
   if( bdio_flush_record(fh) == EOF )
   {
      fh->state = BDIO_E_STATE;
      return EOF;
   }

   /* include endiannes in format, if not specified by user */
   if( (fmt==BDIO_BIN_INT32) && (fh->endian==BDIO_LEND))
      fmt=BDIO_BIN_INT32LE;
   if( (fmt==BDIO_BIN_INT32) && (fh->endian==BDIO_BEND))
      fmt=BDIO_BIN_INT32BE;
   if( (fmt==BDIO_BIN_INT64) && (fh->endian==BDIO_LEND))
      fmt=BDIO_BIN_INT64LE;
   if( (fmt==BDIO_BIN_INT64) && (fh->endian==BDIO_BEND))
      fmt=BDIO_BIN_INT64BE;
   if( (fmt==BDIO_BIN_F32) && (fh->endian==BDIO_LEND))
      fmt=BDIO_BIN_F32LE;
   if( (fmt==BDIO_BIN_F32) && (fh->endian==BDIO_BEND))
      fmt=BDIO_BIN_F32BE;
   if( (fmt==BDIO_BIN_F64) && (fh->endian==BDIO_LEND))
      fmt=BDIO_BIN_F64LE;
   if( (fmt==BDIO_BIN_F64) && (fh->endian==BDIO_BEND))
      fmt=BDIO_BIN_F64BE;

   /* find out whether on this machine swapping of the byte order will be
    * necessary before writing to disk
    */
   fh->rswap=0;
   if(  ((fmt==BDIO_BIN_INT32LE) && (fh->endian==BDIO_BEND))
      ||((fmt==BDIO_BIN_INT32BE) && (fh->endian==BDIO_LEND))
      ||((fmt==BDIO_BIN_F32LE)   && (fh->endian==BDIO_BEND))
      ||((fmt==BDIO_BIN_F32BE)   && (fh->endian==BDIO_LEND))
      ||((fmt==BDIO_BIN_INT64LE) && (fh->endian==BDIO_BEND))
      ||((fmt==BDIO_BIN_INT64BE) && (fh->endian==BDIO_LEND))
      ||((fmt==BDIO_BIN_F64LE)   && (fh->endian==BDIO_BEND))
      ||((fmt==BDIO_BIN_F64BE)   && (fh->endian==BDIO_LEND)) )
   {
      fh->rswap=1;
   }
   /* find out whether data items will have 1, 4 or 8 bytes */
   fh->rdsize=1;
   if(  (fmt==BDIO_BIN_INT32LE) || (fmt==BDIO_BIN_INT32BE)
      ||(fmt==BDIO_BIN_F32LE)   || (fmt==BDIO_BIN_F32BE) )
   {
      fh->rdsize=4;
   }
   if(  (fmt==BDIO_BIN_INT64LE) || (fmt==BDIO_BIN_INT64BE)
      ||(fmt==BDIO_BIN_F64LE)   || (fmt==BDIO_BIN_F64BE) )
   {
      fh->rdsize=8;
   }

   /* start new record */

   /* first 4 byte of a data record (as it appears in the file):
    *
    *        bit7                        bit0
    *         |                           |
    *         v                           v
    *byte0: [f3  f2  f1  f0  rt  sp  sp  m  ]
    *byte1: [l3  l2  l1  l0  u3  u2  u1  u0 ]
    *byte2: [l11 l10 l9  l8  l7  l6  l5  l4 ]
    *byte3: [l19 l18 l17 l16 l15 l14 l13 l12]
    *
    * with:
    * m:   magic bit must be 1
    * sp:  spare bit
    * rt:  0=short record (always=0 at creation)
    * f:   format, If bp==0
    *               0x0: generic binary
    *               0x1: executable
    *               0x2: int32, big endian
    *               0x3: int32, little endian
    *               0x4: int64, big endian
    *               0x5: int64, little endian
    *               0x6: float32, big endian
    *               0x7: float32, little endian
    *               0x8: float64, big endian
    *               0x9: float64, little endian
    *               0xA: generic ascii
    *               0xB: XML
    *               0xC: spare
    *               0xD: spare
    *               0xE: spare
    *               0xF: spare
    *              If bp==1: used for additional bits of record length
    *               f0=l24
    *               f1=l25
    *               f2=l26
    *               f3=l27
    * u:  user defined
    * l:  length of record (excluding header), 0..2^20-1
    * 
    */

   fh->rlongrec = 0;
   fh->rfmt = fmt;
   fh->ruinfo = uinfo;
   fh->rstart = fh->rstart+fh->rlen;
   fh->state  = BDIO_R_STATE;
   fh->rcnt++;

   fh->rlen = 4;
   fh->ridx = 4;

   hdr = HEADER_INT(fh->rfmt, fh->ruinfo, fh->rlen);
   if (fh->endian == BDIO_BEND)
         swap32(&hdr,4);
   memcpy(fh->buf,&hdr, 4);
   fh->bufstart = 0;
   fh->bufidx = 4;
   
   if (fh->hash_auto) MD5_Init(fh->hash);
   if (fh->hash_mode==BDIO_HASH_CHAIN)
      MD5_Update(fh->hash, fh->prev_digest, 16);
   
   return 0;
}


size_t bdio_write(void *ptr, size_t nb, BDIO *fh)
{
   size_t nw=0;
   size_t nr;
   uint64_t lhdr;

   if( !is_valid_bdio("bdio_write", fh) )
   {
      return 0;
   }
   
   if( fh->state != BDIO_R_STATE )
   {
      bdio_error(0, "Error in bdio_write. No record started.",fh);
      /*fh->state = BDIO_E_STATE; */ /*TODO: stay in wrong state? */
      return 0;
   }
   if( (fh->mode != BDIO_W_MODE)  && (fh->mode != BDIO_A_MODE) )
   {
      bdio_error(0, "Error in bdio_write. Not in write or append mode.",fh);
      /*fh->state = BDIO_E_STATE; */ /*TODO: stay in wrong state? */
      return 0;
   }
   if( nb%fh->rdsize != 0 )
   {
      bdio_error(0, "Error in bdio_write. nb is not multiple of data size.",fh);
      return 0;
   }
   
   if (fh->hash_auto)
      MD5_Update(fh->hash, ptr, nb);

   if( !(fh->rlongrec) && (fh->ridx+nb)>(BDIO_MAX_RECORD_LENGTH+4) )
   {
      /* a short record must be turned into a long record */
      if( fh->ridx==4 )
      {
         /* case 1: No data in the record yet. Enlarge header by 4 bytes */
         fh->rlongrec = 1;
         fh->ridx += 4;
         fh->rlen += 4;
         if( fh->bufstart==0 )
            fh->bufidx += 4;
         else
         {
            /* this happens if one appends to an empty record */
            if( fseek(fh->fp, -4, SEEK_CUR) == -1)
            {
               bdio_error(1, "Error in bdio_write. fseek failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            fh->bufstart = 0;
            fh->bufidx = 8;
         }
      } else
      if( fh->bufstart == 0 )
      {
         if( fh->bufidx < BDIO_BUF_SIZE-4 )
         {
            /* case 2: All data is still buffered. Shift buffer by 4 bytes */
            memmove(&(fh->buf[8]),&(fh->buf[4]),fh->bufidx-4);
            fh->rlongrec = 1;
            fh->bufidx += 4;
            fh->rlen += 4;
            fh->ridx += 4;
         }else
         {
            /* case 3: All data is still buffered. Shift buffer-start by 4 */
            /* write a header that is up-to-date after this write */
            lhdr = HEADER_INT_LONG(fh->rfmt, fh->ruinfo, fh->ridx+nb+4);
            if (fh->endian == BDIO_BEND)
               swap64(&lhdr,8);
            if( fwrite(&lhdr,1,8,fh->fp) != 8 )
            {
               bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            if( fwrite(&(fh->buf[4]),1,fh->bufidx-4,fh->fp) != (fh->bufidx-4) )
            {
               bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            fh->rlongrec = 1;
            fh->bufstart = fh->bufidx+4;
            fh->bufidx = 0;
            fh->ridx += 4;
            fh->rlen += 4;
         }
      } else
      {
         /* case 3: move record-content in the file by 4 bytes */
         /* write dummy 4 bytes followed by contents of the buffer */
         if( fwrite(fh->buf,1,4,fh->fp) != 4 )
         {
               bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
         }
         if( fwrite(fh->buf,1,fh->bufidx,fh->fp) != fh->bufidx )
         {
               bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
         }

         /* shift blocks of data 4 bytes down */
         while( BDIO_BUF_SIZE < fh->bufstart )
         {
            nr = BDIO_BUF_SIZE;
            if( (fh->bufstart-nr) < 4 )
               nr-=4;
            if( fseek(fh->fp, -(nr+4+fh->bufidx), SEEK_CUR) == -1)
            {
               bdio_error(1, "Error in bdio_write. fseek failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            if( fread(fh->buf, 1, nr, fh->fp) != nr )
            {
               bdio_error(1, "Error in bdio_write. fread failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            if( fseek(fh->fp, -nr+4, SEEK_CUR) == -1)
            {
               bdio_error(1, "Error in bdio_write. fseek failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            if( fwrite(fh->buf, 1, nr, fh->fp) != nr )
            {
               bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
               fh->state=BDIO_E_STATE;
               return 0;
            }
            fh->bufstart -= nr;
            fh->bufidx = nr;
         }
         /* shift last block 4 bytes down, write new header */
         nr = fh->bufstart-4;
         if( fseek(fh->fp, fh->rstart+4, SEEK_SET) == -1)
         {
            bdio_error(1, "Error in bdio_write. fseek failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         if( fread(fh->buf, 1, nr, fh->fp) != nr )
         {
            bdio_error(1, "Error in bdio_write. fread failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         if( fseek(fh->fp, fh->rstart, SEEK_SET) == -1)
         {
            bdio_error(1, "Error in bdio_write. fseek failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         /* write header that is up-to-date after this write */
         lhdr = HEADER_INT_LONG(fh->rfmt, fh->ruinfo, fh->ridx+nb+4);
         if (fh->endian == BDIO_BEND)
            swap64(&lhdr,8);
         if( fwrite(&lhdr,1,8,fh->fp) != 8 )
         {
            bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         if( fwrite(fh->buf, 1, nr, fh->fp) != nr )
         {
            bdio_error(1, "Error in bdio_write. fwrite failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         if( fseek(fh->fp, 0, SEEK_END) == -1)
         {
            bdio_error(1, "Error in bdio_write. fseek failed with",fh);
            fh->state=BDIO_E_STATE;
            return 0;
         }
         fh->rlongrec = 1;
         fh->ridx    += 4;
         fh->rlen    += 4;
         fh->bufidx   = 0;
         fh->bufstart = fh->ridx;
      }
   }
   nw = buf_write(ptr,nb,fh);
   return nw;
}

size_t bdio_write_f32(float *ptr, size_t nb, BDIO *fh)
{
   /*TODO: come up with something to avoid double-swaps*/
   size_t nw;
   if(   (fh->rfmt != BDIO_BIN_F32BE) && (fh->rfmt != BDIO_BIN_F32LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_write_f32. Record has incompatible format",fh);
      return(0);
   }
   if( fh->rswap )
      swap32(ptr, nb);
   nw = bdio_write((void *) ptr, nb, fh);
   if( fh->rswap )
      swap32(ptr, nb);
   return nw;
}

size_t bdio_write_f64(double *ptr, size_t nb, BDIO *fh)
{
   /*TODO: come up with something to avoid double-swaps*/
   size_t nw;
   if(   (fh->rfmt != BDIO_BIN_F64BE) && (fh->rfmt != BDIO_BIN_F64LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_write_f64. Record has incompatible format",fh);
      return(0);
   }
   if( fh->rswap )
      swap64(ptr, nb);
   nw = bdio_write((void *) ptr, nb, fh);
   if( fh->rswap )
      swap64(ptr, nb);
   return nw;
}

size_t bdio_write_int32(int32_t *ptr, size_t nb, BDIO *fh)
{
   /*TODO: come up with something to avoid double-swaps*/
   size_t nw;
   if(   (fh->rfmt != BDIO_BIN_INT32BE) && (fh->rfmt != BDIO_BIN_INT32LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_write_i32. Record has incompatible format",fh);
      return(0);
   }
   
   if( fh->rswap )
      swap32(ptr, nb);
   nw = bdio_write((void *) ptr, nb, fh);
   if( fh->rswap )
      swap32(ptr, nb);
   return nw;
}

size_t bdio_write_int64(int64_t *ptr, size_t nb, BDIO *fh)
{
   /*TODO: come up with something to avoid double-swaps*/
   size_t nw;
   if(   (fh->rfmt != BDIO_BIN_INT64BE) && (fh->rfmt != BDIO_BIN_INT64LE)
      && (fh->rfmt != BDIO_BIN_GENERIC))
   {
      bdio_error(0,"Error in bdio_write_i64. Record has incompatible format",fh);
      return(0);
   }
   
   if( fh->rswap )
      swap64(ptr, nb);
   nw = bdio_write((void *) ptr, nb, fh);
   if( fh->rswap )
      swap64(ptr, nb);
   return nw;
}


int bdio_flush_record( BDIO *fh)
{
   size_t wr;
   uint32_t hdr;
   uint64_t lhdr;

   if( !is_valid_bdio("bdio_flush_record", fh) )
   {
      return EOF;
   }

   if( (fh->mode != BDIO_W_MODE)  && (fh->mode != BDIO_A_MODE) )
   {
      bdio_error(0,"Error in bdio_flush_record. Not in w or a mode.",fh);
      return 0;
   }
   if( fh->state == BDIO_R_STATE )
   {
      /* finish last record */
      if(fh->rlongrec)
      {
         lhdr = HEADER_INT_LONG(fh->rfmt, fh->ruinfo, fh->rlen);
         if (fh->endian == BDIO_BEND)
            swap64(&lhdr,8);
         /* update record header in file */
         if (fh->bufstart==0)
         {
            memcpy(fh->buf,&lhdr, 8);
         }
         else
         {
            if( fseek(fh->fp,-fh->bufstart,SEEK_CUR)==-1 )
            {
               bdio_error(1,"Error in bdio_flush_record. fseek fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
            wr=fwrite(&lhdr,8,1,fh->fp);
            if( wr!=1)
            {
               bdio_error(1,"Error in bdio_flush_record. fwrite fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
            /* Edge case: 0 < bufstart < 8 can't happen */
            if( fseek(fh->fp,fh->bufstart-8,SEEK_CUR)==-1 )
            {
               bdio_error(1,"Error in bdio_flush_record. fseek fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
         }
      }else
      {
         hdr  = HEADER_INT(fh->rfmt, fh->ruinfo, fh->rlen);
         if (fh->endian == BDIO_BEND)
            swap32(&hdr,4);
         /* update record header in file */
         if (fh->bufstart==0)
         {
            memcpy(fh->buf,&hdr, 4);
         }
         else
         {
            /*edge case: 0<bufidx<4 can't happen */
            if( fseek(fh->fp,-fh->bufstart,SEEK_CUR)==-1 )
            {
               bdio_error(1,"Error in bdio_flush_record. fseek fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
            wr=fwrite(&hdr,4,1,fh->fp);
            if( wr!=1)
            {
               bdio_error(1,"Error in bdio_flush_record. fwrite fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
            if( fseek(fh->fp,fh->bufstart-4,SEEK_CUR)==-1 )
            {
               bdio_error(1,"Error in bdio_flush_record. fseek fails with",fh);
               fh->state=BDIO_E_STATE;
               return EOF;
            }
         }
      }

      /* write content of buffer to disk */
      wr = fwrite(fh->buf, 1, fh->bufidx, fh->fp);
      if( wr != fh->bufidx)
      {
         bdio_error(1,
                      "Error in bdio_flush_record. fwrite fails with",fh);
         fh->state = BDIO_E_STATE;
         return EOF;
      }
      fh->bufstart=0;
      fh->bufidx=0;
      fh->state = BDIO_N_STATE;
      if( fh->hash_auto==BDIO_AUTO_HASH )
      {
         if( bdio_write_hash(fh) != 20)
         {
            bdio_error(1,"Error in bdio_flush_record. Could not write hash record.",fh);
            fh->state=BDIO_E_STATE;
            return EOF;
         }
      }
   }
   
   if( fh->state == BDIO_H_STATE )
   {
      fh->state = BDIO_N_STATE;
      /* nothing else to do, header records have correct lenth entry */
   }
   
   return 0;
}
