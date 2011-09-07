/**********************************************************************************************
 * File         : ucp.h
 * Author       : Jaekyu Lee
 * Date         : 04/26/2011 
 * SVN          : $Id: cache.h,
 * Description  : UCP (Qureshi and Patt MICRO06)
 *********************************************************************************************/


#ifndef RRIP_H
#define RRIP_H


#include <list>

#include "cache.h"


class cache_c;

class cache_ucp_c : public cache_c
{
  public:
    /**
     * Constructor
     */
    cache_ucp_c(int max_bit, string name, uns num_set, uns assoc, uns line_size, 
        uns data_size, uns bank_num, bool cache_by_pass, int core_id, 
        Cache_Type cache_type_info, bool enable_partition); 

    /**
     * Destructor
     */
    virtual ~cache_ucp_c();
    
    /**
     * fine a cache line to replace
     * \param set set id
     */
    cache_entry_c * find_replacement_line(uns set, int appl_id);
    
    /**
     * Initialize a new cache line
     */
    void initialize_cache_line(cache_entry_c *ins_line, Addr tag, Addr addr, int appl_id, 
        bool gpuline, int set_id);
    
    /**
     * Update set on replacement
     */
    void update_cache_on_access(Addr tag, int set, int appl_id);
    void update_atd(Addr tag, int appl_id, int set_id);
    

  private:
    /**
     * Default constructor - do not implement
     */
    cache_ucp_c(); // do not implement
    
    void update_partition(void);

    static const int m_max_application = 20;

    int* m_way_counter[m_max_application]; /**< counter for each ways */
    list<Addr>* m_atd[m_max_application]; /**< auxiliary tag directory */
    int m_quota[m_max_application];
    int* m_num_entry[m_max_application];

    int m_modulo; /**< modulo value for set monitorning */
    int m_last_partition_cycle;
    int m_num_way;
    int m_num_application;

};

#endif
