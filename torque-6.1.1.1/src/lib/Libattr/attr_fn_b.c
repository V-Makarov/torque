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

#include <assert.h>
#include <ctype.h>
#include <memory.h>
#ifndef NDEBUG
#include <stdio.h>
#endif
#include <stdlib.h>
#include <string.h>
#include "pbs_ifl.h"
#include "list_link.h"
#include "attribute.h"
#include "pbs_error.h"
#include "pbs_helper.h"

/*
 * This file contains functions for manipulating attributes of type
 * boolean
 *
 * Each set has functions for:
 * Decoding the value string to the machine representation.
 * Encoding the machine representation of the value to a string
 * Setting the value by =, + or - operators.
 * Comparing a (decoded) value with the pbs_attribute value.
 * Freeing the space calloc-ed to the pbs_attribute value.
 *
 * Some or all of the functions for an pbs_attribute type may be shared with
 * other pbs_attribute types.
 *
 * The prototypes are declared in "attribute.h"
 *
 * -------------------------------------------------------
 * Set of General functions for attributes of type boolean
 * -------------------------------------------------------
 */

static const char *true_val  = ATR_TRUE;
static const char *false_val = ATR_FALSE;

/*
 * decode_b - decode string into boolean pbs_attribute
 *
 * String of "1" decodes to true, all else to false
 * Returns: 0 if ok
 *  >0 error number if error
 *  *patr elements set
 */

#define MAX_VAL_LEN 10

int decode_b(

  pbs_attribute *patr,
  const char * UNUSED(name),  /* pbs_attribute name */
  const char * UNUSED(rescn),  /* resource name, unused here */
  const char    *val,  /* pbs_attribute value */
  int          UNUSED(perm)) /* only used for resources */

  {
  const char *src;
  char *dst;
  char ucVal[MAX_VAL_LEN+1];

  if ((val == (char *)0) || (strlen(val) == 0))
    {
    patr->at_flags = (patr->at_flags & ~ATR_VFLAG_SET) |
                     ATR_VFLAG_MODIFY;
    patr->at_val.at_bool = false;  /* default to false */
    }
  else
    {
    for (src = val,dst = ucVal; *src && (dst < (ucVal + MAX_VAL_LEN)); src++,dst++)
      *dst = (char)toupper((int) * src);
    *dst='\0';

    if ((strcmp(ucVal, true_val) == 0) ||
        (strcmp(ucVal, "TRUE") == 0)   ||
        (strcmp(ucVal, "true") == 0)   ||
        (strcmp(ucVal, "t") == 0)  ||
        (strcmp(ucVal, "T") == 0)  ||
        (strcmp(ucVal, "1") == 0)  ||
        (strcmp(ucVal, "y") == 0)  ||
        (strcmp(ucVal, "Y") == 0))
      patr->at_val.at_bool = true;
    else if ((strcmp(ucVal, false_val) == 0) ||
             (strcmp(ucVal, "FALSE") == 0)   ||
             (strcmp(ucVal, "false") == 0)   ||
             (strcmp(ucVal, "f") == 0)       ||
             (strcmp(ucVal, "F") == 0)       ||
             (strcmp(ucVal, "0") == 0)       ||
             (strcmp(ucVal, "n") == 0)       ||
             (strcmp(ucVal, "N") == 0))
      patr->at_val.at_bool = false;
    else
      return (PBSE_BADATVAL);

    patr->at_flags |= ATR_VFLAG_SET | ATR_VFLAG_MODIFY;
    }

  return (0);
  }

/*
 * encode_b - encode pbs_attribute of type ATR_TYPE_LONG to attr_extern
 *
 * Returns: >0 size of data returned to buffer
 *   =0 if not value, no link added
 *   <0 if error
 */
/*ARGSUSED*/

int encode_b(

  pbs_attribute  *attr,   /* ptr to pbs_attribute */
  tlist_head     *phead,  /* head of pbs_attribute list */
  const char     *atname, /* pbs_attribute name */
  const char     *rsname, /* resource name or null */
  int            UNUSED(mode),   /* encode mode, unused here */
  int            UNUSED(perm))   /* only used for resources */

  {
  size_t   ct;
  svrattrl *pal;
  const char  *value;

  if (!attr)
    return (-1);

  if (!(attr->at_flags & ATR_VFLAG_SET))
    return (0);

  if (attr->at_val.at_bool)
    {
    value = true_val;
    }
  else
    {
    value = false_val;
    }

  ct = strlen(value);

  pal = attrlist_create(atname, rsname, ct+1);

  if (pal == (svrattrl *)0)
    return (-1);

  strncpy(pal->al_value, value, ct);

  pal->al_flags = attr->at_flags;

  append_link(phead, &pal->al_link, pal);

  return (1);
  }

/*
 * set_b - set pbs_attribute of type ATR_TYPE_LONG
 *
 * A=B --> A set to value of B
 * A+B --> A = A | B  (inclusive or, turn on)
 * A-B --> A = A & ~B  (and not, clear)
 *
 * Returns: 0 if ok
 *  >0 if error
 */

int set_b(
    
  pbs_attribute *attr,
  pbs_attribute *new_attr,
  enum batch_op  op)

  {
  assert(attr && new_attr && (new_attr->at_flags & ATR_VFLAG_SET));

  switch (op)
    {

    case SET:
      attr->at_val.at_bool = new_attr->at_val.at_bool;
      break;

    case INCR:
      attr->at_val.at_bool = attr->at_val.at_bool || new_attr->at_val.at_bool; /* "or" */
      break;

    case DECR:
      attr->at_val.at_bool = attr->at_val.at_bool && !new_attr->at_val.at_bool;
      break;

    default:
      return (PBSE_INTERNAL);
    }

  attr->at_flags |= ATR_VFLAG_SET | ATR_VFLAG_MODIFY;

  return (0);
  }

/*
 * comp_b - compare two attributes of type ATR_TYPE_LONG
 *
 * Returns: 0 if same value
 *  +1 if different value
 */

int comp_b(
    
  pbs_attribute *attr,
  pbs_attribute *with)

  {
  if (!attr || !with)
    return (1);

  if (((attr->at_val.at_bool == false) && (with->at_val.at_bool == false)) ||
      ((attr->at_val.at_bool != false) && (with->at_val.at_bool != false)))
    return (0);
  else
    return (1);
  }

/*
 * free_b - use free_null() to (not) free space
 */