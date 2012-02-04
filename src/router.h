/*
Copyright (c) <2012>, <Georgia Institute of Technology> All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted 
provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.

Redistributions in binary form must reproduce the above copyright notice, this list of 
conditions and the following disclaimer in the documentation and/or other materials provided 
with the distribution.

Neither the name of the <Georgia Institue of Technology> nor the names of its contributors 
may be used to endorse or promote products derived from this software without specific prior 
written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/


/**********************************************************************************************
 * File         : router.h 
 * Author       : Jaekyu Lee
 * Date         : 12/7/2011
 * SVN          : $Id: dram.h 867 2009-11-05 02:28:12Z kacear $:
 * Description  : Interconnection network
 *********************************************************************************************/


#ifndef ROUTER_H
#define ROUTER_H

#include <vector>
#include <unordered_map>
#include <queue>

#include "macsim.h"


#define CPU_ROUTER 0
#define GPU_ROUTER 1
#define L3_ROUTER 2
#define MC_ROUTER 3


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Flit data structure
///////////////////////////////////////////////////////////////////////////////////////////////
class flit_c
{
  public:
    /**
     * Constructor
     */
    flit_c();

    /**
     * Destructor
     */
    ~flit_c();

    /**
     * Initialize function
     */
    void init(void);

  public:
    int        m_src; /**< msg source */
    int        m_dst; /**< msg destination */
    int        m_dir; /**< direction */
    bool       m_head; /**< header flit? */
    bool       m_tail; /**< tail flit? */
    mem_req_s* m_req; /**< pointer to the memory request */
    int        m_state; /**< current state */
    Counter    m_timestamp; /**< timestamp */
    Counter    m_rdy_cycle; /**< ready cycle */
    Counter    m_rc_changed; /**< route changed? */
    bool       m_vc_batch; /**< VCA batch */
    bool       m_sw_batch; /**< SWA batch */
    int        m_id; /**< flit id */
};

class router_wrapper_c;


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_c
{
  friend class router_wrapper_c;

  public:
    /**
     * Constructor
     */
    router_c(macsim_c* simBase, int type, int id);

    /**
     * Destructor
     */
    ~router_c();

    /**
     * Run a cycle (tick) function
     */
    void run_a_cycle();

    /**
     * Inject a packet into the network
     */
    bool send_packet(mem_req_s* req);

    /**
     * Insert a packet into the next router
     */
    void insert_packet(flit_c* flit, int ip, int ivc);

    /**
     * Get router id
     */
    int get_id(void);

    /**
     * Set router id
     */
    void set_id(int id);

    /**
     * Eject a packet from the router
     */
    mem_req_s* receive_req(void);

    /**
     * Pop a request
     */
    void pop_req(void);

    /**
     * Initialize a router
     */
    void init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool);

    /**
     * Set links for a router
     */
    void set_link(int dir, router_c* link);

    /**
     * Printer the router information
     */
    void print(ofstream& out);

    /**
     * Print link information
     */
    void print_link_info();

  private:
    /**
     * LT (Link Traversal) stage
     */
    void link_traversal(void);

    /**
     * SWT (Switch Traversal) stage
     */
    void sw_traversal(void);

    /**
     * SWA (Switch Allocation) stage
     */
    void sw_allocation(void);

    /**
     * VCA (Virtual Channel Allocation) stage
     */
    void vc_allocation(void);

    /**
     * RC (Route Calculation) stage
     */
    void route_calculation(void);

    /**
     * VC arbitration
     */
    void pick_vc_winner(int op, int ovc, int& ip, int& ivc);

    /**
     * SW arbitration
     */
    void pick_swa_winner(int op, int& ip, int& ivc, int sw_id);

    /**
     * Get output virtual channel occupancy
     */
    int get_ovc_occupancy(int port);

    /**
     * Get output virtual channel occupancy per type
     */
    int get_ovc_occupancy(int port, bool type);

    /**
     * Check starvation
     */
    void check_starvation(void);

    /**
     * Check channel (for statistics)
     */
    void check_channel(void);

  private:
    macsim_c* m_simBase; /**< pointer to simulation base class */
    int m_type; /**< router type */
    int m_id; /**< router id */
    int m_total_router; /**< number of total routers */

    int m_num_vc; /**< number of virtual channels */
    int m_num_port; /**< number of ports */
    int m_link_latency; /**< link latency */
    int m_link_width; /**< link width */
    int* m_total_packet; /**< number of total packets (global) */
    bool m_vc_partition; /**< vc partitioning enable */
    int m_cpu_partition; /**< number of vcs for CPU */
    int m_gpu_partition; /**< number of vcs for GPU */
 
    bool m_channel_partition; /**< pc partitioning enable */
    int m_cpu_ch_partition; /**< number of pcs for CPU */
    int m_gpu_ch_partition; /**< number of pcs for GPU */

    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
    
    // virtual channel
    bool** m_ivc_avail; /**< input vc availability */
    int** m_ivc_rc; /**< output port id */
    int** m_ivc_vc; /**< output vc */
    int** m_ivc_sw; /**< output switch id */
    list<flit_c*>** m_ivc_buffer; /**< ivc buffer */

    // switch
    bool** m_sw_avail; /**< switch availability */
    int** m_sw_ip; /**< per switch input port availability */ 
    int** m_sw_vc; /**< per switch vc availability */
    int m_num_switch; /**< number of switch */

    bool** m_ovc_avail; /**< number of output vc availablity */

    unordered_map<int, int> m_opposite_dir; /**< opposite direction map */

    // arbitration
    int* m_last_vc_winner; /**< last vca winner */
    int* m_last_sw_winner; /**< last swa winner */
    int m_arbitration_policy; /**< arbitration policy */

    int m_sw_iter; /**< number of switch allocation stage iteration */

    queue<mem_req_s*>* m_req_buffer; /**< request buffer */
    router_c* m_link[5]; /**< links */
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Router Wrapper Class
///////////////////////////////////////////////////////////////////////////////////////////////
class router_wrapper_c
{
  public:
    /**
     * Constructor
     */
    router_wrapper_c(macsim_c* simBase);

    /**
     * Destructor
     */
    ~router_wrapper_c();

    /**
     * Run a cycle
     */
    void run_a_cycle(void);

    /**
     * Create a router
     */
    router_c* create_router(int type);

    /**
     * Initialize interconnection network
     */
    void init(void);

    /**
     * Print all router information
     */
    void print(void);

  private:
    router_wrapper_c(); // do not implement

  private:
    macsim_c* m_simBase; /**< sim base */
    vector<router_c*> m_router; /**< all routers */
    int m_num_router; /**< number of routers */
    int m_total_packet; /**< number of total packets */
    pool_c<flit_c>* m_flit_pool; /**< flit data structure pool */
};

#endif
