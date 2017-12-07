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
/*
 * svr_attr_def is the array of pbs_attribute definitions for the server.
 * Each legal server pbs_attribute is defined here
 */

#include <pbs_config.h>		/* the master config generated by configure */

#include "pbs_ifl.h"
#include "list_link.h"
#include "attribute.h"

/* External Functions Referenced */

extern int manager_oper_chk (pbs_attribute * pattr, void *pobject, int actmode);
extern int servername_chk (pbs_attribute * pattr, void *pobject, int actmode);
extern int schiter_chk (pbs_attribute * pattr, void *pobject, int actmode);

extern int nextjobnum_chk (pbs_attribute * pattr, void *pobject, int actmode);
extern int set_nextjobnum (pbs_attribute * attr, pbs_attribute * new_attr,
			   enum batch_op op);

extern int poke_scheduler (pbs_attribute * pattr, void *pobject, int actmode);

extern int encode_svrstate (pbs_attribute * pattr, tlist_head * phead,
			    const char *aname, const char *rsname, int mode, int perm);

extern int decode_rcost (pbs_attribute * patr, const char *name, const char *rn, const char *val, int perm);
extern int encode_rcost (pbs_attribute * attr, tlist_head * phead, const char *atname,
			 const char *rsname, int mode, int perm);
extern int set_rcost (pbs_attribute * attr, pbs_attribute * new_attr, enum batch_op);
extern void free_rcost (pbs_attribute * attr);
extern int set_null (pbs_attribute * patr, pbs_attribute * new_attr, enum batch_op op);

extern int token_chk (pbs_attribute * pattr, void *pobject, int actmode);
extern int set_tokens (struct pbs_attribute *attr, struct pbs_attribute *new_attr,
		       enum batch_op op);

extern int extra_resc_chk (pbs_attribute * pattr, void *pobject, int actmode);
extern void free_extraresc (pbs_attribute * attr);
extern void restore_attr_default (struct pbs_attribute *);
int         update_user_acls(pbs_attribute *pattr, void *pobject, int actmode);
int         update_group_acls(pbs_attribute *pattr, void *pobject, int actmode);
int         node_exception_check(pbs_attribute *pattr, void *pobject, int actmode);
int         check_default_gpu_mode_str(pbs_attribute *pattr, void *pobject, int actmode);
extern int  keep_completed_val_check(pbs_attribute *pattr,void *pobj,int actmode);
/* DIAGTODO: write diag_attr_def.c */

/*
 * The entries for each pbs_attribute are (see attribute.h):
 * name,
 * decode function,
 * encode function,
 * set function,
 * compare function,
 * free value space function,
 * action function,
 * access permission flags,
 * value type
 */

/* define ATTR_XXX in pbs_ifl.h */
/* sync SRV_ATTR_XXX w/enum srv_atr in server.h */
/* define default in server_limits.h */
/* set default in pbsd_init() in pbsd_init.c */

attribute_def svr_attr_def[] =
  {

  /* SRV_ATR_State */
    { (char *)ATTR_status,  /* "server_state" */
    decode_null,
    encode_svrstate,
    set_null,
    comp_l,
    free_null,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER,
    },

  /* SRV_ATR_scheduling */
  { (char *)ATTR_scheduling, /* "scheduling" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    poke_scheduler,
    NO_USER_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER,
  },

  /* SRV_ATR_max_running */
  { (char *)ATTR_maxrun,  /* "max_running" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    free_null,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MaxUserRun */
  { (char *)ATTR_maxuserrun, /* "max_user_run" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    free_null,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MaxGrpRun */
  { (char *)ATTR_maxgrprun,  /* "max_group_run" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    free_null,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_TotalJobs */
  { (char *)ATTR_total,  /* "total_jobs" */
    decode_null,
    encode_l,
    set_null,
    comp_l,
    free_null,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobsByState */
  { (char *)ATTR_count,  /* "state_count" */
    decode_null,  /* note-uses fixed buffer in server struct */
    encode_str,
    set_null,
    comp_str,
    free_null,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_acl_host_enable */
  { (char *)ATTR_aclhten,  /* "acl_host_enable" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_acl_hosts */
  { (char *)ATTR_aclhost,  /* "acl_hosts" */
    decode_arst,
    encode_arst,
    set_hostacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AclUserEnabled */ /* User ACL to be used */
  { (char *)ATTR_acluren,  /* "acl_user_enable" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AclUsers */  /* User Acess Control List */
  { (char *)ATTR_acluser,  /* "acl_users" */
    decode_arst,
    encode_arst,
    set_uacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AclRoot */  /* List of which roots may execute jobs */
  { (char *)ATTR_aclroot,  /* "acl_roots"    */
    decode_arst,
    encode_arst,
    set_uacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_Gres_modifiers */  /* List of users who may modify the GRES for their own running jobs */
  { (char *)ATTR_gresmodifiers,  /* "acl_users" */
    decode_arst,
    encode_arst,
    set_uacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_managers */
  { (char *)ATTR_managers,  /* "managers" */
    decode_arst,
    encode_arst,
    set_uacl,
    comp_arst,
    free_arst,
    manager_oper_chk,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_operators */
  { (char *)ATTR_operators,  /* "operators" */
    decode_arst,
    encode_arst,
    set_uacl,
    comp_arst,
    free_arst,
    manager_oper_chk,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_dflt_que */
  { (char *)ATTR_dfltque,  /* "default_queue" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_log_events */
  { (char *)ATTR_logevents,  /* "log_events" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    restore_attr_default,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_mailfrom */
  { (char *)ATTR_mailfrom,  /* "mail_from" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_query_others */
  { (char *)ATTR_queryother, /* "query_other_jobs" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_resource_avail */
  { (char *)ATTR_rescavail,  /* "resources_available" */
    decode_resc,
    encode_resc,
    set_resc,
    comp_resc,
    free_resc,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_RESC,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_resource_deflt */
  { (char *)ATTR_rescdflt,  /* "resources_default" */
    decode_resc,
    encode_resc,
    set_resc,
    comp_resc,
    free_resc,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_RESC,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_ResourceMax */
  { (char *)ATTR_rescmax,  /* "resources_max" */
    decode_resc,
    encode_resc,
    set_resc,
    comp_resc,
    free_resc,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_RESC,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_resource_assn */
  { (char *)ATTR_rescassn,  /* "resources_assigned" */
    decode_resc,
    encode_resc,
    set_resc,
    comp_resc,
    free_resc,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_RESC,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_resource_cost */
  { (char *)ATTR_resccost,  /* "resources_cost" */
    decode_rcost, /* these are not right, haven't figured this out yet */
    encode_rcost,
    set_rcost,
    NULL_FUNC,
    free_rcost,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_RESC,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_sys_cost */
  { (char *)ATTR_syscost,  /* "system_cost" */
    decode_l,
    encode_l,
    set_l,
    NULL_FUNC,
    free_null,
    NULL_FUNC,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_schedule_iteration */
  { (char *)ATTR_schedit,  /* "schedule_iteration" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    free_noop,  /* disable unset */
    schiter_chk,
    NO_USER_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_ping_rate */
  {   (char *)ATTR_pingrate,          /* "node_ping_rate" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_check_rate */
  {   (char *)ATTR_ndchkrate,         /* "node_check_rate" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_noop,  /* disable unset */
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_tcp_timeout */
  {   (char *)ATTR_tcptimeout,         /* "tcp_timeout" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      restore_attr_default,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_Comment */
  { (char *)ATTR_comment,  /* "comment"  - information */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_DefNode */
  { (char *)ATTR_defnode,  /* "default_node" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_NodePack */
  { (char *)ATTR_nodepack,  /* "node_pack" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_NodeSuffix */
  {   (char *)ATTR_nodesuffix,        /* "node_suffix" */
      decode_str,
      encode_str,
      set_str,
      comp_str,
      free_str,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_STR,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobStatRate */
  { (char *)ATTR_jobstatrate, /* "job_stat_rate" */
    decode_l,
    encode_l,
    set_l,
    comp_l,
    restore_attr_default,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_PollJobs */
  { (char *)ATTR_polljobs,  /* "poll_jobs" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    restore_attr_default,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_LogLevel */
  {   (char *)ATTR_loglevel,  /* "log_level" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      restore_attr_default,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_DownOnError */
  {   (char *)ATTR_downonerror, /* "down_on_error" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_DisableServerIdCheck */
  {   (char *)ATTR_disableserveridcheck,       /* "disable_server_id_check" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobNanny */
  {   (char *)ATTR_jobnanny,  /* "job_nanny" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_OwnerPurge */
  {   (char *)ATTR_ownerpurge,       /* "owner_purge" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_QCQLimits */
  {   (char *)ATTR_qcqlimits,       /* "queue_centric_limits" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MomJobSync */
  { (char *)ATTR_momjobsync, /* "mom_job_sync" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MailDomain */
  { (char *)ATTR_maildomain, /* "mail_domain" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_version */
  { (char *)ATTR_pbsversion, /* "pbs_version" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_KillDelay */
  {   (char *)ATTR_killdelay,         /* "kill_delay" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AclLogic */
  {   (char *)ATTR_acllogic,          /* "acl_logic_or" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AclGroupSloppy */
  {   (char *)ATTR_aclgrpslpy,          /* "acl_group_sloppy" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_KeepCompleted */
  {   (char *)ATTR_keepcompleted,     /* "keep_completed" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      keep_completed_val_check,//in svr_func.c line 657
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_SubmitHosts */
  {   (char *)ATTR_submithosts,         /* "submit_hosts" */
      decode_arst,
      encode_arst,
      set_arst,
      comp_arst,
      free_arst,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_ARST,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AllowNodeSubmit */
  {   (char *)ATTR_allownodesubmit,     /* "allow_node_submit" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AllowProxyUser */
  {   (char *)ATTR_allowproxyuser,     /* "allow_proxy_user" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AutoNodeNP */
  {   (char *)ATTR_autonodenp,          /* "auto_node_np" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  }, 

  /* SRV_ATR_LogFileMaxSize */
  {   (char *)ATTR_logfilemaxsize,      /* "log_file_max_size" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_LogFileRollDepth */
  {   (char *)ATTR_logfilerolldepth,    /* "log_file_roll_depth" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SVR_ATR_LogKeepDays */
  {
      (char *)ATTR_logkeepdays,          /* "log_keep_days" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_NextJobNumber */
  { (char *)ATTR_nextjobnum,
    decode_l,
    encode_l,
    set_nextjobnum,
    comp_l,
    free_noop,
    nextjobnum_chk,
    MGR_ONLY_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_tokens */
  {  (char *)ATTR_tokens,
     decode_arst,
     encode_arst,
     set_tokens,
     comp_arst,
     free_arst,
     token_chk,
     MGR_ONLY_SET,
     ATR_TYPE_ARST,
     PARENT_TYPE_SERVER
  },

  /* SRV_ATR_NetCounter */
  { (char *)ATTR_netcounter,  /* "net_counter" */
    decode_null,
    encode_str,
    set_null,
    comp_str,
    free_null,
    NULL_FUNC,
    READ_ONLY,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_ExtraResc */
  {   (char *)ATTR_extraresc,  /* "extra_resc" */
      decode_arst,
      encode_arst,
      set_arst,
      comp_arst,
      free_extraresc,
      extra_resc_chk,
      NO_USER_SET,
      ATR_TYPE_ARST,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_ServerName */
  {   (char *)ATTR_servername,     /* "server_name" */
      decode_str,
      encode_str,
      set_str,
      comp_str,
      free_str,
      servername_chk,
      MGR_ONLY_SET,
      ATR_TYPE_STR,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_SchedVersion */
  {   (char *)ATTR_schedversion,     /* "sched_version" */
      decode_str,
      encode_str,
      set_str,
      comp_str,
      free_str,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_STR,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_AcctKeepDays */
  {   (char *)ATTR_acctkeepdays,      /* "accounting_keep_days" */
    	decode_l,
    	encode_l,
    	set_l,
    	comp_l,
    	free_null,
    	NULL_FUNC,
    	NO_USER_SET,
    	ATR_TYPE_LONG,
    	PARENT_TYPE_SERVER
  },

  /* SRV_ATR_lockfile */
  {	  (char *)ATTR_lockfile,		/* "lock_file" */
    	decode_str,
    	encode_str,
    	set_str,
    	comp_str,
    	free_str,
    	NULL_FUNC,
    	MGR_ONLY_SET,
    	ATR_TYPE_STR,
    	PARENT_TYPE_SERVER
  },

  /* SRV_ATR_LockfileUpdateTime */
  {   (char *)ATTR_LockfileUpdateTime, /* lock_file_update_time */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_LockfileCheckTime */
  {   (char *)ATTR_LockfileCheckTime, /* lock_file_check_time */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_CredentialLifetime */
  {   (char *)ATTR_credentiallifetime,  /* "credential_lifetime" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },
	
  /* SRV_ATR_JobMustReport */
  { (char *)ATTR_jobmustreport,      /* "job_must_report" */
    	decode_b,
    	encode_b,
    	set_b,
    	comp_b,
    	free_null,
    	NULL_FUNC,
    	MGR_ONLY_SET,
    	ATR_TYPE_BOOL,
    	PARENT_TYPE_SERVER
  },

  /* SRV_ATR_checkpoint_dir */
  {   (char *)ATTR_checkpoint_dir,   /* "checkpoint_dir" */
      decode_str,
      encode_str,
      set_str,
      comp_str,
      free_str,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_STR,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_display_job_server_suffix */
  { (char *)ATTR_dispsvrsuffix, /* "display_job_server_suffix" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_job_suffix_alias */
  { (char *)ATTR_jobsuffixalias, /* "job_suffix_alias" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_use_jobs_subdirs */
  { (char *)ATTR_usejobssubdirs, /* "use_jobs_subdirs" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MailSubjectFmt */
  { (char *)ATTR_mailsubjectfmt, /* "mail_subject_fmt" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MailBodyFmt */
  { (char *)ATTR_mailbodyfmt, /* "mail_body_fmt" */
    decode_str,
    encode_str,
    set_str,
    comp_str,
    free_str,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_STR,
    PARENT_TYPE_SERVER
  },
    /* SRV_ATR_NPDefault */
  {   (char *)ATTR_npdefault,          /* "np_default" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

    /* SRV_ATR_clonebatchsize */
  {   (char *)ATTR_clonebatchsize,          /* "clone_batch_size" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

    /* SRV_ATR_clonebatchdelay */
  {   (char *)ATTR_clonebatchdelay,          /* "clone_batch_delay" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobStartTimeout */
  {   (char *)ATTR_jobstarttimeout,         /* "job_start_timeout" */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      NO_USER_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobForceCancelTime */
  {   (char *)ATTR_jobforcecanceltime,     /* job_force_cancel_time */
      decode_l,
      encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MaxArraySize */
  {  (char *)ATTR_maxarraysize,           /* max_job_array_size */
     decode_l,
     encode_l,
     set_l,
     comp_l,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_LONG,
     PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MaxSlotLimit */
  {  (char *)ATTR_maxslotlimit,           /* max_slot_limit */
     decode_l,
     encode_l,
     set_l,
     comp_l,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_LONG,
     PARENT_TYPE_SERVER
  },

  /* SRV_ATR_RecordJobInfo */
  {  (char *)ATTR_recordjobinfo,         /* record_job_info */
     decode_b,
     encode_b,
     set_b,
     comp_b,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_BOOL,
     PARENT_TYPE_SERVER
  },

    /* SRV_ATR_RecordJobScript */
  {  (char *)ATTR_recordjobscript,         /* record_job_script */
     decode_b,
     encode_b,
     set_b,
     comp_b,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_BOOL,
     PARENT_TYPE_SERVER
  },

  /* SRV_ATR_JobLogFileMaxSize */
  {(char *)ATTR_joblogfilemaxsize,		/* "job_log_file_max_size" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   NO_USER_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_JobLogFileRollDepth */
  {(char *)ATTR_joblogfilerolldepth,	/* "job_log_file_roll_depth" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   NO_USER_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  /* SVR_ATR_JobLogKeepDays */
  {
   (char *)ATTR_joblogkeepdays,		/* "job_log_keep_days" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   NO_USER_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

#ifdef MUNGE_AUTH
  /* SRV_ATR_authusers */
  {(char *)ATTR_authusers,		/* "autorized_users" */
   decode_arst,
   encode_arst,
   set_uacl,
   comp_arst,
   free_arst,
   manager_oper_chk,
   MGR_ONLY_SET,
   ATR_TYPE_ACL,
   PARENT_TYPE_SERVER},
#endif

  /* SRV_ATR_minthreads */
  {ATTR_minthreads,               /* "min_threads" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER
  },

  /* SRV_ATR_maxthreads */
  {ATTR_maxthreads,               /* "max_threads" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER
  },

  /* SRV_ATR_threadidleseconds */
  {ATTR_threadidleseconds,        /* "thread_idle_seconds" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER
  },

  /* SRV_ATR_MoabArrayCompatible */
  {ATTR_moabarraycompatible, /* moab_array_compatible */
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   NO_USER_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER
  },

  /* SRV_ATR_NoMailForce */
  {ATTR_nomailforce, /* "no_mail_force" */
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_CrayEnabled */
  {ATTR_crayenabled,   /* "cray_enabled" */
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_InteractiveJobsCanRoam */ /* NOTE: this only has effect if pbs_server is cray enabled */
  {ATTR_interactivejobscanroam, /* "interactive_jobs_can_roam" */
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_MaxUserQueuable */
  {ATTR_maxuserqueuable, /* "max_user_queuable" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_AutomaticRequeueExitCode */
  {ATTR_automaticrequeueexitcode, /* automatic_requeue_exit_code */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_nppcu */
  {ATTR_nppcu,   /* "nppcu" */
   decode_nppcu,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_job_sync_timeout */
  {ATTR_jobsynctimeout,
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

   /* SRV_ATR_pass_cpu_clock */
  {ATTR_pass_cpu_clock,
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_job_full_report_time */
  {ATTR_job_full_report_time, /* "job_full_report_time" */
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},



  /* site supplied server pbs_attribute definitions if any, see site_svr_attr_*.h  */
#include "site_svr_attr_def.h"

   /* SRV_ATR_CopyOnRerun */
  {(char *)ATTR_copy_on_rerun, /* "copy_on_rerun" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER},

   /* SRV_ATR_JobExclusiveOnUse */
  {(char *)ATTR_job_exclusive_on_use, /* "job_exclusive_on_use" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER},

  /* SRV_ATR_DisableAutoRequeue */
  {(char *)ATTR_disable_automatic_requeue, /* "disable_automatic_requeue" */
   decode_b,
   encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER},

  /* SRV_ATR_ExitCodeCanceledJob */
   {(char *)ATTR_exitcodecanceledjob, /* "exit_code_canceled_job" */
    decode_l,
    encode_l,
     set_l,
     comp_l,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_LONG,
     PARENT_TYPE_SERVER},

    /* SRV_ATR_TimeoutForJobDelete */
    {(char *)ATTR_timeoutforjobdelete, /* "timeout_for_job_delete" */
     decode_l,
     encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER},

    /* SRV_ATR_TimeoutForJobRequeue */
    {(char *)ATTR_timeoutforjobrequeue, /* "timeout_for_job_requeue" */
     decode_l,
     encode_l,
      set_l,
      comp_l,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_LONG,
      PARENT_TYPE_SERVER},

    /* SRV_ATR_DontWriteNodesFile */
    {(char *)ATTR_dontwritenodesfile, /* "dont_write_nodes_file" */
     decode_b,
     encode_b,
     set_b,
     comp_b,
     free_null,
     NULL_FUNC,
     MGR_ONLY_SET,
     ATR_TYPE_BOOL,
     PARENT_TYPE_SERVER},

  /* SRV_ATR_acl_users_hosts */
  { (char *)ATTR_aclusershosts,  /* "acl_user_hosts" */
    decode_arst,
    encode_arst,
    set_hostacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_acl_groups_hosts */
  { (char *)ATTR_aclgroupshosts,  /* "acl_group_hosts" */
    decode_arst,
    encode_arst,
    set_hostacl,
    comp_arst,
    free_arst,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  /* SRV_ATR_node_submit_exceptions */
  { (char *)ATTR_nodesubmitexceptions,  /* "node_submit_exceptions" */
    decode_arst,
    encode_arst,
    set_arst,
    comp_arst,
    free_arst,
    node_exception_check,
    MGR_ONLY_SET,
    ATR_TYPE_ACL,
    PARENT_TYPE_SERVER
  },

  // SRV_ATR_LegacyVmem 
  {(char *)ATTR_legacy_vmem, // "legacy_vmem"
   decode_b,
   encode_b,
   set_b,
   comp_b,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_BOOL,
   PARENT_TYPE_SERVER},

  /* SRV_ATR_NoteAppendOnError*/
  {   (char *)ATTR_note_append_on_error, /* "note_append_on_error" */
      decode_b,
      encode_b,
      set_b,
      comp_b,
      free_null,
      NULL_FUNC,
      MGR_ONLY_SET,
      ATR_TYPE_BOOL,
      PARENT_TYPE_SERVER
  },

  // SRV_ATR_EmailBatchSeconds
  {(char *)ATTR_email_batch_seconds, // "email_batch_seconds"
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER},

  // SRV_ATR_tcp_incoming_timeout
  {(char *)ATTR_tcpincomingtimeout,         /* "tcp_incoming_timeout" */
  decode_l,
  encode_l,
  set_l,
  comp_l,
  free_null,
  NULL_FUNC,
  NO_USER_SET,
  ATR_TYPE_LONG,
  PARENT_TYPE_SERVER
  },
   
  // SRV_ATR_GhostArrayRecovery
  {(char *)ATTR_ghost_array_recovery, /* "ghost_array_recovery" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_BOOL,
    PARENT_TYPE_SERVER
  },

  // SRV_ATR_CgroupPerTask
  {(char *)ATTR_cgroup_per_task, /* "cgroup_per_task" */
    decode_b,
    encode_b,
    set_b,
    comp_b,
    free_null,
    NULL_FUNC,
    MGR_ONLY_SET,
    ATR_TYPE_LONG,
    PARENT_TYPE_SERVER
  },

  // SRV_ATR_IdleSlotLimit
  {(char *)ATTR_idle_slot_limit, // "idle_slot_limit"
   decode_l,
   encode_l,
   set_l,
   comp_l,
   free_null,
   NULL_FUNC,
   MGR_ONLY_SET,
   ATR_TYPE_LONG,
   PARENT_TYPE_SERVER
  },

  // SRV_ATR_DefaultGpuMode
  {(char *)ATTR_default_gpu_mode, // "default_gpu_mode"
   decode_str,
   encode_str,
   set_str,
   comp_str,
   free_null,
   check_default_gpu_mode_str,
   MGR_ONLY_SET,
   ATR_TYPE_STR,
   PARENT_TYPE_SERVER
  },

  };