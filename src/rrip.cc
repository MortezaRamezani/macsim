/**********************************************************************************************
 * File         : rrip.cc
 * Author       : Jaekyu Lee
 * Date         : 04/26/2011 
 * SVN          : $Id: cache.h,
 * Description  : RRIP (Jaleel et al. ISCA 2010) 
 *********************************************************************************************/


#include <cmath>

#include "rrip.h"
#include "cache.h"
#include "debug_macros.h"

#include "all_knobs.h"

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_CACHE_LIB, ## args)
#define DEBUG_MEM(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_MEM_TRACE, ## args)


// constructor
cache_rrip_c::cache_rrip_c(int max_bit, string name, uns num_set, uns assoc, uns line_size, 
    uns data_size, uns bank_num, bool cache_by_pass, int core_id, Cache_Type cache_type_info, 
    bool enable_partition, macsim_c* simBase) : cache_c(name, num_set, assoc, line_size, 
      data_size, bank_num, cache_by_pass, core_id, cache_type_info, enable_partition, simBase)
{
  m_max_lru_value   = static_cast<int>(pow(2, max_bit) - 1);
  m_insertion_value = *m_simBase->m_knobs->KNOB_RRIP_CACHE_INSERT_AT;

  // assume 32 sets for 1 SDM
  m_modulo = num_set / 32;

  for (int ii = 0; ii < m_max_application; ++ii) {
    m_sdm_counter[ii] = 0;
  }

  m_sdm_max_counter_value = 
    static_cast<int>(pow(2, static_cast<int>(*m_simBase->m_knobs->KNOB_RRIP_CACHE_NUM_COUNTER_BIT)));

  m_bip_epsilon = *m_simBase->m_knobs->KNOB_RRIP_CACHE_BIP_EPSILON;
}


// destructor
cache_rrip_c::~cache_rrip_c()
{
}


// find an entry to be replaced based on the policy
cache_entry_c* cache_rrip_c::find_replacement_line(uns set, int appl_id) 
{
  int index = -1;
  while (index == -1) {
    for (int ii = 0; ii < m_assoc; ++ii) {
      cache_entry_c* line = &(m_set[set]->m_entry[ii]);
      // find invalid or 2^n-1 entry
      if (line->m_valid != true || line->m_last_access_time == m_max_lru_value) {
        index = ii;
        break;
      }
    }

    if (index == -1) {
      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        ++line->m_last_access_time;
      }
    }
  }

  return &(m_set[set]->m_entry[index]);
}


// initialize a cache line
void cache_rrip_c::initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, 
    int appl_id, bool gpuline, int set_id) 
{
  ins_line->m_valid            = true;
  ins_line->m_tag              = tag;
  ins_line->m_base             = (addr & ~m_offset_mask);
  ins_line->m_access_counter   = 0;
  ins_line->m_pref             = false;
  ins_line->m_appl_id          = appl_id;
  ins_line->m_gpuline          = gpuline;

  // SRRIP
  if (!*m_simBase->m_knobs->KNOB_RRIP_CACHE_DYNAMIC_ON) {
    ins_line->m_last_access_time = m_insertion_value;
  }
  // DRRIP
  else {
    // SRRIP
    if (set_id % m_modulo == (appl_id * 2)) {
      ins_line->m_last_access_time = m_insertion_value;
    }
    // BIMODAL
    else if (set_id % m_modulo == (appl_id * 2 + 1)) {
      // SRRIP
      if (rand() % 100 < m_bip_epsilon) {
        ins_line->m_last_access_time = m_insertion_value;
      }
      // LIP
      else {
        ins_line->m_last_access_time = m_max_lru_value;
      }
    }
    // Followers
    else {
      // BIMODAL favor
      if (m_sdm_counter[appl_id] >= 0) {
        if (rand() % 100 < m_bip_epsilon) {
          ins_line->m_last_access_time = m_insertion_value;
        }
        else {
          ins_line->m_last_access_time = m_max_lru_value;
        }
      }
      // SRRIP favor
      else {
        ins_line->m_last_access_time = m_insertion_value;
      }
    }
  }


  if (ins_line->m_gpuline) { 
    ++m_num_gpu_line;
    ++m_set[set_id]->m_num_gpu_line;
  }
  else {
    ++m_num_cpu_line;
    ++m_set[set_id]->m_num_cpu_line;
  }
}


// update a line upon cache hits
// use FP (Frequency Priority)
void cache_rrip_c::update_line_on_hit(cache_entry_c* line, int set, int appl_id)
{
  if (line->m_last_access_time > 0)
    --line->m_last_access_time;
}


void cache_rrip_c::update_cache_on_miss(int set_id, int appl_id)
{
  // DRRIP set dueling implementation
  if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_DYNAMIC_ON) {
    // SRRIP
    if (set_id % m_modulo == (appl_id * 2)) {
      ++m_sdm_counter[appl_id];
      if (m_sdm_counter[appl_id] > m_sdm_max_counter_value) {
        m_sdm_counter[appl_id] = m_sdm_max_counter_value;
      }
    }
    // BIMODAL
    else if (set_id % m_modulo == (appl_id * 2 + 1)) {
      --m_sdm_counter[appl_id];
      if (m_sdm_counter[appl_id] < -1 * m_sdm_max_counter_value) {
        m_sdm_counter[appl_id] = m_sdm_max_counter_value * -1;
      }
    }
  }
}

