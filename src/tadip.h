/**********************************************************************************************
 * File         : rrip.h
 * Author       : Jaekyu Lee
 * Date         : 04/26/2011 
 * SVN          : $Id: cache.h,
 * Description  : TADIP (Jaleel et al. PACT 2008) 
 *********************************************************************************************/

#ifndef TADIP_H
#define TADIP_H

#include "cache.h"


class cache_c;

class cache_tadip_c : public cache_c
{
  public:
    /**
     * Constructor
     */
    cache_tadip_c(string name, uns num_set, uns assoc, uns line_size, uns data_size, 
        uns bank_num, bool cache_by_pass, int core_id, Cache_Type cache_type_info, 
        bool enable_partition, macsim_c* simBase); 

    /**
     * Destructor
     */
    virtual ~cache_tadip_c();
    
    /**
     * fine a cache line to replace
     * \param set set id
     */
    cache_entry_c * find_replacement_line(uns set, int appl_id);
    
    /**
     * Initialize a new cache line
     */
    void initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, int appl_id, 
        bool gpuline, int set_id, bool skip);

    /**
     * Update cache on cache misses - for set dueling
     */
    void update_cache_on_miss(int set_id, int appl_id);

  private:
    /**
     * Default constructor - do not implement
     */
    cache_tadip_c(); // do not implement

    static const int m_max_application = 20;

    int m_max_lru_value; /**< maximum lru value in RRIP */
    int m_insertion_value; /**< lru value upon insertion */
    int m_modulo; /**< modulo value for set monitorning */
    int *m_sdm_counter;
    Counter *m_total_miss;
    int m_sdm_max_counter_value;
    int m_bip_epsilon;
};

#endif
