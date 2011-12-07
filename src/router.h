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

class flit_c
{
  public:
    flit_c();
    ~flit_c();
    void init(void);

  public:
    int        m_src;
    int        m_dst;
    int        m_dir;
    bool       m_head;
    bool       m_tail;
    mem_req_s* m_req;
    int        m_state;
    Counter    m_timestamp;
    Counter    m_rdy_cycle;
    Counter    m_rc_changed;
};

class router_wrapper_c;

class router_c
{
  friend class router_wrapper_c;

  public:
    router_c(macsim_c* simBase, int id);
    ~router_c();

    void run_a_cycle();
    bool send_packet(mem_req_s* req);
    void insert_packet(flit_c* flit, int ip, int ivc);
    int get_id(void);
    mem_req_s* receive_req(void);
    void pop_req(void);
    void init(int total_router, int* total_packet);
    void set_link(int dir, router_c* link);
    void print(ofstream& out);

  private:
    void link_traversal(void);
    void sw_traversal(void);
    void sw_allocation(void);
    void vc_allocation(void);
    void route_calculation(void);

    void pick_vc_winner(int op, int ovc, int& ip, int& ivc);
    void pick_swa_winner(int op, int& ip, int& ivc);

    void check_starvation(void);

  private:
    macsim_c* m_simBase;
    int m_id;
    int m_total_router;

    int m_num_vc;
    int m_num_port;
    int m_link_latency;
    int m_link_width;
    int* m_total_packet;

    pool_c<flit_c>* m_flit_pool;
    
    // virtual channel
    bool** m_ivc_avail;
    int** m_ivc_rc; /**< output port */
    int** m_ivc_vc; /**< output vc */
    list<flit_c*>** m_ivc_buffer;

    // switch
    bool* m_sw_avail;
    int* m_sw_ip;
    int* m_sw_vc;

    bool** m_ovc_avail;

    unordered_map<int, int> m_opposite_dir;

    //
    queue<mem_req_s*>* m_req_buffer;

    router_c* m_link[5];
};

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
    router_c* create_router(void);

    /**
     * Initialize interconnection network
     */
    void init(void);

    void print(void);

  private:
    router_wrapper_c(); // do not implement

  private:
    macsim_c* m_simBase; /**< sim base */
    vector<router_c*> m_router; /**< all routers */
    int m_num_router; /**< number of routers */
    int m_total_packet;
};

#endif
