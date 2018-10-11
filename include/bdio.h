/** @file bdio.h
 *  @brief Header file for the bdio-library
 *  @details Details & license
 *  @version 1.0
 *  @author Tomasz Korzec, Alberto Ramos,  Hubert Simma
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


/** @mainpage
  @section intro Introduction
  This library provides tools for reading, writing and manipulating
  bdio files (<a href="http://www.bdio.org/bdio.pdf">bdio.pdf</a>). These are 
  binary files, structured into records, which contain a thin layer of 
  meta-data each. This meta-data allows to keep the files machine independent and
  browsable, while maintaining the advantages of raw binary I/O, i.e. 
  minimal file-size and decent speed.
   
  @section dep Dependencies
  The library should compile on standard UNIX systems with standard C99 compilers.<br>
  It depends on the standard C library, in particular
 - stdio.h
 - stdint.h (C99)
 - stdlib.h
 - string.h
 - time.h
 - errno.h
 .
 Furthermore the C POSIX library is needed
 - unistd.h
 - sys/types.h
 - sys/socket.h
 - netdb.h
 - pwd.h 
 @section install Compilation
 - Copy the source code to some folder
 - Edit the Makefile. Adjust the line
   @code CC=gcc -pedantic -fstrict-aliasing -Wall -Wno-long-long -Werror @endcode
   to your liking. A strict standard "-std=c99" cannot be enforced due to the
   requirement of posix extensions, but something like
   @code CC=gcc -D_POSIX_C_SOURCE=200112L -std=c99 ... @endcode
   might work. Alternatively (and on some systems necessary), the dependence on 
   POSIX libraires can be completely turned off by
   @code CC=gcc -D_NO_POSIX_LIBS ... @endcode
   In this case user and host are not determined automatically, and these strings
   are instead set to "unknown".
 - Run make. The library that your application will need to link to should appear in
   build/libbdio.a
 - Your application needs to include bdio.h and be compiled with e.g.
   @code gcc yourapp.c -L <bdio_path>/build/ -I <bdio_path>/include -lbdio @endcode
 
 @section ex Examples
   A very simple application is <a href="www.bdio.org/examples/ex0.c">ex0.c</a>
   @include /home/korzec/bdio/examples/ex0/ex0.c
   Further simple programs using bdio can be found here
   - <a href="http://www.bdio.org/examples/ex1.c">ex1.c</a>
   - <a href="http://www.bdio.org/examples/ex2.c">ex2.c</a>
   - <a href="http://www.bdio.org/examples/ex3_1.c">ex3_1.c</a>
   - <a href="http://www.bdio.org/examples/ex3_2.c">ex3_2.c</a>
 */


#ifndef H_BDIO
#define H_BDIO 1

/* record data formats */
/** @def BDIO_BIN_GENERIC
 *  @brief record format for binary data not covered by other formats
 */
#define BDIO_BIN_GENERIC 0x00
/** @def BDIO_ASC_EXEC
 *  @brief record format for executable ASCII data like shell scripts
 */
#define BDIO_ASC_EXEC    0x01
/** @def BDIO_BIN_INT32BE
 *  @brief record format for 32bit integers with big endian byte ordering
 */
#define BDIO_BIN_INT32BE 0x02
/** @def BDIO_BIN_INT32LE
 *  @brief record format for 32bit integers with little endian byte ordering
 */
#define BDIO_BIN_INT32LE 0x03
/** @def BDIO_BIN_INT64BE
 *  @brief record format for 64bit (long) integers with big endian byte ordering
 */
#define BDIO_BIN_INT64BE 0x04
/** @def BDIO_BIN_INT64LE
 *  @brief record format for 64bit (long) integers with little endian byte ordering
 */
#define BDIO_BIN_INT64LE 0x05
/** @def BDIO_BIN_F32BE
 *  @brief record format for single precision floating point numbers  with big endian byte ordering
 */
#define BDIO_BIN_F32BE   0x06
/** @def BDIO_BIN_F32LE
 *  @brief record format for single precision floating point numbers  with little endian byte ordering
 */
#define BDIO_BIN_F32LE   0x07
/** @def BDIO_BIN_F64BE
 *  @brief record format for double precision floating point numbers  with big endian byte ordering
 */
#define BDIO_BIN_F64BE   0x08
/** @def BDIO_BIN_F64LE
 *  @brief record format for double precision floating point numbers  with little endian byte ordering
 */
#define BDIO_BIN_F64LE   0x09
/** @def BDIO_ASC_GENERIC
 *  @brief record format for generic ASCII data
 */
#define BDIO_ASC_GENERIC 0x0A
/** @def BDIO_ASC_XML
 *  @brief record format for XML data
 */
#define BDIO_ASC_XML     0x0B

/** @def BDIO_BIN_INT32
 *  @brief record format 32bit integers with unspecified byte order, which is determined automatically
 */
#define BDIO_BIN_INT32   0xF0
/** @def BDIO_BIN_INT64
 *  @brief record format 64bit (long) integers with unspecified byte order, which is determined automatically
 */
#define BDIO_BIN_INT64   0xF1
/** @def BDIO_BIN_F32
 *  @brief record format single precision floating point numbers with unspecified byte order, which is determined automatically
 */
#define BDIO_BIN_F32     0xF2
/** @def BDIO_BIN_F64
 *  @brief record format double precision floating point numbers with unspecified byte order, which is determined automatically
 */
#define BDIO_BIN_F64     0xF3

/* I/O modes 'r','w','a' */
/** @def BDIO_R_MODE
 *  @brief I/O mode: read
 */
#define BDIO_R_MODE 0
/** @def BDIO_W_MODE
 *  @brief I/O mode: write
 */
#define BDIO_W_MODE 1
/** @def BDIO_A_MODE
 *  @brief I/O mode: append
 */
#define BDIO_A_MODE 2

/* possible values for BDIO.state */
/** @def BDIO_H_STATE
 *  @brief BDIO.state: inside a header
 */
#define BDIO_H_STATE 1
/** @def BDIO_R_STATE
 *  @brief BDIO.state: inside a data record
 */
#define BDIO_R_STATE 2
/** @def BDIO_N_STATE
 *  @brief BDIO.state: neither in a header nor in a data record
 */
#define BDIO_N_STATE 3
/** @def BDIO_E_STATE
 *  @brief BDIO.state: in an error state
 */
#define BDIO_E_STATE 4

/* little endian, big endian */
/** @def BDIO_LEND
 *  @brief little endian (least significant byte first ordering, e.g. intel)
 */
#define BDIO_LEND 0
/** @def BDIO_BEND
 *  @brief big endian (most significant byte first ordering, e.g. bluegene)
 */
#define BDIO_BEND 1

/* auto hash records */
/** @def BDIO_NO_HASH
 *  @brief No calculation of checksums
 */
#define BDIO_NO_HASH   0
/** @def BDIO_AUTO_HASH
 *  @brief Automatic calculation of checksums
 */
#define BDIO_AUTO_HASH 1



/* hash modes */
/** @def BDIO_HASH_SINGL
 *  @brief single mode
 */
#define BDIO_HASH_SINGL 0
/** @def BDIO_HASH_CHAIN
 *  @brief chain mode
 */
#define BDIO_HASH_CHAIN 1

/** @def BDIO_HASH_MAGIC_S
 *  @brief magic number for hash records in single mode
 */
#define BDIO_HASH_MAGIC_S 1515784845
/** @def BDIO_HASH_MAGIC_C
 *  @brief magic number for hash records in chain mode
 */
#define BDIO_HASH_MAGIC_C 1515784846



#include <stdint.h>
#include <stdio.h>
/* for MD5 checksums */
#include <md5.h>

/* data types */
/** @struct BDIO bdio.h
 *  @brief bdio file descriptor
 *  @details Contains the state of the BDIO file and all data from the last
 *           header.
 */
typedef struct
{
   int state;     /**< state of an open bdio file:
                       BDIO_H_STATE,
                       BDIO_R_STATE,
                       BDIO_N_STATE or
                       BDIO_E_STATE */
   int mode;      /**< mode in which the bdio file was opened:
                       BDIO_R_MODE,
                       BDIO_W_MODE or
                       BDIO_A_MODE */
   int endian;    /**< native byte ordering of the machine
                       BDIO_LEND or BDIO_BEND */
   FILE *fp;      /**< file pointer to the bdio file */
   FILE *msg;     /**< file pointer to file for error messages (e.g. stderr) */
   int verbose;      /**< 0: silent mode, 1: print all errors, default: 0 */
   char error[256];  /**< Error message of last error that occured */
   char ferror[256]; /**< Error message of first error that occured */
   int nerror;       /**< number of errors that occured so far */

   unsigned char *buf; /**< buffer containing the last header/record
                            to be written */

   int hcnt;       /**< number of headers encountered (including current) */

   /* information contained in last header */
   int hmagic ;    /**< magic number of the last header */
   int hversion;   /**< bdio version of the last header */
   int hdirinfo1;  /**< minimal number of non-header records to follow the last */
                   /**< encountered header, before */
                   /**<  EOF or next header */
   int hdirinfo2;  /**< minimal number of non-header bytes to follow the last */
                   /**< encountered header, before */
                   /**< EOF or next header */
   int hcdate;     /**< date of first creation of the last header */
   int hmdate;     /**< date of last modification of the last header */
   char *hcuser;   /**< user of first creation of the last header */
   char *hmuser;   /**< user of last modification of the last header */
   char *hchost;   /**< host of first creation of the last header */
   char *hmhost;   /**< host of last modification of the last header */
   char *hpinfo;   /**< protocol-info | schema | generator | spec of the last header */
   int hssize;     /**< combined length of user/host/pinfo strings of the last header */
   uint64_t hstart;/**< starting position of last header record */

   /* information about current record */
   uint64_t rstart;/**< start position of current record or header */
   int rcnt;       /**< number of records encountered (including current) */
   uint64_t rlen;  /**< total length of current record or header */
                   /**< including the record-head */
   char rlongrec;  /**< 1 if record is a "long record", 0 else */
                     
   uint64_t ridx; /**< offset within current record or header */
   int rfmt;      /**< format of current record */
   int ruinfo;    /**< user info of current record */
   int rdsize;    /**< size of a data-item in the current record e.g int32 -> 4 */
   char rswap;    /**< 1/0 = records data has/has not to be byte-swapped */

   /* information about the buffer */
   uint64_t bufstart; /**< offset in record where the buffer starts */
   int bufidx;        /**< position in buffer; a byte at position bufidx */
                      /**< in the buffer is written to position */
                      /**< rstart+bufstart+bufidx in the file */
                      
   /* hash information */
   int hash_auto;     /**< If BDIO_AUTO_HASH, MD5 hash are comoputed
                           on-the-fly when writting a record. 
                           Default: BDIO_NO_HASH */
   int hash_mode;     /**< If mode is BDIO_HASH_CHAIN the hashe of 
                           a record is initialized with the hash of the 
                           previous record. 
                           Default: BDIO_HASH_SINGL */
   unsigned char prev_digest[16]; /**< If hash mode is BDIO_HASH_CHAIN, this 
                                       stores the previous record's hash.*/
   MD5_CTX *hash;    /**< The current hash ??? */
} BDIO;



/* prototypes */
/** @fn int bdio_is_hash_record(unsigned char digest[16], BDIO *fh)
    @brief Checks, whether a record is a MD5 hash record
    @detail Must be called before reading anything from the record
    @param[out] digest the checksum stored in this MD5 hash record
    @param[in] fh pointer to a BDIO file descriptor structure
    @return true if the current record is a MD5 record, false otherwise
    @author Alberto Ramos, Tomasz Korzec
 */
int bdio_is_hash_record(unsigned char digest[16], BDIO *fh);


/** @fn void bdio_hash_auto(BDIO *fh)
    @brief Enables the automatic computation of MD5 checksums of records.
    @param[in] fh pointer to a BDIO file descriptor structure
    @author Alberto Ramos
 */
void bdio_hash_auto(BDIO *fh);

/** @fn void bdio_hash_chain(BDIO *fh);
    @brief Enables the chain mode for checksum calculation
    @detail When turned on, hashes of records are initialized with previous
            record's hash.
    @param[in] fh pointer to a BDIO file descriptor structure
    @author Alberto Ramos
 */
void bdio_hash_chain(BDIO *fh);

/** @fn void bdio_perror(const char *s, BDIO *fh)
    @brief Print an error string to BDIO.msg
    @details Print the string pointed to by s followed by the description of the
    last error that occured to BDIO.msg. <br>
    If s is NULL, only the description of the last error is printed. <p>
    The function fails if fh is invalid or if the error-stream associated with fh
    is invalid.
    @param[in] s 0-terminated string with user-provided error message
    @param[in] fh pointer to a BDIO file descriptor structure
 */
void bdio_perror(const char *s, BDIO *fh);


/** @fn void bdio_pferror(const char *s, BDIO *fh)
    @brief Print an error string to BDIO.msg 
    @details Print the string pointed to by s followed by the description of the
    <b>first</b> error that occured to BDIO.msg. <br>
    If s is NULL, only the description of the first error is printed. <p>
    The function fails if fh is invalid or if the error-stream associated with fh
    is invalid.
    @param[in] s 0-terminated string with user-provided error message
    @param[in] fh pointer to a BDIO file descriptor structure
 */
void bdio_pferror(const char *s, BDIO *fh);

/** @fn BDIO *bdio_open(const char* file, const char* mode, char* protocol_info)
    @brief Open a bdio file in mode 'r' (read), 'w' (write) or 'a' (append).
    @details In read mode, the header record is read and checked. The stream
    remains positioned in the header record, therefore bdio_seek_record
    must be called to enter the next record.
    protocol_info may be NULL, but if it is not NULL, it must match the
    protocol_info string of the first header.<p>
  
    In write mode, the header record is written. The stream remains
    positioned in the header record, therefore bdio_start_record must
    be called to start a new record.
    protocol_info must be a 0-terminated string (at least "").<p>
  
    In append mode, the header records are read and checked. The stream
    is positioned at the end of the last header or after the last record,
    therefore bdio_start_record or bdio_append_record
    (only possible if the last item was a record)  must be called to
    start a new record.
    If the file is not empty, protocol_info may be NULL - if not NULL it must
    match the one of the last header.
    If the file is empty, protocol_info must be a 0-terminated string.<p>
   
   The maximal length protocol_info may have is: XXXX TODO <-- <p>
   
   bdio_open fails if
    - there is not enough memory for the buffers,
    - an invalid mode was specified,
    - the machine has an unknown endianness,
    - there was an I/O error during reading/writing of the header,
    - the header is not a valid bdio header,
    - protocol_info fails to fulfill the requirements,
    - fopen, fclose, fwrite, fread, ftell fail for some reason.
   @return Upon successful completion bdio_open returns a pointer to a bdio file
    structure.  Otherwise, NULL is returned
   @param[in] file 0-terminated string specifying the file name
   @param[in] mode a bdio file mode, can be BDIO_R_MODE, BDIO_W_MODE or BDIO_A_MODE
   @param[in] protocol_info 0-terminated string specifying the protocol-info
 */
BDIO *bdio_open(const char* file, const char* mode, char* protocol_info);


/** @fn int bdio_close(BDIO *fh)
    @brief Close a bdio file.
    @details If the file has been opened in write or append mode
    this function must be called also to guarantee that the last written
    record is correctly flushed (update of the record length).<p>
    bdio_close fails if
    - fclose fails,
    - fh is a NULL pointer
    - fh is in state BDIO_E_STATE.
    .
    In the last case bdio_close will
    close the file without flushing.
    @return Upon successful completion 0 is returned. Otherwise, EOF is returned
    In either case any further access
    (including another call to bdio_close) to fh results in undefined behavior.
    @param[in] fh pointer to a BDIO file descriptor structure
 */
int bdio_close(BDIO *fh);


/** @fn void bdio_set_user(char *u)
    @brief Set the default user name (overrides automatic determination)
    @param[in] u pointer to a 0-terminated string
 */
void bdio_set_user(char* u);

/** @fn void bdio_set_host(char *u)
    @brief Set the default host name (overrides automatic determination)
    @param[in] u pointer to a 0-terminated string
 */
void bdio_set_host(char* u);

/** @fn void bdio_set_dflt_msg(FILE *stream)
    @brief Set the default stream for error messages
    @param[in] stream pointer to a file into which error messages should be 
               printed by default for any newly opened bdio file.
 */
void bdio_set_dflt_msg(FILE *stream);


/** @fn int bdio_set_msg(FILE *stream,BDIO *fh)
    @brief Set the stream for error messages in fh to stream
    @details Fails if fh is invalid or stream is NULL
    @return Upon successfull completion 0 is returned. Otherwise EOF is
    returned.
    @param[in] stream pointer to a file into which error messages associated with
               bdio file fh are printed
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_set_msg(FILE *stream,BDIO *fh);


/** @fn void bdio_set_dflt_verbose(int v)
    @brief Change the behavior for reporting of errors
    @details Set the default for newly opened bdio files to
    verbose (v !=0 ) or silent (v==0) <p>
    Fails if v is negative
    @param[in] v new default behavior
 */
void bdio_set_dflt_verbose(int v);


/** @fn int bdio_set_verbose(int v,BDIO *fh)
   @brief Set bdio file fh to verbose (v>0) or silent (v==0)
   @details Fails if fh is invalid or in error state or v is negative
   @return Upon successfull completion 0 is returned. Otherwise EOF is
    returned
   @param[in] v v==0 means silent mode and v>0 means verbose mode
   @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_set_verbose(int v,BDIO *fh);


/** @fn char* bdio_get_hchost(BDIO *fh)
    @brief Get the host on which the last header was created
    @details Fails if fh is invalid or in error state
    @return 0-terminated string with a host name
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
char* bdio_get_hchost(BDIO *fh);


/** @fn char* bdio_get_hcuser(BDIO *fh)
    @brief Get the user name who created the last header
    @details Fails if fh is invalid or in error state
    @return 0-terminated string with a user name
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
char* bdio_get_hcuser(BDIO *fh);


/** @fn char* bdio_get_hmhost(BDIO *fh)
    @brief Get the host on which the last header was most recently modified
    @details Fails if fh is invalid or in error state
    @return 0-terminated string with a host name
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
char* bdio_get_hmhost(BDIO *fh);


/** @fn char* bdio_get_hmuser(BDIO *fh)
    @brief Get the user name who modified the last header most recently
    @details Fails if fh is invalid or in error state
    @return 0-terminated string with a user name
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
char* bdio_get_hmuser(BDIO *fh);


/** @fn char* bdio_get_hpinfo(BDIO *fh)
    @brief Get the protocol info of the last header
    @details Fails if fh is invalid or in error state
    @return 0-terminated string with a protocol info
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
char* bdio_get_hpinfo(BDIO *fh);


/** @fn int bdio_get_hcdate(BDIO *fh)
    @brief Get the time of creation of the last header
    @details Fails if fh is invalid or in error state
    @return unix time (seconds since Jan 01 1970)
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_hcdate(BDIO *fh);


/** @fn int bdio_get_hmdate(BDIO *fh)
    @brief Get the time of the most recent modification of the last header
    @details Fails if fh is invalid or in error state
    @return unix time (seconds since Jan 01 1970)
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_hmdate(BDIO *fh);

/** @fn int bdio_get_ruinfo(BDIO *fh)
    @brief Get the user info of the current record
    @details Fails if fh is invalid or in error state
    @return user info, integer between 0 and 15
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_ruinfo(BDIO *fh);


/** @fn int bdio_get_hversion(BDIO *fh)
    @brief Get version number in last header
    @details Fails if fh is invalid or in error state
    @return Upon success the integer version number is returned. Upon failure EOF is returned
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_hversion(BDIO *fh);


/** @fn int bdio_get_hcnt(BDIO *fh)
    @brief Get number of headers read or written so far (including current)
    @details Fails if fh is a null pointer
    or if fh is in state BDIO_E_STATE
    @return Upon success the number of headers is returned. Upon failure EOF is returned.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_hcnt(BDIO *fh);


/** @fn int bdio_get_rfmt(BDIO *fh)
    @brief Get format of current record (written or read).
    @details Fails if fh is a null pointer
    or if fh is not in state BDIO_R_STATE
    @return If fh is in state BDIO_R_STATE the format of the record is returned.<br>
            If fh is not in a record or if an error occurs, EOF is returned.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_rfmt(BDIO *fh);


/** @fn uint64_t bdio_get_rlen(BDIO *fh)
    @brief Get length of data content of current record (written or read).
    @details Fails if fh if is a null pointer
    or if fh is in state BDIO_E_STATE
    @return Upon success the length is returned. Upon failure EOF is
    returned.<br>
    If fh is currently not in a record, 0 is returned
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
uint64_t bdio_get_rlen(BDIO *fh);



/** @fn int bdio_get_rcnt(BDIO *fh)
    @brief Get number of records read or written so far (including current)
    @details Fails if fh is a null pointer
             or if fh is in state BDIO_E_STATE
    @return Upon success the number of records is returned. Upon failure EOF is
    returned.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_get_rcnt(BDIO *fh);


/** @fn int bdio_is_in_record(BDIO *fh)
    @brief Returns 1 if fh is in a record and 0 otherwise
    @details Fails if fh is invalid
    @return 1 if fh->state is BDIO_R_STATE and 0 otherwise
    @param[in] fh pointer to a BDIO file descriptor structure
 */
int bdio_is_in_record(BDIO *fh);


/** @fn int bdio_is_in_header(BDIO *fh)
    @brief Returns 1 if fh is in a header and 0 otherwise
    @details Fails if fh is invalid
    @return 1 if fh->state is BDIO_H_STATE and 0 otherwise
    @param[in] fh pointer to a BDIO file descriptor structure
 */
int bdio_is_in_header(BDIO *fh);


/** @fn int bdio_seek_record(BDIO *fh)
    @brief Position bdio stream to start of next record and read its header.
    @details fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - fh is not in read mode
    - fseek or fread fail
    @return Upon success 0 is returned, otherwise EOF is returned. Upon end of
            file, EOF is returned.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
int bdio_seek_record(BDIO *fh);


/** @fn size_t bdio_read(void *buf, size_t nb, BDIO *fh)
    @brief Read nb bytes from fh into buf.
    @details Independent of the endiannes of the machine and the record type, exactly
    the bytes from the file are copied. If a byte-swap is necessary, the user
    is responsible for doing it.<p>
    Fails if
    -fh is a null pointer
    -fh is in state BDIO_E_STATE
    -fh is not in read mode
    -nb is too large
    -fread fails
    @return Returns the number of bytes read. <br>
    If an error occurs, or the end of the current record
    or the file is reached, the return
    value is a short item count (or zero).
    @param[in] buf pointer to a location where the read data is copied to.
    @param[in] nb number of bytes to be read.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
size_t bdio_read(void *buf, size_t nb, BDIO *fh);


/** @fn size_t bdio_read_f32(float *buf, size_t nb, BDIO *fh)
    @brief brief Read nb bytes from fh into buf. nb must be a multiple of 4.
    @details If the endianness of the machine differs from the one of the current record,
    the byte order of the read data is swapped.
    If this swapping is not desired the general bdio_read should be used.<p>
    Fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_fread fails
    @return Returns the number of bytes read.<br>
    If an error occurs, or the end of the current record
    or the file is reached, the return
    value is a short item count (or zero).
    @param[in] buf float-pointer to a location where the read data is copied to.
    @param[in] nb number of bytes to be read (multiple of 4).
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
size_t bdio_read_f32(float *buf, size_t nb, BDIO *fh);


/** @fn size_t bdio_read_f64(double *buf, size_t nb, BDIO *fh)
    @brief brief Read nb bytes from fh into buf. nb must be a multiple of 8.
    @details If the endianness of the machine differs from the one of the current record,
    the byte order of the read data is swapped.
    If this swapping is not desired the general bdio_read should be used.<p>
    Fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_fread fails
    @return Returns the number of bytes read.<br>
    If an error occurs, or the end of the current record
    or the file is reached, the return
    value is a short item count (or zero).
    @param[in] buf double-pointer to a location where the read data is copied to.
    @param[in] nb number of bytes to be read (multiple of 8).
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
size_t bdio_read_f64(double *buf, size_t nb, BDIO *fh);



/** @fn size_t bdio_read_int32(int32_t *buf, size_t nb, BDIO *fh)
    @brief brief Read nb bytes from fh into buf. nb must be a multiple of 4.
    @details If the endianness of the machine differs from the one of the current record,
    the byte order of the read data is swapped.
    If this swapping is not desired the general bdio_read should be used.<p>
    Fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_fread fails
    @return Returns the number of bytes read.<br>
    If an error occurs, or the end of the current record
    or the file is reached, the return
    value is a short item count (or zero).
    @param[in] buf int32_t-pointer to a location where the read data is copied to.
    @param[in] nb number of bytes to be read (multiple of 4).
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
size_t bdio_read_int32(int32_t *buf, size_t nb, BDIO *fh);


/** @fn size_t bdio_read_int64(int64_t *buf, size_t nb, BDIO *fh)
    @brief brief Read nb bytes from fh into buf. nb must be a multiple of 8.
    @details If the endianness of the machine differs from the one of the current record,
    the byte order of the read data is swapped.
    If this swapping is not desired the general bdio_read should be used.<p>
    Fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_fread fails
    @return Returns the number of bytes read.<br>
    If an error occurs, or the end of the current record
    or the file is reached, the return
    value is a short item count (or zero).
    @param[in] buf int64_t-pointer to a location where the read data is copied to.
    @param[in] nb number of bytes to be read (multiple of 8).
    @param[in] fh pointer to a BDIO file descriptor structure.
 */
size_t bdio_read_int64(int64_t *buf, size_t nb, BDIO *fh);


/** @fn int bdio_start_record(int fmt, int uinfo, BDIO *fh)
    @brief Position bdio stream after the current record and start writing a new record with specified format and uinfo.
    @details Fails if
    - fh is a null pointer
    - fh is in state BDIO_E_STATE
    - fh is not in write/append mode
    - fmt has an illegal value
    - info has an illegal value
    - the endianness of the system can not be determined
    - bdio_flush_record fails
    @return Upon success 0 is returned, otherwise EOF is returned.
    @param[in] fmt supported formats are
    format                 | content 
    -----------------------|----------------------------------------------------
    BDIO_ASC_EXEC          |  for ASCII data containing shell scripts
    BDIO_ASC_XML           |  for ASCII data containing XML
    BDIO_ASC_GENERIC       |  for generic ASCII data
    BDIO_BIN_GENERIC       |  for generic binary data
    BDIO_BIN_INT32BE       |  for big endian 32 bit integers
    BDIO_BIN_INT32LE       |  for little endian 32 bit integers
    BDIO_BIN_INT64BE       |  for big endian 64 bit integers
    BDIO_BIN_INT64LE       |  for little endian 64 bit integers
    BDIO_BIN_F32BE         |  for big endian single precision floats
    BDIO_BIN_F32LE         |  for little endian single precision floats
    BDIO_BIN_F64BE         |  for big endian double precision floats
    BDIO_BIN_F64LE         |  for little endian double precision floats
    BDIO_BIN_INT32         |  for 32 bit integers stored in machine endianness
    BDIO_BIN_INT64         |  for 64 bit integers stored in machine endianness
    BDIO_BIN_F32           |  for single precision floats in machine endianness
    BDIO_BIN_F64           |  for double precision floats in machine endianness
  
    @param[in] uinfo is a number between 0 and 15 specified by the user.
    @param[in] fh pointer to a BDIO file descriptor structure.
 */ 
int bdio_start_record(int fmt, int uinfo, BDIO *fh);


/** @fn int bdio_append_record(int fmt, int uinfo, BDIO *fh)
    @brief If the last item in the file was a record: position the stream at the end of it. Otherwise  EOF is returned.
    @details The specified format and uinfo must match the last record's values.<br>
     fails if
     - fh is a null pointer
     - fh is in state BDIO_E_STATE
     - fh is not in write/append mode
     - fmt does not match last record's value
     - info does not match last record's value
     - fh is not in BDIO_N_STATE
    @return Upon success 0 is returned, otherwise EOF is returned.
    @param[in] fmt Supported formats are
    format              | content 
    --------------------|----------------------------------------------------
    BDIO_ASC_EXEC       | for ASCII data containing shell scripts
    BDIO_ASC_XML        | for ASCII data containing XML
    BDIO_ASC_GENERIC    | for generic ASCII data
    BDIO_BIN_GENERIC    | for generic binary data
    BDIO_BIN_INT32BE    | for big endian 32 bit integers
    BDIO_BIN_INT32LE    | for little endian 32 bit integers
    BDIO_BIN_INT64BE    | for big endian 64 bit integers
    BDIO_BIN_INT64LE    | for little endian 64 bit integers
    BDIO_BIN_F32BE      | for big endian single precision floats
    BDIO_BIN_F32LE      | for little endian single precision floats
    BDIO_BIN_F64BE      | for big endian double precision floats
    BDIO_BIN_F64LE      | for little endian double precision floats
    BDIO_BIN_INT32      | for 32 bit integers stored in last record's endianness
    BDIO_BIN_INT64      | for 64 bit integers stored in last record's endianness
    BDIO_BIN_F32        | for single precision floats in last record's endianness
    BDIO_BIN_F64        | for double precision floats in last record's endianness
   @param[in] uinfo is a number between 0 and 15 specified by the user.
   @param[in] fh pointer to a BDIO file descriptor structure.
  */
int bdio_append_record(int fmt, int uinfo, BDIO *fh);



/** @fn size_t bdio_write(void *ptr, size_t nb, BDIO *fh)
    @brief Write nb bytes from ptr to fh.
    @details nb must be a multiple of the record's data-type-size (e.g. multiple of
    8 for BDIO_BIN_F64)
    Independent of the endianness of the current record, EXACTLY the data
    pointed to by ptr is written into the file. The user is responsible for
    the correct byte order. If automatic correct byte ordering is desired,
    the functions bdio_write_f32, bdio_write_f64, bdio_write_i32 and
    bdio_write_i64 should be used.<p>
    Fails if
    - fh if is a null pointer
    - fh is in state BDIO_E_STATE
    - fh is not in write/append mode
    - fwrite fails
    @return Returns number of bytes written.
            If an error occurs the return
            value is a short item count (or zero).
    @param[in] ptr pointer to data which is to be written.
    @param[in] nb number of bytes to be written. Must be a multiple of the 
               size of the record's data-type. E.g. multiple of 4 for 
               BDIO_BIN_F32 records.
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
size_t bdio_write(void *ptr, size_t nb, BDIO *fh);



/** @fn size_t bdio_write_f32(float *ptr, size_t nb, BDIO *fh)
    @brief Write nb bytes from ptr to fh.
    @details nb must be a multiple of 4.
    If the endiannes of the machine differs from the one of the current record,
    the byte order of the data in ptr is swapped before writing. If this
    automatic swapping is not desired, the general bdio_write should be used.<p>
    Fails if
    - fh if is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_write fails
    @return Returns number of bytes written.
            If an error occurs the return
            value is a short item count (or zero).
    @param[in] ptr pointer to float data which is to be written.
    @param[in] nb number of bytes to be written. Must be a multiple of 4. 
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
size_t bdio_write_f32(float *ptr, size_t nb, BDIO *fh);



/** @fn size_t bdio_write_f64(double *ptr, size_t nb, BDIO *fh)
    @brief Write nb bytes from ptr to fh.
    @details nb must be a multiple of 8.
    If the endiannes of the machine differs from the one of the current record,
    the byte order of the data in ptr is swapped before writing. If this
    automatic swapping is not desired, the general bdio_write should be used.<p>
    Fails if
    - fh if is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_write fails
    @return Returns number of bytes written.
            If an error occurs the return
            value is a short item count (or zero).
    @param[in] ptr pointer to double data which is to be written.
    @param[in] nb number of bytes to be written. Must be a multiple of 8. 
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
size_t bdio_write_f64(double *ptr, size_t nb, BDIO *fh);



/** @fn size_t bdio_write_int32(int32_t *ptr, size_t nb, BDIO *fh)
    @brief Write nb bytes from ptr to fh.
    @details nb must be a multiple of 4.
    If the endiannes of the machine differs from the one of the current record,
    the byte order of the data in ptr is swapped before writing. If this
    automatic swapping is not desired, the general bdio_write should be used.<p>
    Fails if
    - fh if is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_write fails
    @return Returns number of bytes written.
            If an error occurs the return
            value is a short item count (or zero).
    @param[in] ptr pointer to int32_t data which is to be written.
    @param[in] nb number of bytes to be written. Must be a multiple of 4. 
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
size_t bdio_write_int32(int32_t *ptr, size_t nb, BDIO *fh);



/** @fn size_t bdio_write_int64(int64_t *ptr, size_t nb, BDIO *fh)
    @brief Write nb bytes from ptr to fh.
    @details nb must be a multiple of 8.
    If the endiannes of the machine differs from the one of the current record,
    the byte order of the data in ptr is swapped before writing. If this
    automatic swapping is not desired, the general bdio_write should be used.<p>
    Fails if
    - fh if is a null pointer
    - fh is in state BDIO_E_STATE
    - bdio_write fails
    @return Returns number of bytes written.
            If an error occurs the return
            value is a short item count (or zero).
    @param[in] ptr pointer to int64_t data which is to be written.
    @param[in] nb number of bytes to be written. Must be a multiple of 8. 
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
size_t bdio_write_int64(int64_t *ptr, size_t nb, BDIO *fh);




/** @fn int bdio_flush_record( BDIO *fh)
    @brief Finalize the current record and set fh to BDIO_N_STATE.
    @details If fh is open in write/append mode:<br>
      If state is BDIO_R_STATE, the current record is finalized
      by flushing, writing the length into the record-header, moving the
      file pointer to the end of the file and changing the state to
      BDIO_N_STATE<p>
      If state is BDIO_H_STATE, the header record is finalized by flushing
      and setting the state to BDIO_N_STATE <p>
      If state is BDIO_N_STATE, nothing is done <p>
      A call to this function is usually <b>not</b> necessary. A call is
      advised only before engaging in operations that may lead to a crash or 
      early termination, But even then, calling bdio_close would be preferred.<p>
      Fails if
     - fh is a null pointer
     - fh is in state BDIO_E_STATE
     - fh is not in write/append mode
     - fflush, ftell, fwrite or fseek fail
    @return Upon successful completion 0 is returned.
       Otherwise, EOF is returned.
    @param[in] fh pointer to a BDIO file descriptor structure.
  */
int bdio_flush_record( BDIO *fh);

#endif
