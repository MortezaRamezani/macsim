/**********************************************************************************************
 * File         : ucp.cc
 * Author       : Jaekyu Lee
 * Date         : 04/26/2011 
 * SVN          : $Id: cache.h,
 * Description  : UCP (Qureshi and Patt MICRO06)
 *********************************************************************************************/


#include <cmath>
#include <cassert>

#include "ucp.h"
#include "cache.h"
#include "debug_macros.h"

#include "memory.param.h"


#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_CACHE_LIB, ## args)
#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE, ## args)


// constructor
cache_ucp_c::cache_ucp_c(int max_bit, string name, uns num_set, uns assoc, uns line_size, 
    uns data_size, uns bank_num, bool cache_by_pass, int core_id, Cache_Type cache_type_info, 
    bool enable_partition) : cache_c(name, num_set, assoc, line_size, data_size, bank_num,
      cache_by_pass, core_id, cache_type_info, enable_partition)
{
  // assume 32 sets for 1 SDM
  m_modulo = num_set / 32;
  m_num_way = assoc;
  m_num_application = *m_simBase->m_knobs->KNOB_UCP_CACHE_NUM_APPLICATION;

  for (int ii = 0; ii < m_max_application; ++ii) {
    m_way_counter[ii] = new int[assoc];
    fill_n(m_way_counter[ii], assoc, 0); 
  }

  for (int ii = 0; ii < m_max_application; ++ii) {
    m_atd[ii] = new list<Addr>[32];
    for (int jj = 0; jj < assoc; ++jj) {
      m_atd[ii]->push_back(0);
    }
  }

  for (int ii = 0; ii < m_max_application; ++ii) {
    m_num_entry[ii] = new int[num_set];
    fill_n(m_num_entry[ii], num_set, 0);
  }

  // initially assign equal amount
  for (int ii = 0; ii < m_max_application; ++ii) {
    m_quota[ii] = num_set / m_num_application;
  }

  m_last_partition_cycle = *m_simBase->m_knobs->KNOB_UCP_CACHE_PARTITION_PERIOD;
}


// destructor
cache_ucp_c::~cache_ucp_c()
{
}


// find an entry to be replaced based on the policy
cache_entry_c* cache_ucp_c::find_replacement_line(uns set, int appl_id) 
{
  int index = -1;
  Counter min_lru = g_simulation_cycle + 1;
  if (appl_id == -1) {
    for (int ii = 0; ii < m_assoc; ++ii) {
      cache_entry_c* line = &(m_set[set]->m_entry[ii]);
      // find invalid or LRU entry
      if (line->m_valid != true) {
        index = ii;
        break;
      }

      if (line->m_last_access_time < min_lru) {
        index = ii;
        min_lru = line->m_last_access_time;
      }
    }
  }
  else {
    // evict a block from other applications
    if (m_num_entry[appl_id][set] < m_quota[appl_id]) {
      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        if (line->m_valid != true) {
          index = ii;
          break;
        }

        if (line->m_appl_id != appl_id && line->m_last_access_time < min_lru) {
          index = ii;
          min_lru = line->m_last_access_time;
        }
      }
    }
    // evict a block from own
    else {
      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        if (line->m_appl_id == appl_id && line->m_last_access_time < min_lru) {
          index = ii;
          min_lru = line->m_last_access_time;
        }
      }
    }
  }

  assert(index != -1);

  return &(m_set[set]->m_entry[index]);
}


// initialize a cache line
void cache_ucp_c::initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, 
    int appl_id, bool gpuline, int set_id) 
{
  ins_line->m_valid            = true;
  ins_line->m_tag              = tag;
  ins_line->m_base             = (addr & ~m_offset_mask);
  ins_line->m_access_counter   = 0;
  ins_line->m_pref             = false;
  ins_line->m_appl_id          = appl_id;
  ins_line->m_gpuline          = gpuline;
  ins_line->m_last_access_time = g_simulation_cycle;

  ++m_num_entry[appl_id][set_id];
}


void cache_ucp_c::update_cache_on_access(Addr line_addr, int set, int appl_id)
{
  if (set % m_modulo == 0) {
    bool hit = false;
    int count = 0;
    for (auto I = m_atd[appl_id][set].begin(), E = m_atd[appl_id][set].end(); I != E; ++I) {
      if ((*I) == line_addr) {
        ++m_way_counter[appl_id][count];
        hit = true;
        break;
      }
      ++count;
    }

    if (!hit) {
      m_atd[appl_id][set].pop_back();
    }
    else {
      m_atd[appl_id][set].remove(line_addr);
    }
    m_atd[appl_id][set].push_front(line_addr);
  } 

  if (g_simulation_cycle - m_last_partition_cycle >= *m_simBase->m_knobs->KNOB_UCP_CACHE_PARTITION_PERIOD) {
    update_partition();
  }
}


// update application partition every *m_simBase->m_knobs->KNOB_UCP_CACHE_PARTITION_PERIOD cycle
// use lookahead algorithm
void cache_ucp_c::update_partition(void)
{
  // assign at least 1 way
  fill_n(m_quota, m_max_application, 1);
  int count = m_num_application;
  while (count++ < m_num_way) {
    int max = m_way_counter[0][m_quota[0]];
    int max_appl_id = 0;
    for (int ii = 1; ii < m_num_application; ++ii) {
      if (m_way_counter[ii][m_quota[ii]] > max) {
        max = m_way_counter[ii][m_quota[ii]];
        max_appl_id = ii;
      }
    }
    
    ++m_quota[max_appl_id];
  }
}

