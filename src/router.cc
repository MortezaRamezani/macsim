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


#include <fstream>

#include "router.h"
#include "all_knobs.h"
#include "all_stats.h"
#include "utils.h"
#include "debug_macros.h"
#include "assert_macros.h"

#define LOCAL 0
#define LEFT  1
#define RIGHT 2
#define UP    3
#define DOWN  4

#define HEAD 0
#define BODY 1
#define TAIL 2

#define INIT 0
#define IB   1
#define RC   2
#define VCA  3
#define SA   4
#define ST   5
#define LT   6

#define OLDEST_FIRST 0
#define ROUND_ROBIN  1
#define CPU_FIRST    2
#define GPU_FIRST    3

#define GPU_FRIENDLY 0
#define CPU_FRIENDLY 1
#define MIXED        2
#define INTERLEAVED  3



#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ## args)

/////////////////////////////////////////////////////////////////////////////////////////
// m_sw_avail : switch availability
//   allocated to a flit with VCA stage
//   deallocated when this flit finishes LT stage
/////////////////////////////////////////////////////////////////////////////////////////


int g_total_packet = 0;
int g_total_cpu_packet = 0;
int g_total_gpu_packet = 0;

flit_c::flit_c()
{
  init();
}

flit_c::~flit_c()
{
}

void flit_c::init(void)
{
  m_src       = -1;
  m_dst       = -1;
  m_head      = false;
  m_tail      = false;
  m_req       = NULL;
  m_state     = INIT;
  m_rdy_cycle = ULLONG_MAX;
  m_vc_batch  = false;
  m_sw_batch  = false;
  m_id        = 0;
}


/////////////////////////////////////////////////////////////////////////////////////////


credit_c::credit_c()
{
}

credit_c::~credit_c()
{
}


/////////////////////////////////////////////////////////////////////////////////////////


// constructor
router_c::router_c(macsim_c* simBase, int type, int id)
  : m_simBase(simBase), m_type(type), m_id(id)
{
  // configurations
  m_num_vc              = *KNOB(KNOB_NUM_VC); 
  m_num_port            = *KNOB(KNOB_NUM_PORT);
  m_link_latency        = *KNOB(KNOB_LINK_LATENCY);
  m_arbitration_policy  = *KNOB(KNOB_ARBITRATION_POLICY);
  m_link_width          = *KNOB(KNOB_LINK_WIDTH);
  m_enable_vc_partition = *KNOB(KNOB_ENABLE_NOC_VC_PARTITION);
  m_num_vc_cpu          = *KNOB(KNOB_CPU_VC_PARTITION);
  assert(m_num_vc_cpu < m_num_vc);
  assert(m_num_vc_cpu >= 2);
  assert(m_num_vc - m_num_vc_cpu >= 2);

  // link setting
  m_opposite_dir[LOCAL] = LOCAL;
  m_opposite_dir[LEFT]  = RIGHT;
  m_opposite_dir[RIGHT] = LEFT;
  m_opposite_dir[UP]    = DOWN;
  m_opposite_dir[DOWN]  = UP;

  m_link[LOCAL] = NULL;
  m_link[LEFT] = NULL;
  m_link[RIGHT] = NULL;
  m_link[UP] = NULL;
  m_link[DOWN] = NULL;

  // memory allocations
  m_req_buffer = new queue<mem_req_s*>;

  m_injection_buffer = new list<mem_req_s*>;
  m_injection_buffer_max_size = 32;
  m_injection_buffer_size     = 0;

  m_input_buffer = new list<flit_c*>*[m_num_port];
  m_output_buffer = new list<flit_c*>*[m_num_port];
  m_route = new int*[m_num_port];
  m_output_vc_avail = new bool*[m_num_port];
  m_output_vc_id = new int*[m_num_port];
  m_output_port_id = new int*[m_num_port];
  m_credit = new int*[m_num_port];

  for (int ii = 0; ii < m_num_port; ++ii) {
    m_input_buffer[ii] = new list<flit_c*>[m_num_vc];
    m_output_buffer[ii] = new list<flit_c*>[m_num_vc];
    m_route[ii] = new int[m_num_vc];
    fill_n(m_route[ii], m_num_vc, -1); 

    m_output_vc_avail[ii] = new bool[m_num_vc];
    fill_n(m_output_vc_avail[ii], m_num_vc, true);

    m_output_vc_id[ii] = new int[m_num_vc];
    fill_n(m_output_vc_id[ii], m_num_vc, -1);

    m_output_port_id[ii] = new int[m_num_vc];
    fill_n(m_output_port_id[ii], m_num_vc, -1);

    m_credit[ii] = new int[m_num_vc];
    fill_n(m_credit[ii], m_num_vc, 10);
  }

  m_buffer_max_size = 10;
  
  // switch
  m_sw_avail  = new Counter[m_num_port];
  fill_n(m_sw_avail, m_num_port, 0);

  m_link_avail = new Counter[m_num_port];
  fill_n(m_link_avail, m_num_port, 0);

  m_pending_credit = new list<credit_c*>;

  // clock
  m_cycle = 0;
}


// destructor
router_c::~router_c()
{
  delete m_req_buffer;
  delete m_injection_buffer;
  for (int ii = 0; ii < m_num_port; ++ii) {
    delete[] m_input_buffer[ii];
    delete[] m_output_buffer[ii];
    delete[] m_route[ii];
    delete[] m_output_vc_avail[ii];
    delete[] m_output_vc_id[ii];
    delete[] m_credit[ii];
  }
  delete[] m_input_buffer;
  delete[] m_output_buffer;
  delete[] m_route;
  delete[] m_output_vc_avail;
  delete[] m_output_vc_id;
  delete[] m_credit;

  delete[] m_sw_avail;
  delete[] m_link_avail;
  
  delete m_pending_credit;
}


// get router id
int router_c::get_id(void)
{
  return m_id;
}


void router_c::set_id(int id)
{
  m_id = id;
}


// set links to other routers
void router_c::set_link(int dir, router_c* link)
{
  m_link[dir] = link;

  int opposite_dir;
  if (dir == LEFT)       opposite_dir = RIGHT;
  else if (dir == RIGHT) opposite_dir = LEFT;
  else if (dir == UP)    opposite_dir = DOWN;
  else if (dir == DOWN)  opposite_dir = UP;
}


// Tick fuction
void router_c::run_a_cycle(void)
{
//  check_starvation();
  process_pending_credit();
  stage_lt();
//  stage_st();
  stage_sa();  
  stage_vca();
  stage_rc();
  local_packet_injection();

  // stats
//  check_channel();
  ++m_cycle;
}


// insert a packet from the network interface (NI)
bool router_c::inject_packet(mem_req_s* req)
{
  if (m_injection_buffer_size < m_injection_buffer_max_size) {
    m_injection_buffer->push_back(req);
    ++m_injection_buffer_size;

    DEBUG("cycle:%-10lld node:%d [IN] req_id:%d src:%d dst:%d\n",
        m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);
    return true;
  }

  return false;
}


// insert a packet from the injection buffer
void router_c::local_packet_injection(void)
{
  while (1) {
    if (m_injection_buffer->empty())
      break;

    mem_req_s* req = m_injection_buffer->front();
    int num_flit = 1;
    if (req->m_msg_type == NOC_FILL) 
      num_flit += req->m_size / m_link_width; 
    
    bool req_inserted = false;
    for (int ii = 0; ii < m_num_vc; ++ii) {
      // check buffer availability to insert a new request
      if (m_input_buffer[0][ii].size() + num_flit <= m_buffer_max_size) {
        // flit generation and insert into the buffer
        STAT_EVENT(TOTAL_PACKET_CPU + req->m_ptx);
        req->m_noc_cycle = m_cycle;

        ++g_total_packet;
        if (req->m_ptx) {
          ++g_total_gpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_GPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_GPU, g_total_gpu_packet);
        }
        else {
          ++g_total_cpu_packet;
          STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE_CPU);
          STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET_CPU, g_total_cpu_packet);
        }

        STAT_EVENT(NOC_AVG_ACTIVE_PACKET_BASE);
        STAT_EVENT_N(NOC_AVG_ACTIVE_PACKET, g_total_packet);

        for (int jj = 0; jj < num_flit; ++jj) {
          flit_c* new_flit = m_flit_pool->acquire_entry();
          new_flit->m_req = req;
          new_flit->m_src = req->m_msg_src;
          new_flit->m_dst = req->m_msg_dst;

          if (jj == 0) new_flit->m_head = true;
          else new_flit->m_head = false;

          if (jj == num_flit-1) new_flit->m_tail = true;
          else new_flit->m_tail = false;

          new_flit->m_state      = IB;
          new_flit->m_timestamp  = m_cycle;
          new_flit->m_rdy_cycle  = m_cycle;
          new_flit->m_id         = jj;

          // insert all flits to input_buffer[LOCAL][vc]
          m_input_buffer[0][ii].push_back(new_flit);
        }

        m_injection_buffer->pop_front();
        --m_injection_buffer_size;
        req_inserted = true;

        DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d\n",
            m_cycle, m_id, req->m_id, req->m_msg_src, req->m_msg_dst);

        break;
      }
    }

    // Nothing was scheduled. Stop inserting!
    if (!req_inserted)
      break;
  }
}


// RC stage
void router_c::stage_rc(void)
{
  // RC (Route Calculation) stage
  for (int port = 0; port < m_num_port; ++port) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (m_route[port][vc] != -1 || m_input_buffer[port][vc].empty())
        continue;

      flit_c* flit = m_input_buffer[port][vc].front();
      if (flit->m_head == true && flit->m_state == IB && flit->m_rdy_cycle <= m_cycle) {
        // ------------------------------
        // shortest distance routing
        // ------------------------------
        if (flit->m_dst == m_id) {
          m_route[port][vc] = LOCAL;

        }
        else {
          // ------------------------------
          // Ring
          // ------------------------------
          int left;
          int right;
          if (m_id > flit->m_dst) {
            left = m_id - flit->m_dst;
            right = flit->m_dst - m_id + m_total_router;
          }
          else {
            left = m_id - flit->m_dst + m_total_router;
            right = flit->m_dst - m_id;
          }

          if (left < right) m_route[port][vc] = LEFT;
          else m_route[port][vc] = RIGHT;
        }

        flit->m_state = RC;
        DEBUG("cycle:%-10lld node:%d [RC] req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d rc:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, port, vc, m_route[port][vc]);
      }
    }
  }
}


// For virtual channel partitioning
// VC 0~N1-1 : CPU VCs
// VC N1~N2-1: GPU VCs
void router_c::stage_vca(void)
{
  // VCA (Virtual Channel Allocation) stage
  // for each output port
  for (int oport = 0; oport < m_num_port; ++oport) {
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      // if there is available vc in the output port
      if (m_output_vc_avail[oport][ovc]) { 
        int iport, ivc;
        stage_vca_pick_winner(oport, ovc, iport, ivc);
        if (iport != -1) {
          flit_c* flit = m_input_buffer[iport][ivc].front();
          flit->m_state = VCA;
          
          m_output_port_id[iport][ivc]  = oport;
          m_output_vc_id[iport][ivc]    = ovc;
          m_output_vc_avail[oport][ovc] = false;

          DEBUG("cycle:%-10lld node:%d [VA] req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d "
              "op:%d oc:%d ptx:%d\n",
              m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
              flit->m_req->m_msg_dst, iport, ivc, m_route[iport][ivc], ovc, flit->m_req->m_ptx);
        }
      }
    }
  }
}


void router_c::stage_vca_pick_winner(int oport, int ovc, int& iport, int& ivc)
{
  bool type_allowed[2];
  type_allowed[0] = true;
  type_allowed[1] = true;

  if (m_enable_vc_partition) {
    // CPU partition
    if (ovc < m_num_vc_cpu) {
      type_allowed[1] = false;
    }
    else {
      type_allowed[0] = false;
    }
  }

  // Oldest-first arbitration
  if (m_arbitration_policy == OLDEST_FIRST) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    iport = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == oport)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_input_buffer[ii][jj].empty() || m_route[ii][jj] != oport)
          continue;

        flit_c* flit = m_input_buffer[ii][jj].front();

        // header && RC stage && oldest
        if (flit->m_head == true && flit->m_state == RC && type_allowed[flit->m_req->m_ptx] && 
            flit->m_timestamp < oldest_timestamp) {
          // ------------------------------
          // bubble routing (m_type: processors)
          // ------------------------------
          if (ii == LOCAL && get_ovc_occupancy(oport, flit->m_req->m_type) < 2) { 
            continue;
          }

          if (ii == LOCAL && m_type < 2 && *m_stop_injection)
            continue;

          oldest_timestamp = flit->m_timestamp;
          iport = ii;
          ivc   = jj;
        }
      }
    }
  }
  else
    assert(0);
}


int router_c::get_ovc_occupancy(int port, bool type)
{
  int search_begin = 0;
  int search_end = m_num_vc;

  if (m_enable_vc_partition) {
    if (type == false) {
      search_end = m_num_vc_cpu;
    }
    else {
      search_begin = m_num_vc_cpu;
    }
  }

  int count = 0;

  for (int vc = search_begin; vc < search_end; ++vc) 
    if (m_output_vc_avail[port][vc])
      ++count;

  return count;
}


// SA (Switch Allocation) stage
void router_c::stage_sa(void)
{
  for (int op = 0; op < m_num_port; ++op) {
    if (m_sw_avail[op] <= m_cycle) {
      int ip, ivc;
      stage_sa_pick_winner(op, ip, ivc, 0); 

      if (ip != -1) {
        m_sw_avail[op]   = m_cycle + 1;

        flit_c* flit = m_input_buffer[ip][ivc].front();
        flit->m_state = ST;

        // insert a flit to the output buffer 
        int ovc = m_output_vc_id[ip][ivc];
        m_output_buffer[op][ovc].push_back(flit);

        // pop a flit from the input buffer
        m_input_buffer[ip][ivc].pop_front();

        // all flits traversed, so need to free a input vc
        if (flit->m_tail) {
          m_route[ip][ivc]          = -1;
          m_output_vc_id[ip][ivc]   = -1;
          m_output_port_id[ip][ivc] = -1;
        }

        // send a credit back to previous router
        if (ip != LOCAL) {
          credit_c* credit = m_credit_pool->acquire_entry();
          credit->m_port = m_opposite_dir[ip];
          credit->m_vc = ivc;
          credit->m_rdy_cycle = m_cycle + 1;
          m_link[ip]->insert_credit(credit);
        }


        DEBUG("cycle:%-10lld node:%d [ST] req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d "
            "route:%d port:%d\n",
            m_cycle, m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, 
            flit->m_req->m_msg_dst, ip, ivc, m_route[ip][ivc], ovc, m_route[ip][ivc], m_output_port_id[ip][ivc]);
      }
    }
  }
}


void router_c::stage_sa_pick_winner(int op, int& ip, int& ivc, int sw_id)
{
  if (m_arbitration_policy == OLDEST_FIRST) {
    Counter oldest_timestamp = ULLONG_MAX;
    ip = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      if (ii == op)
        continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        // find a flit that acquires a vc in current output port
        if (m_route[ii][jj] == op && 
            m_output_port_id[ii][jj] == op && 
            !m_input_buffer[ii][jj].empty()) {
          flit_c* flit = m_input_buffer[ii][jj].front();

          // VCA & Header or IB & Body/Tail
          if (((flit->m_head && flit->m_state == VCA) ||
               (!flit->m_head && flit->m_state == IB)) && 
              flit->m_timestamp < oldest_timestamp) {
            oldest_timestamp = flit->m_timestamp;
            ip = ii;
            ivc = jj;
          }
        }
      }
    }
  }
}


// ST-stage
void router_c::stage_st(void)
{
}

void router_c::stage_lt(void)
{
  for (int port = 0; port < m_num_port; ++port) {
    if (m_link_avail[port] > m_cycle) 
      break;
    Counter oldest_cycle = ULLONG_MAX;
    flit_c* f = NULL;
    int vc = -1;
    for (int ii = 0; ii < m_num_vc; ++ii) {
      if (m_output_buffer[port][ii].empty() || m_credit[port][ii] == 0)
        continue;

      flit_c* flit = m_output_buffer[port][ii].front();
      assert(flit->m_state == ST);
      
      if (flit->m_timestamp < oldest_cycle) {
        oldest_cycle = flit->m_timestamp;
        vc           = ii;
        f            = flit;
      }
    }

    if (vc != -1) {
      // insert to next router
      if (port != LOCAL) {
        DEBUG("cycle:%-10lld node:%d [LT] req_id:%d flit_id:%d src:%d dst:%d port:%d vc:%d\n",
            m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src, 
            f->m_req->m_msg_dst, port, vc);

        m_link[port]->insert_packet(f, m_opposite_dir[port], vc);
        f->m_rdy_cycle     = m_cycle + m_link_latency;
        m_link_avail[port] = m_cycle + m_link_latency; // link busy
      }

      // delete flit in the buffer
      m_output_buffer[port][vc].pop_front();

      if (port != LOCAL)
        --m_credit[port][vc];

      // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
      if (f->m_tail) {
        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER, m_cycle - f->m_timestamp);

        STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE_CPU + m_type);
        STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER_CPU + m_type, m_cycle - f->m_timestamp);

        m_output_vc_avail[port][vc] = true;


        if (port == LOCAL) {
          --g_total_packet;
          if (f->m_req->m_ptx) {
            --g_total_gpu_packet;
          }
          else {
            --g_total_cpu_packet;
          }
          m_req_buffer->push(f->m_req);
          DEBUG("cycle:%-10lld node:%d [TT] req_id:%d flit_id:%d src:%d dst:%d vc:%d\n",
              m_cycle, m_id, f->m_req->m_id, f->m_id, f->m_req->m_msg_src, 
              f->m_req->m_msg_dst, vc);

          STAT_EVENT(NOC_AVG_LATENCY_BASE);
          STAT_EVENT_N(NOC_AVG_LATENCY, m_cycle - f->m_req->m_noc_cycle);

          STAT_EVENT(NOC_AVG_LATENCY_BASE_CPU + f->m_req->m_ptx);
          STAT_EVENT_N(NOC_AVG_LATENCY_CPU + f->m_req->m_ptx, m_cycle - f->m_req->m_noc_cycle);
        }
      }

      if (port == LOCAL) {
        f->init();
        m_flit_pool->release_entry(f);
      }
    }
  }
}


void router_c::check_channel(void)
{
#if 0
  for (int op = 0; op < m_num_port; ++op) {
    for (int sw = 0; sw < m_num_switch; ++sw) {
      if (op != LOCAL && m_sw_avail[op][sw]) {
        STAT_EVENT(NOC_IDLE_CHANNEL);
        STAT_EVENT(NOC_IDLE_CHANNEL_CPU + m_type); 
      }
    }
  } 
#endif
}


void router_c::insert_packet(flit_c* flit, int port, int vc)
{
  // IB (Input Buffering) stage
  if (flit->m_head)
    DEBUG("cycle:%-10lld node:%d [IB] req_id:%d src:%d dst:%d ip:%d vc:%d\n",
        m_cycle, m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst, port, vc);

  m_input_buffer[port][vc].push_back(flit);
  flit->m_state = IB;
  flit->m_timestamp = m_cycle;
}


void router_c::insert_credit(credit_c* credit)
{
  m_pending_credit->push_back(credit);
}


void router_c::process_pending_credit(void)
{
  if (m_pending_credit->empty())
    return ;

  auto I = m_pending_credit->begin();
  auto E = m_pending_credit->end();
  do {
    credit_c* credit = (*I++);
    if (credit->m_rdy_cycle <= m_cycle) {
      ++m_credit[credit->m_port][credit->m_vc];
      m_pending_credit->remove(credit);
      m_credit_pool->release_entry(credit);
    }
  } while (I != E);
}


mem_req_s* router_c::receive_req(void)
{
  if (m_req_buffer->empty())
    return NULL;
  else
    return m_req_buffer->front();
}


void router_c::pop_req(void)
{
  m_req_buffer->pop();
}


void router_c::init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool, pool_c<credit_c>* credit_pool, bool* stop_injection)
{
  m_total_router   = total_router;
  m_total_packet   = total_packet;
  m_flit_pool      = flit_pool;
  m_credit_pool    = credit_pool;
  m_stop_injection = stop_injection;
}


void router_c::print(ofstream& out)
{
}

void router_c::print_link_info(void)
{
  cout << "Ring configuration\n";
  cout << m_id << " <-> ";
  router_c* current = m_link[RIGHT];
  do {
    cout << current->get_id() << " <-> ";
    current = current->m_link[RIGHT];
  } while (current != this);
  cout << "\n";
}


void router_c::check_starvation(void)
{
  if (m_type >= 2 && *m_stop_injection == false) {
    for (int vc = 0; vc < m_num_vc; ++vc) {
      if (!m_input_buffer[0][vc].empty()) {
        flit_c* flit = m_input_buffer[0][vc].front();
        if (m_cycle - flit->m_timestamp >= 1000) {
          *m_stop_injection = true;
          return ;
        }
      }
    }
  }
#if 0
  for (int ip = 0; ip < m_num_port; ++ip) {
    for (int ivc = 0; ivc < m_num_vc; ++ivc) {
      if (m_ivc_buffer[ip][ivc].empty())
        continue;

      flit_c* flit = m_ivc_buffer[ip][ivc].front();
      if (CYCLE - flit->m_rc_changed >= 1000 && flit->m_state == RC) {
        int count = 0;
        for (int ii = 0; ii < m_num_vc; ++ii) {
          if (m_ovc_avail[m_opposite_dir[m_ivc_rc[ip][ivc]]][ii])
            count++;
        }

        if (count > 0) {
          m_ivc_rc[ip][ivc] = m_opposite_dir[m_ivc_rc[ip][ivc]];
          flit->m_dir = m_ivc_rc[ip][ivc];
          flit->m_rc_changed = CYCLE;
          DEBUG("node:%d req_id:%d rc changed %d count:%d\n", 
              m_id, flit->m_req->m_id, m_ivc_rc[ip][ivc], count);
        }
      }
    }
  }
#endif
}

/////////////////////////////////////////////////////////////////////////////////////////


router_wrapper_c::router_wrapper_c(macsim_c* simBase)
  : m_simBase(simBase)
{
  m_total_packet = 0;
  m_num_router = 0;

  m_flit_pool  = new pool_c<flit_c>(100, "flit");
  m_credit_pool = new pool_c<credit_c>(100, "credit");

  m_cycle = 0;
}

router_wrapper_c::~router_wrapper_c()
{
  delete m_flit_pool;
  delete m_credit_pool;
}


// run one cycle for all routers
void router_wrapper_c::run_a_cycle(void)
{
  // randomized tick function
  m_stop_injection = false;
  for (int ii = 0; ii < m_num_router; ++ii) {
    if (m_stop_injection)
      break;
    m_router[ii]->check_starvation();
  }


  int index = m_cycle % m_num_router;
  for (int ii = index; ii < index + m_num_router; ++ii) {
    m_router[ii % m_num_router]->run_a_cycle();
  }

  ++m_cycle;
}

// create a router
router_c* router_wrapper_c::create_router(int type)
{
  report("router:" << m_num_router << " type:" << type << " created");

  router_c* new_router = new router_c(m_simBase, type, m_num_router++);
  m_router.push_back(new_router);

  return new_router;
}

void router_wrapper_c::init(void)
{
  // topology - setting router links
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &g_total_packet, m_flit_pool, m_credit_pool, &m_stop_injection);
  }


  string mapping;
  stringstream sstr;
  if (*KNOB(KNOB_ROUTER_PLACEMENT) == GPU_FRIENDLY) {
    for (int ii = 0; ii < m_num_router; ++ii) {
      sstr << ii << ",";
    }
    sstr >> mapping;
    mapping = mapping.substr(0, mapping.length()-1);
  }
  else if (*KNOB(KNOB_ROUTER_PLACEMENT) == CPU_FRIENDLY) {
    int num_large_core = *KNOB(KNOB_NUM_SIM_LARGE_CORES);
    int num_small_core = *KNOB(KNOB_NUM_SIM_SMALL_CORES);
    for (int ii = num_large_core; ii < num_large_core + num_small_core; ++ii)
      sstr << ii << ",";
    for (int ii = 0; ii < num_large_core; ++ii)
      sstr << ii << ",";
    for (int ii = num_large_core+num_small_core; ii < m_num_router; ++ii)
      sstr << ii << ",";
    sstr >> mapping;
    mapping = mapping.substr(0, mapping.length()-1);
  }
  else if (*KNOB(KNOB_ROUTER_PLACEMENT) == MIXED) {
    mapping = "0,1,2,3,4,5,6,7,12,8,9,10,11,13";
  }
  else if (*KNOB(KNOB_ROUTER_PLACEMENT) == INTERLEAVED) {
    mapping = "0,4,1,5,2,6,3,7,8,9,10,11,12,13"; 
  } 
  int search_pos = 0;
  int pos;
  vector<int> map_func;
  while (1) {
    pos = mapping.find(',', search_pos);
    if (pos == string::npos) {
      string sub = mapping.substr(search_pos);
      map_func.push_back(atoi(sub.c_str()));
      break;
    }

    string sub = mapping.substr(search_pos, pos - search_pos);
    map_func.push_back(atoi(sub.c_str()));
    search_pos = pos + 1;
  }

  assert(m_num_router == map_func.size());

  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[map_func[ii]]->set_link(LEFT,  m_router[map_func[(ii-1+m_num_router)%m_num_router]]);
    m_router[map_func[ii]]->set_link(RIGHT, m_router[map_func[(ii+1)%m_num_router]]);
    m_router[map_func[ii]]->set_id(ii);
  }

  m_router[0]->print_link_info();
}

void router_wrapper_c::print(void)
{
  ofstream out("router.out");
  out << "total_packet:" << m_total_packet << "\n";
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->print(out);
  }
}


/////////////////////////////////////////////////////////////////////////////////////////
