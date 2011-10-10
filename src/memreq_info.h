/**********************************************************************************************
 * File         : memreq_info.h
 * Author       : Hyesoon Kim
 * Date         : 3/20/2008
 * SVN          : $Id: memreq_info.h,v 1.5 2008-09-17 21:01:41 kacear Exp $:
 * Description  : Memory request information
 *********************************************************************************************/


#ifndef MEMREQ_INFO_H_INCLUDED
#define MEMREQ_INFO_H_INCLUDED


#include <string>
#include <list>
#include <deque>
#include <functional>

#define DONT_INCLUDE_MANIFOLD
#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory component type
///////////////////////////////////////////////////////////////////////////////////////////////
enum MEMORY_TYPE {
  MEM_L1 = 1,
  MEM_L2,
  MEM_L3,
  MEM_MC,
  MEM_LAST,
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief NOC request type
///////////////////////////////////////////////////////////////////////////////////////////////
enum NOC_TYPE {
  NOC_FILL,
  NOC_NEW,
  NOC_LAST,
  MAX_NOC_STATE,
};

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory request state
///
/// if you change this order or add anything, fix mem_state[] in memreq_info.cc
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Mem_Req_State_enum {
  MEM_INV,
  MEM_NEW,
  MEM_MERGED,
  MEM_OUTQUEUE_NEW,
  MEM_IN_NOC,
  MEM_OUT_FILL,
  MEM_OUT_WB,
  MEM_FILL_NEW,
  MEM_FILL_WAIT_DONE,
  MEM_FILL_WAIT_FILL,
  MEM_DRAM_START,
  MEM_DRAM_CMD,
  MEM_DRAM_DATA,
  MEM_DRAM_DONE,
  MEM_NOC_START,
  MEM_NOC_DONE,
  MEM_STATE_MAX,
} Mem_Req_State;



///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Memory request type
///
/// if you change this order or add anything, 
/// fix Mem_Req_Priority_Offset [] and mem_req_type_names [] in memreq_info.cc
///////////////////////////////////////////////////////////////////////////////////////////////
typedef enum Mem_Req_Type_enum {
  MRT_IFETCH,			
  MRT_DFETCH,
  MRT_DSTORE,
  MRT_IPRF,
  MRT_DPRF,
  MRT_WB,
  MRT_SW_DPRF,
  MRT_SW_DPRF_NTA,
  MRT_SW_DPRF_T0,
  MRT_SW_DPRF_T1,
  MRT_SW_DPRF_T2,
  MAX_MEM_REQ_TYPE,
} Mem_Req_Type;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief prefetch request information structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct pref_req_info_s {
  Counter m_prefetcher_id;      /**< prefetcher id */
  Addr    m_loadPC;             /**< load pc */
  int     m_core_id;            /**< core id */
} pref_req_info_s;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief memory request data structure
///////////////////////////////////////////////////////////////////////////////////////////////
typedef struct mem_req_s {
  /**
   * Constructor
   */
  mem_req_s(macsim_c* simBase);

  int           m_id;           /**< unique request id */                         
  int           m_appl_id;      /**< application id */
  int           m_core_id;      /**< core id */
  int           m_thread_id;    /**< thread id */
  int           m_block_id;     /**< GPU block id */
  Mem_Req_State m_state;        /**< memory request state */   
  Mem_Req_Type  m_type;         /**< request type */
  Counter       m_priority;     /**< priority */
  Addr          m_addr;         /**< request address */ 
  uns           m_size;         /**< request size */
  Counter       m_rdy_cycle;    /**< request ready cycle */
  Addr          m_pc;           /**< load pc */
  uns8          m_prefetcher_id; /**< prefetcher id, if prefetch request */
  Addr          m_pref_loadPC;  /**< prefetch load pc */
  bool          m_ptx;          /**< GPU request */
  queue_c*      m_queue;        /**< current memory queue in */
  int           m_cache_id[MEM_LAST]; /**< each level cache id */
  uop_c*        m_uop;          /**< uop that generates this request */
  Counter       m_in;           /**< request inserted cycle */
  bool          m_dirty;        /**< wb request? */
  bool          m_done;         /**< request done flag */
  mem_req_s*    m_merged_req;   /**< merged request */
  int           m_msg_type;     /**< noc request type */
  int           m_msg_src;      /**< source node id */
  int           m_msg_dst;      /**< destination node id */
  int           m_bypass;       /**< bypass last level cache */
  bool          m_skip; 

  macsim_c*     m_simBase;      /**< reference to macsim base class for sim globals */
  
  function<bool (mem_req_s*)> m_done_func; /**< done function */
  list<mem_req_s*> m_merge; /**< merged request list */
} mem_req_s;


class mem_req_c {
  public:
    static const char* mem_req_type_name[MAX_MEM_REQ_TYPE]; // memory request type string
    static const char* mem_state[MEM_STATE_MAX];            // memory request state string
    
};

#endif  /* #ifndef MEMORY_H_INCLUDED  */
