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
#include "utils.h"
#include "debug_macros.h"
#include "all_knobs.h"
#include "statistics.h"

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

  m_sdm_counter = new int[m_max_application];
  for (int ii = 0; ii < m_max_application; ++ii) {
    m_sdm_counter[ii] = 0;
//    m_total_miss[ii] = 0;
  }

  m_total_miss = new Counter[m_max_application];

  m_sdm_max_counter_value = 
    static_cast<int>(pow(2, static_cast<int>(*m_simBase->m_knobs->KNOB_RRIP_CACHE_NUM_COUNTER_BIT)));

  m_bip_epsilon = *m_simBase->m_knobs->KNOB_RRIP_CACHE_BIP_EPSILON;
  m_access_count_by_type[2] = {0};
  m_total_access_count = 0;
  m_total_insert_count = 0;
  m_access_ratio = 0.0;
}


// destructor
cache_rrip_c::~cache_rrip_c()
{
}


// find an entry to be replaced based on the policy
cache_entry_c* cache_rrip_c::find_replacement_line(uns set, int appl_id) 
{
//  bool gpuline = m_simBase->m_PCL->get_appl_type(appl_id);
  bool gpuline = true;
  int index = -1;
  while (index == -1) {
    for (int ii = 0; ii < m_assoc; ++ii) {
      cache_entry_c* line = &(m_set[set]->m_entry[ii]);
      // find invalid or 2^n-1 entry
      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU) {
        if (line->m_valid != true || (line->m_gpuline == gpuline && line->m_last_access_time == m_max_lru_value)) {
          index = ii;
          break;
        }
      }
      else {
        if (line->m_valid != true || line->m_last_access_time == m_max_lru_value) {
          index = ii;
          break;
        }
      }
    }


    if (index == -1) {
      int count = 0;
      for (int ii = 0; ii < m_assoc; ++ii) {
        cache_entry_c* line = &(m_set[set]->m_entry[ii]);
        if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU) {
          if (line->m_gpuline == gpuline) {
            ++line->m_last_access_time;
            ++count;
          }
        }
        else {
          ++line->m_last_access_time;
        }
      }

      if (count == 0) {
        gpuline = !gpuline;
      }
    }
  }

  ++m_total_insert_count;
#if 0
  if (m_total_insert_count % 10000 == 0) {
    for (int ii = 0; ii < m_assoc; ++ii) {
      cache_entry_c* line = &(m_set[set]->m_entry[ii]);
      if (line->m_gpuline == false && line->m_last_access_time < m_max_lru_value)
        ++line->m_last_access_time;
    }
  }
#endif

  return &(m_set[set]->m_entry[index]);
}


// initialize a cache line
void cache_rrip_c::initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, 
    int appl_id, bool gpuline, int set_id, bool skip) 
{
  ins_line->m_valid            = true;
  ins_line->m_tag              = tag;
  ins_line->m_base             = (addr & ~m_offset_mask);
  ins_line->m_access_counter   = 0;
  ins_line->m_pref             = false;
  ins_line->m_appl_id          = appl_id;
  ins_line->m_gpuline          = gpuline;
  ins_line->m_skip             = skip;


  // SRRIP
  if (!*m_simBase->m_knobs->KNOB_RRIP_CACHE_DYNAMIC_ON) {
    if (*m_simBase->m_knobs->KNOB_RRIP_BIP_ALWAYS && gpuline) {
      // SRRIP
      if (rand() % 100 < m_bip_epsilon) {
        ins_line->m_last_access_time = m_insertion_value;
      }
      // LIP
      else {
        ins_line->m_last_access_time = m_max_lru_value;
      }
    }
    else {
      ins_line->m_last_access_time = m_insertion_value;
    }
  }
  // DRRIP
  else {
    // BIMODAL
    if ((*m_simBase->m_knobs->KNOB_RRIP_BIP_ALWAYS && gpuline) ||
        set_id % m_modulo == (appl_id * 2 + 1)) {
      // SRRIP
      if (rand() % 100 < m_bip_epsilon) {
        ins_line->m_last_access_time = m_insertion_value;
      }
      // LIP
      else {
        ins_line->m_last_access_time = m_max_lru_value;
      }
    }
    // SRRIP
    else if (set_id % m_modulo == (appl_id * 2)) {
      ins_line->m_last_access_time = m_insertion_value;
    }
    // Followers
    else {
      // performance not affected by the cache, so try to avoid caching/promotion
      bool no_cache = false;
      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU && gpuline) {
        if (*m_simBase->m_knobs->KNOB_COLLECT_CPI_INFO > 0 && m_simBase->m_PCL->get_psel_mask() == false)
          no_cache = true;

        if (*m_simBase->m_knobs->KNOB_COLLECT_CPI_INFO_FOR_MULTI_GPU > 0 && m_simBase->m_PCL->get_psel_mask(appl_id) == false)
          no_cache = true;
      }

      if (*m_simBase->m_knobs->KNOB_CACHE_FOR_STREAM_CPU && !gpuline) {
        no_cache = true;
      }

#if 0
      if (ins_line->m_skip) {
        no_cache = false;
      }
#endif

      // check access rate and try to insert CPU line closer to MRU if access ratio shows
      // significant difference between CPU and GPU
      int rrpv_delta = 0;
#if 0
      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU && m_simBase->m_PCL->get_appl_type((appl_id) == false)) {
        if (m_access_ratio >= 4.0) {
          rrpv_delta = 2;
        }
        else if (m_access_ratio >= 2.0) {
          rrpv_delta = 1;
        }
      }
#endif

#if 0
      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU && m_simBase->m_PCL->get_appl_type(appl_id) == true) {
        if (m_access_ratio >= 3.0) {
          rrpv_delta = -1;
        }

        if (ins_line->m_skip) {
          rrpv_delta = 0;
        }
      }
#endif
      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU && m_simBase->m_PCL->get_appl_type(appl_id) == true) {
        rrpv_delta = -1;
      }

      if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_MULTI_GPU) {
        rrpv_delta = 0;
      }

//      if (*m_simBase->m_knobs->KNOB_CACHE_FOR_STREAM_CPU && m_simBase->m_PCL->get_appl_type(appl_id) == false) {
//        rrpv_delta = -1;
//      }
      
      if (*m_simBase->m_knobs->KNOB_CACHE_FOR_STREAM_CPU) {
        rrpv_delta = -1;
      }


      // BIMODAL favor
      if (no_cache) {
        ins_line->m_last_access_time = m_max_lru_value;
      }
      // SRRIP favor
      else if (m_sdm_counter[appl_id] <= 0) {
        ins_line->m_last_access_time = m_insertion_value - rrpv_delta;
      }
      else {
      //else if (m_sdm_counter[appl_id] >= 0) {
        // with small probability, insert it to original position (2n-2)
        if (rand() % 100 < m_bip_epsilon) {
          ins_line->m_last_access_time = m_insertion_value - rrpv_delta;
        }
        // LIP insertion (2n-1)
        else {
          ins_line->m_last_access_time = m_max_lru_value;
        }
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
  // performance not affected by the cache, so try to avoid caching/promotion
  bool no_cache = false;
  if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_FOR_GPU && line->m_gpuline) {
//    if (rand() % 100 < 90)
//      no_cache = true;

    if (*m_simBase->m_knobs->KNOB_COLLECT_CPI_INFO > 0 && m_simBase->m_PCL->get_psel_mask() == false)
      no_cache = true;

    if (*m_simBase->m_knobs->KNOB_COLLECT_CPI_INFO_FOR_MULTI_GPU > 0 && m_simBase->m_PCL->get_psel_mask(line->m_appl_id) == false)
      no_cache = true;
  }


  if (*m_simBase->m_knobs->KNOB_CACHE_FOR_STREAM_CPU && !line->m_gpuline) {
    no_cache = true;
  }
      

  if (!no_cache && line->m_last_access_time > 0) {
//    --line->m_last_access_time;
    line->m_last_access_time = 0;
    //--line->m_last_access_time;
    if (m_simBase->m_PCL->get_appl_type(appl_id) == true)
      STAT_EVENT(L3_HIT_PROMOTION_GPU);
    else
      STAT_EVENT(L3_HIT_PROMOTION_CPU);
  }
}




void cache_rrip_c::update_cache_on_miss(int set_id, int appl_id)
{
  // DRRIP set dueling implementation
  if (*m_simBase->m_knobs->KNOB_RRIP_CACHE_DYNAMIC_ON) {
    m_total_miss[appl_id]++;
    // SRRIP
    if (set_id % m_modulo == (appl_id * 2)) {
      ++m_sdm_counter[appl_id];
      if (m_sdm_counter[appl_id] > m_sdm_max_counter_value) {
        m_sdm_counter[appl_id] = m_sdm_max_counter_value;
      }
//      if (appl_id != 0)
    }
    // BIMODAL
    else if (set_id % m_modulo == (appl_id * 2 + 1)) {
      --m_sdm_counter[appl_id];
      if (m_sdm_counter[appl_id] < -1 * m_sdm_max_counter_value) {
        m_sdm_counter[appl_id] = m_sdm_max_counter_value * -1;
      }
//      if (appl_id != 0)
//      cout << "[" << appl_id << "] [bip] couter:" << m_sdm_counter[appl_id] << "\n";
    }
      
    if (m_total_miss[appl_id] % 50000 == 0) {
      cout << "[" << appl_id << ":" << m_core_id << "] couter:" << m_sdm_counter[appl_id] << "\n";
    }
  }
}


void cache_rrip_c::update_cache_on_access(Addr line_addr, int set, int appl_id)
{
  if (m_total_access_count % 1000 == 0) {
    if (m_access_count_by_type[0] == 0) {
//      fprintf(g_mystderr, "cpu:%d gpu:%d nn\n", m_access_count_by_type[0], m_access_count_by_type[1]);
      m_access_ratio = 1000.0;
    }
    else {
      m_access_ratio = static_cast<float>(1.0 * m_access_count_by_type[1] / m_access_count_by_type[0]);
//      fprintf(g_mystderr, "cpu:%d gpu:%d %.2f\n", m_access_count_by_type[0], m_access_count_by_type[1], m_access_ratio);
    }
//    m_total_access_count = 0;
  }

  // GPU
  if (m_simBase->m_PCL->get_appl_type(appl_id) == true)
    ++m_access_count_by_type[1];
  // CPU
  else
    ++m_access_count_by_type[0];
  ++m_total_access_count;
}
