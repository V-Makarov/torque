/*
*         OpenPBS (Portable Batch System) v2.3 Software License
*
* Copyright (c) 1999-2000 Veridian Information Solutions, Inc.
* All rights reserved.
*
* ---------------------------------------------------------------------------
* For a license to use or redistribute the OpenPBS software under conditions
* other than those described below, or to purchase support for this software,
* please contact Veridian Systems, PBS Products Department ("Licensor") at:
*
*    www.OpenPBS.org  +1 650 967-4675                  sales@OpenPBS.org
*                        877 902-4PBS (US toll-free)
* ---------------------------------------------------------------------------
*
* This license covers use of the OpenPBS v2.3 software (the "Software") at
* your site or location, and, for certain users, redistribution of the
* Software to other sites and locations.  Use and redistribution of
* OpenPBS v2.3 in source and binary forms, with or without modification,
* are permitted provided that all of the following conditions are met.
* After December 31, 2001, only conditions 3-6 must be met:
*
* 1. Commercial and/or non-commercial use of the Software is permitted
*    provided a current software registration is on file at www.OpenPBS.org.
*    If use of this software contributes to a publication, product, or
*    service, proper attribution must be given; see www.OpenPBS.org/credit.html
*
* 2. Redistribution in any form is only permitted for non-commercial,
*    non-profit purposes.  There can be no charge for the Software or any
*    software incorporating the Software.  Further, there can be no
*    expectation of revenue generated as a consequence of redistributing
*    the Software.
*
* 3. Any Redistribution of source code must retain the above copyright notice
*    and the acknowledgment contained in paragraph 6, this list of conditions
*    and the disclaimer contained in paragraph 7.
*
* 4. Any Redistribution in binary form must reproduce the above copyright
*    notice and the acknowledgment contained in paragraph 6, this list of
*    conditions and the disclaimer contained in paragraph 7 in the
*    documentation and/or other materials provided with the distribution.
*
* 5. Redistributions in any form must be accompanied by information on how to
*    obtain complete source code for the OpenPBS software and any
*    modifications and/or additions to the OpenPBS software.  The source code
*    must either be included in the distribution or be available for no more
*    than the cost of distribution plus a nominal fee, and all modifications
*    and additions to the Software must be freely redistributable by any party
*    (including Licensor) without restriction.
*
* 6. All advertising materials mentioning features or use of the Software must
*    display the following acknowledgment:
*
*     "This product includes software developed by NASA Ames Research Center,
*     Lawrence Livermore National Laboratory, and Veridian Information
*     Solutions, Inc.
*     Visit www.OpenPBS.org for OpenPBS software support,
*     products, and information."
*
* 7. DISCLAIMER OF WARRANTY
*
* THIS SOFTWARE IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND. ANY EXPRESS
* OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT
* ARE EXPRESSLY DISCLAIMED.
*
* IN NO EVENT SHALL VERIDIAN CORPORATION, ITS AFFILIATED COMPANIES, OR THE
* U.S. GOVERNMENT OR ANY OF ITS AGENCIES BE LIABLE FOR ANY DIRECT OR INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* This license will be governed by the laws of the Commonwealth of Virginia,
* without reference to its choice of law rules.
*/



#include <pbs_config.h>   /* the master config generated by configure */
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include "lib_ifl.h" /* DIS_tcp_setup, DIS_tcp_cleanup */


#if defined(FD_SET_IN_SYS_SELECT_H)
#  include <sys/select.h>
#endif

#include "dis.h"
#include "dis_internal.h"
#include "dis_init.h"
#include "log.h"
#include "../Libutils/u_lock_ctl.h"
#include "../Libnet/lib_net.h" /* socket_* */

#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif

#define MAX_SOCKETS 65536
time_t pbs_tcp_timeout = 300; // seconds



void DIS_tcp_settimeout(

  long timeout)  /* I */

  {
  pbs_tcp_timeout = timeout;

  return;
  }  /* END DIS_tcp_settimeout() */



/*
 * tcp_pack_buff - pack existing data into front of buffer
 *
 * Moves "uncommited" data to front of buffer and adjusts pointers.
 * Does a character by character move since data may over lap.
 */

static void tcp_pack_buff(

  struct tcpdisbuf *tp)

  {
  size_t amt;
  size_t start;
  size_t i;

  start = tp->tdis_trailp - tp->tdis_thebuf;

  if (start != 0)
    {
    amt  = tp->tdis_eod - tp->tdis_trailp;

    for (i = 0;i < amt;++i)
      {
      *(tp->tdis_thebuf + i) = *(tp->tdis_thebuf + i + start);
      }
    *(tp->tdis_thebuf + amt) = '\0';

    tp->tdis_leadp  -= start;

    tp->tdis_trailp -= start;
    tp->tdis_eod    -= start;
    }

  return;
  }  /* END tcp_pack_buff() */




/*
 * tcp_read - read data from tcp stream to "fill" the buffer
 * Update the various buffer pointers.
 *
 * Return: >0 number of characters read
 *   0 if EOD (no data currently avalable)
 *  -1 if error
 *  -2 if EOF (stream closed)
 */

int tcp_read(

  struct tcp_chan *chan,
  long long       *read_len,
  long long       *avail_len,
  unsigned int     timeout)

  {
  int               rc = PBSE_NONE;
  unsigned long     newsize;
  char             *ptr;
  int               tdis_buf_len = 0;
  int               max_read_len = 0;
  char             *new_data = NULL;
  struct tcpdisbuf *tp;
  int               tmp_leadp = 0;
  int               tmp_trailp = 0;
  int               tmp_eod = 0;
  char              err_msg[1024];


  tp = &chan->readbuf;

  /* must compact any uncommitted data into bottom of buffer */
  tcp_pack_buff(tp);

  chan->IsTimeout = 0;
  chan->SelectErrno = 0;
  chan->ReadErrno = 0;
  tdis_buf_len = tp->tdis_bufsize;
  max_read_len = tp->tdis_bufsize - (tp->tdis_eod - tp->tdis_thebuf);

  /*
   * we don't want to be locked out by an attack on the port to
   * deny service, so we time out the read, the network had better
   * deliver promptly
   */

  if ((rc = socket_read(chan->sock, &new_data, read_len, timeout)) != PBSE_NONE)
    {
    switch (rc)
      {
      case PBSE_TIMEOUT:

        chan->IsTimeout = 1;

        break;

      default:

        chan->SelectErrno = rc;
        chan->ReadErrno = rc;

        break;
      }

    if (new_data != NULL)
      free(new_data);

    return(rc);
    }
  /* data read is less than buffer size */
  else if (max_read_len > *read_len)
    {
    memcpy(tp->tdis_eod, new_data, *read_len);
    tp->tdis_eod += *read_len;
    *tp->tdis_eod = '\0';
    *avail_len = tp->tdis_eod - tp->tdis_leadp;
    max_read_len = tp->tdis_eod - tp->tdis_thebuf;

    if (max_read_len > tdis_buf_len)
      {
      snprintf(err_msg, sizeof(err_msg),
        "eod ptr BEYOND end of buffer!! (fit) Remaining buffer = %d, read_len = %lld",
        max_read_len, *read_len);
      log_err(PBSE_INTERNAL,__func__,err_msg);
      }

    free(new_data);
    }
  /* data read is greater than buffer size */
  else if (max_read_len <= *read_len)
    {
    newsize = (tdis_buf_len + *read_len) * 2;
    if ((ptr = (char *)calloc(1, newsize+1)) == NULL)
      {
      log_err(ENOMEM,__func__,"Could not allocate memory to read buffer");
      rc = PBSE_MEM_MALLOC;
      free(new_data);
      return rc;
      }

    tmp_leadp = tp->tdis_leadp - tp->tdis_thebuf;
    tmp_trailp = tp->tdis_trailp - tp->tdis_thebuf;
    tmp_eod = tp->tdis_eod - tp->tdis_thebuf;

    snprintf(ptr, newsize, "%s%s", tp->tdis_thebuf, new_data);
    free(tp->tdis_thebuf);
    tp->tdis_thebuf = ptr;
    tp->tdis_bufsize = newsize;
    tp->tdis_eod = tp->tdis_thebuf + tmp_eod + *read_len;
    tp->tdis_trailp = tp->tdis_thebuf + tmp_trailp;
    tp->tdis_leadp = tp->tdis_thebuf + tmp_leadp;
    *avail_len = tp->tdis_eod - tp->tdis_leadp;

    max_read_len = tp->tdis_eod - tp->tdis_thebuf;
    tdis_buf_len = newsize;

    if (max_read_len > tdis_buf_len)
      {
      snprintf(err_msg, sizeof(err_msg), "eod ptr BEYOND end of buffer!!(expand) Remaining buffer = %d, read_len = %lld", max_read_len, *read_len);
      log_err(PBSE_INTERNAL,__func__,err_msg);
      }

    free(new_data);
    }

  return(rc);
  }  /* END tcp_read() */





/*
 * DIS_tcp_wflush - flush tcp/dis write buffer
 *
 * Writes "committed" data in buffer to file discriptor,
 * packs remaining data (if any), resets pointers
 * Returns: 0 on success, -1 on error
 *      NOTE:  does not close fd
 *
 */

int DIS_tcp_wflush(

  struct tcp_chan *chan)  /* I */

  {
  int               i;
  char             *pbs_debug = NULL;

  struct tcpdisbuf *tp = &chan->writebuf;
  char             *pb = tp->tdis_thebuf;
  size_t            ct = tp->tdis_trailp - tp->tdis_thebuf;

  pbs_debug = getenv("PBSDEBUG");

  while ((i = write_ac_socket(chan->sock, pb, ct)) != (ssize_t)ct)
    {
    if (i == -1)
      {
      if (errno == EINTR)
        {
        continue;
        }

      /* FAILURE */

      if (pbs_debug != NULL)
        {
        fprintf(stderr,
          "TCP write of %d bytes (%.32s) [sock=%d] failed, errno=%d (%s)\n",
          (int)ct, pb, chan->sock, errno, strerror(errno));
        }
      
      return(-1);
      }  /* END if (i == -1) */
    else
      {
      ct -= i;
      pb += i;
      }
    }  /* END while (i) */

  /* SUCCESS */

  tp->tdis_eod = tp->tdis_leadp;

  tcp_pack_buff(tp);

  return(0);
  }  /* END DIS_tcp_wflush() */





/*
 * DIS_tcp_clear - reset tpc/dis buffer to empty
 */

static void DIS_tcp_clear(

  struct tcpdisbuf *tp)

  {
  tp->tdis_leadp  = tp->tdis_thebuf;
  tp->tdis_trailp = tp->tdis_thebuf;
  tp->tdis_eod    = tp->tdis_thebuf;

  return;
  }





void DIS_tcp_reset(

  struct tcp_chan *chan,
  int i)

  {
  if (i == 0)
    DIS_tcp_clear(&chan->readbuf);
  else
    DIS_tcp_clear(&chan->writebuf);
  return;
  }  /* END DIS_tcp_reset() */





/*
 * tcp_rskip - tcp/dis support routine to skip over data in read buffer
 *
 * Returns: 0 on success, -1 on error
 */

int tcp_rskip(

  struct tcp_chan *chan,
  size_t ct)

  {
  struct tcpdisbuf *tp;
  tp = &chan->readbuf;
  if (tp->tdis_leadp - tp->tdis_eod < (ssize_t)ct)
    {
    /* this isn't the best thing to do, but this isn't used, so */
    return(-1);
    }
  tp->tdis_leadp += ct;
  return(0);
  }



/*
 * tcp_gets - tcp/dis support routine to get a string from read buffer
 *
 * Return: number of characters read (>=1)
 *  -1 if error
 *  -2 if EOF/EOD (stream closed)
 */

int tcp_gets(

  struct tcp_chan *chan,
  char            *str,
  size_t           ct,
  unsigned int     timeout)

  {
  int               rc = 0;
  struct tcpdisbuf *tp;
  long long         data_read = 0;
  long long         data_avail = 0;

  tp = &chan->readbuf;
  /* length of usable data in current buffer */
  data_avail = tp->tdis_eod - tp->tdis_leadp;

  while ((size_t)data_avail < ct)
    {
    /* not enough data, try to get more */
    if ((rc = tcp_read(chan, &data_read, &data_avail, timeout)) != PBSE_NONE)
      {
      if (data_read == 0)
        rc = -2;
      else
        rc = -1;
      return(rc);  /* Error or EOF */
      }
    }
  memcpy((char *)str, tp->tdis_leadp, ct);
  tp->tdis_leadp += ct;
  return((int)ct);
  }  /* END tcp_gets() */


/*
 * tcp_getc - see tcp_gets
 */

int tcp_getc(

  struct tcp_chan *chan,
  unsigned int     timeout)

  {
  int rc = DIS_SUCCESS;
  char ret_val;
  if ((rc = tcp_gets(chan, &ret_val, 1, timeout)) < 0)
    return rc;
  return (int)ret_val;
  }  /* END tcp_getc() */



/*
 * tcp_puts - tcp/dis support routine to put a counted string of characters
 * into the write buffer.
 *
 * Returns: >= 0, the number of characters placed
 *   -1 if error
 */

int tcp_puts(

  struct tcp_chan *chan,  /* I */
  const char *str, /* I */
  size_t      ct)  /* I */

  {
  struct tcpdisbuf *tp = NULL;
  char             *temp = NULL;
  int               leadpct;
  int               trailpct; 
  size_t            newbufsize;
  char              log_buf[LOCAL_LOG_BUF_SIZE];

  /* NOTE:  currently, failures may occur if THE_BUF_SIZE is not large enough */
  /*        this should be changed to allow proper operation with degraded    */
  /*        performance (how?) */

  tp = &chan->writebuf;

  if (tp->tdis_bufsize == 0)
   {
   snprintf(log_buf,sizeof(log_buf),
     "write buffer's tdis_bufsize was unexpectely found with a value of 0");
   log_err(-1, __func__, log_buf);
   return(-1);
   }

  if ((tp->tdis_thebuf + tp->tdis_bufsize - tp->tdis_leadp) < (ssize_t)ct)
    {
    /* not enough room, reallocate the buffer */
    leadpct = (int)(tp->tdis_thebuf - tp->tdis_leadp);
    trailpct = (int)(tp->tdis_thebuf - tp->tdis_trailp);
    newbufsize = tp->tdis_bufsize + THE_BUF_SIZE + ct*2;
    temp = (char *)calloc(1, newbufsize+1);
    if (!temp)
      {
      /* FAILURE */
      snprintf(log_buf,sizeof(log_buf),
        "out of space in buffer and cannot calloc message buffer (bufsize=%ld, buflen=%d, ct=%d)\n",
        tp->tdis_bufsize,
        (int)(tp->tdis_leadp - tp->tdis_thebuf),
        (int)ct);
      log_err(ENOMEM, __func__, log_buf);
      return(-1);
      }

    memcpy(temp, tp->tdis_thebuf, tp->tdis_bufsize);
    if (strlen(tp->tdis_thebuf) > tp->tdis_bufsize)
      {
      snprintf(log_buf, sizeof(log_buf),
          "line #%d, The length of the string is GREATER than the size of buf",
          __LINE__);
      log_err(ENOMEM, __func__, log_buf);
      }
    free(tp->tdis_thebuf);
    tp->tdis_thebuf = temp;
    tp->tdis_bufsize = newbufsize;
    tp->tdis_leadp = tp->tdis_thebuf - leadpct;
    tp->tdis_trailp = tp->tdis_thebuf - trailpct;
    tp->tdis_eod = tp->tdis_thebuf + newbufsize;

    }

  memcpy(tp->tdis_leadp, (char *)str, ct);

  tp->tdis_leadp += ct;

  return(ct);
  }  /* END tcp_puts() */




/*
 * tcp_rcommit - tcp/dis support routine to commit/uncommit read data
 */

int tcp_rcommit(

  struct tcp_chan *chan,
  int commit_flag)

  {

  struct tcpdisbuf *tp;

  tp = &chan->readbuf;

  if (commit_flag)
    {
    /* commit by moving trailing up */

    tp->tdis_trailp = tp->tdis_leadp;
    }
  else
    {
    /* uncommit by moving leading back */

    tp->tdis_leadp = tp->tdis_trailp;
    }

  return(0);
  }  /* END tcp_rcommit() */





/*
 * tcp_wcommit - tcp/dis support routine to commit/uncommit write data
 */

int tcp_wcommit(

  struct tcp_chan *chan,
  int commit_flag)

  {

  struct tcpdisbuf *tp;

  tp = &chan->writebuf;

  if (commit_flag)
    {
    /* commit by moving trailing up */

    tp->tdis_trailp = tp->tdis_leadp;
    }
  else
    {
    /* uncommit by moving leading back */

    tp->tdis_leadp = tp->tdis_trailp;
    }
  return(0);
  }



int tcp_chan_has_data(
    struct tcp_chan *chan)
  {
  int rc = FALSE;
  struct tcpdisbuf *tp;
  tp = &chan->readbuf;

  if (tp->tdis_eod != tp->tdis_leadp)
    rc = TRUE;
  return rc;
  }



/*
 * DIS_tcp_setup - setup supports routines for dis, "data is strings", to
 * use tcp stream I/O.  Also initializes an array of pointers to
 * buffers and a buffer to be used for the given fd.
 * 
 * NOTE:  tmpArray is global
 *
 * NOTE:  does not return FAILURE - FIXME
 */

struct tcp_chan * DIS_tcp_setup(

  int fd)

  {
  struct tcp_chan  *chan = NULL;
  struct tcpdisbuf *tp = NULL;

  /* check for bad file descriptor */
  if (fd < 0)
    {
    return(NULL);
    }

  if ((chan = (struct tcp_chan *)calloc(1, sizeof(struct tcp_chan))) == NULL)
    {
    log_err(ENOMEM, "DIS_tcp_setup", "calloc failure");
    return(NULL);
    }

  /* Assign socket to struct */
  chan->sock = fd;

  chan->reused = FALSE;

  /* Setting up the read buffer */
  tp = &chan->readbuf;
  if ((tp->tdis_thebuf = (char *)calloc(1, THE_BUF_SIZE+1)) == NULL)
    {
    free(chan);
    log_err(errno,"DIS_tcp_setup","calloc failure");
    return(NULL);
    }

  tp->tdis_bufsize = THE_BUF_SIZE;
  DIS_tcp_clear(tp);

  /* Setting up the write buffer */
  tp = &chan->writebuf;
  if ((tp->tdis_thebuf = (char *)calloc(1, THE_BUF_SIZE+1)) == NULL)
    {
    free(chan->readbuf.tdis_thebuf);
    free(chan);
    log_err(errno,"DIS_tcp_setup","calloc failure");
    return(NULL);
    }

  tp->tdis_bufsize = THE_BUF_SIZE;
  DIS_tcp_clear(tp);

  return(chan);
  }  /* END DIS_tcp_setup() */



/*
 * DIS_tcp_cleanup()
 *
 * The desctructor for tcp_chan - frees it and its members
 */

void DIS_tcp_cleanup(
    
  struct tcp_chan *chan)

  {
  struct tcpdisbuf *tp = NULL;

  if (chan == NULL)
    return;
  tp = &chan->readbuf;
  if (tp->tdis_thebuf != NULL)
    free(tp->tdis_thebuf);

  tp = &chan->writebuf;
  if (tp->tdis_thebuf != NULL)
    free(tp->tdis_thebuf);

  free(chan);
  } // END DIS_tcp_cleanup()



void DIS_tcp_close(
    
  struct tcp_chan *chan)

  {
  int sock = chan->sock;
  DIS_tcp_cleanup(chan);
  if (sock != -1)
    close(sock);
  }

/* END tcp_dis.c */