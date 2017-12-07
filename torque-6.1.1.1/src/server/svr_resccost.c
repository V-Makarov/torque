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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include "pbs_ifl.h"
#include "list_link.h"
#include "attribute.h"
#include "resource.h"
#include "pbs_error.h"
#include "server_limits.h"
#include "server.h"
#include "pbs_job.h"

/*
 * This file contains the functions for manipulating the server
 * pbs_attribute "resource cost", which is of type ATR_TYPE_LIST
 *
 * It contains functions for:
 * Decoding the value string to the machine representation, a long
 *  integer within the resource cost structure.
 * Encoding the long integer value to external form
 * Setting the value by =, + or - operators.
 * Freeing the storage space used by the list.
 */
/*
 * note - it was my orginal intent to have the cost be an integer recorded
 * in the resource_defination structure itself.  It seemed logical, one
 * value per definition, why not.  But "the old atomic set" destroys that
 * idea.  Have to be able to have temporary attributes with their own
 * values...  Hence it came down to another linked-list of values.
 */

/*
 * Resource_cost entry, one per resource type which has been set.
 * The list is headed in the resource_cost pbs_attribute.
 */

struct resource_cost
  {
  list_link rc_link;
  resource_def   *rc_def;
  long  rc_cost;
  };




/*
 * add_cost_entry - add a new cost entry to the resource_cost list
 */

static struct resource_cost *add_cost_entry(

  pbs_attribute *patr,
  resource_def  *prdef)

  {
  struct resource_cost *pcost;

  pcost = (struct resource_cost *)calloc(1, sizeof(struct resource_cost));

  if (pcost != NULL)
    {
    CLEAR_LINK(pcost->rc_link);

    pcost->rc_def = prdef;
    pcost->rc_cost = 0;

    append_link(&patr->at_val.at_list, &pcost->rc_link, pcost);
    }

  return(pcost);
  }





/*
 * decode_rcost - decode string into resource cost value
 *
 * Returns: 0 if ok
 *  >0 error number if error
 *  *patr elements set
 */

int decode_rcost(

  pbs_attribute *patr,
  const char   *name,  /* pbs_attribute name */
  const char *rescn, /* resource name, unused here */
  const char    *val,   /* pbs_attribute value */
  int            perm)  /* used only with resources */

  {
  resource_def *prdef;

  struct resource_cost *pcost;
  void free_rcost(pbs_attribute *);

  if ((val == NULL) || (rescn == NULL))
    {
    patr->at_flags = (patr->at_flags & ~ATR_VFLAG_SET) | ATR_VFLAG_MODIFY;

    return(0);
    }

  if (patr->at_flags & ATR_VFLAG_SET)
    {
    free_rcost(patr);
    }

  prdef = find_resc_def(svr_resc_def, rescn, svr_resc_size);

  if (prdef == NULL)
    {
    return(PBSE_UNKRESC);
    }

  pcost = (struct resource_cost *)GET_NEXT(patr->at_val.at_list);

  while (pcost != NULL)
    {
    if (pcost->rc_def == prdef)
      break; /* have entry in attr already */

    pcost = (struct resource_cost *)GET_NEXT(pcost->rc_link);
    }

  if (pcost == NULL)
    {
    /* add entry */

    if ((pcost = add_cost_entry(patr, prdef)) == NULL)
      {
      return(PBSE_SYSTEM);
      }
    }

  pcost->rc_cost = atol(val);

  patr->at_flags |= ATR_VFLAG_SET | ATR_VFLAG_MODIFY;

  return(0);
  }





/*
 * encode_rcost - encode pbs_attribute of type long into attr_extern
 *
 * Returns: >0 if ok
 *   =0 if no value, no attrlist link added
 *   <0 if error
 */
/*ARGSUSED*/


int encode_rcost(

  pbs_attribute *attr,   /* ptr to pbs_attribute */
  tlist_head    *phead,   /* head of attrlist list */
  const char   *atname,  /* pbs_attribute name */
  const char   *rsname,  /* resource name or null */
  int            mode,   /* encode mode, unused here */
  int            perm)  /* used only with resources */

  {
  svrattrl *pal;

  struct resource_cost *pcost;

  if (!attr)
    {
    /* FAILURE */

    return(-1);
    }

  if (!(attr->at_flags & ATR_VFLAG_SET))
    {
    /* NO-OP */

    return(0);
    }

  pcost = (struct resource_cost *)GET_NEXT(attr->at_val.at_list);

  while (pcost != NULL)
    {
    rsname = pcost->rc_def->rs_name;

    if ((pal = attrlist_create(atname, rsname, 23)) == NULL)
      {
      /* FAILURE */

      return(-1);
      }

    snprintf(pal->al_value, 22, "%ld", pcost->rc_cost);

    pal->al_flags = attr->at_flags;

    append_link(phead, &pal->al_link, pal);

    pcost = (struct resource_cost *)GET_NEXT(pcost->rc_link);
    }

  return(1);
  }





/*
 * set_rcost - set pbs_attribute A to pbs_attribute B,
 * either A=B, A += B, or A -= B
 *
 * Returns: 0 if ok
 *  >0 if error
 */

int set_rcost(
   
  pbs_attribute *old,
  pbs_attribute *new_attr,
  enum batch_op  op)

  {
  struct resource_cost *pcnew;
  struct resource_cost *pcold;

  assert(old && new_attr && (new_attr->at_flags & ATR_VFLAG_SET));

  pcnew = (struct resource_cost *)GET_NEXT(new_attr->at_val.at_list);

  while (pcnew)
    {
    pcold = (struct resource_cost *)GET_NEXT(old->at_val.at_list);

    while (pcold)
      {
      if (pcnew->rc_def == pcold->rc_def)
        break;

      pcold = (struct resource_cost *)GET_NEXT(pcold->rc_link);
      }

    if (pcold == (struct resource_cost *)0)
      if ((pcold = add_cost_entry(old, pcnew->rc_def)) == (struct resource_cost *)0)
        return (PBSE_SYSTEM);

    switch (op)
      {

      case SET:
        pcold->rc_cost = pcnew->rc_cost;
        break;

      case INCR:
        pcold->rc_cost += pcnew->rc_cost;
        break;

      case DECR:
        pcold->rc_cost -= pcnew->rc_cost;
        break;

      default:
        return (PBSE_INTERNAL);
      }

    pcnew = (struct resource_cost *)GET_NEXT(pcnew->rc_link);
    }

  old->at_flags |= (ATR_VFLAG_SET | ATR_VFLAG_MODIFY);

  return (0);
  }


/*
 * free_rcost - free space used by resource cost pbs_attribute
 */

void free_rcost(
    
  pbs_attribute *pattr)

  {
  struct resource_cost *pcost;

  while ((pcost = (struct resource_cost *)GET_NEXT(pattr->at_val.at_list)))
    {
    delete_link(&pcost->rc_link);
    (void)free(pcost);
    }

  pattr->at_flags &= ~ATR_VFLAG_SET;
  }

/*
 * calc_job_cost - Compute the "job cost":
 * jc = sum(each resource cost * amount of resource) + per system cost
 *
 * If a resource is of type "size", the product is likely to be large,
 * so for those resources types, the cost is per MB.  All others, the
 * cost is per unit.
 */

long calc_job_cost(
    
  job *pjob)

  {
  long      amt;
  long      cost = 0;

  struct resource_cost *pcost;
  resource *pjobr;
  int   shiftct;

  pthread_mutex_lock(server.sv_attr_mutex);
  pcost = (struct resource_cost *)GET_NEXT(server.sv_attr[SRV_ATR_resource_cost].at_val.at_list);

  while (pcost)
    {
    pjobr = find_resc_entry(&pjob->ji_wattr[JOB_ATR_resource],
                            pcost->rc_def);

    if (pjobr)
      {
      switch (pcost->rc_def->rs_type)
        {

        case ATR_TYPE_LONG:
          cost += pjobr->rs_value.at_val.at_long * pcost->rc_cost;
          break;

        case ATR_TYPE_SIZE:
          amt = pjobr->rs_value.at_val.at_size.atsv_num;
          shiftct = pjobr->rs_value.at_val.at_size.atsv_shift - 20;

          if (shiftct < 0)
            amt = amt >> -shiftct;
          else
            amt = amt << shiftct;

          if (pjobr->rs_value.at_val.at_size.atsv_units)
            amt *= sizeof(int);

          cost += amt * pcost->rc_cost;

          break;
        }
      }

    pcost = (struct resource_cost *)GET_NEXT(pcost->rc_link);
    }

  cost += server.sv_attr[SRV_ATR_sys_cost].at_val.at_long;
  pthread_mutex_unlock(server.sv_attr_mutex);

  return (cost);
  }