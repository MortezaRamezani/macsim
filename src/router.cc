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
  m_dir       = -1;
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


// constructor
router_c::router_c(macsim_c* simBase, int type, int id)
  : m_simBase(simBase), m_type(type), m_id(id)
{
  m_num_vc             = *KNOB(KNOB_NUM_VC); 
  m_num_port           = *KNOB(KNOB_NUM_PORT);
  m_link_latency       = *KNOB(KNOB_LINK_LATENCY) + 1;
  m_arbitration_policy = *KNOB(KNOB_ARBITRATION_POLICY);
  m_link_width         = *KNOB(KNOB_LINK_WIDTH);
  m_num_switch         = *KNOB(KNOB_NUM_SWITCH);
  if (*KNOB(KNOB_ENABLE_HETEROGENEOUS_LINK)) {
    if (type == CPU_ROUTER)
      m_num_switch = *KNOB(KNOB_NUM_SWITCH_CPU);
    else if (type == GPU_ROUTER)
      m_num_switch = *KNOB(KNOB_NUM_SWITCH_GPU);
    else 
      m_num_switch = *KNOB(KNOB_NUM_SWITCH_MEM);
  }


  if (*KNOB(KNOB_ENABLE_HETEROGENEOUS_LINK_WIDTH)) {
    if (type == CPU_ROUTER)
      m_sw_iter = *KNOB(KNOB_NUM_SWITCH_ITER_CPU);
    else if (type == GPU_ROUTER)
      m_sw_iter = *KNOB(KNOB_NUM_SWITCH_ITER_GPU);
    else 
      m_sw_iter = *KNOB(KNOB_NUM_SWITCH_ITER_MEM);
  }
  else
    m_sw_iter = *KNOB(KNOB_NUM_SWITCH_ITER);

  // vc partitioning
  m_vc_partition = *KNOB(KNOB_ENABLE_NOC_VC_PARTITION);
  m_cpu_partition = *KNOB(KNOB_CPU_VC_PARTITION);
  m_gpu_partition = *KNOB(KNOB_GPU_VC_PARTITION);

  // channel partitioning
  m_channel_partition = *KNOB(KNOB_ENABLE_CHANNEL_PARTITION);
  m_cpu_ch_partition = *KNOB(KNOB_NUM_CHANNEL_CPU);
  m_gpu_ch_partition = *KNOB(KNOB_NUM_CHANNEL_GPU);

  if (m_channel_partition) {
    assert(m_num_switch == (m_cpu_ch_partition + m_gpu_ch_partition));
    assert(*KNOB(KNOB_ENABLE_HETEROGENEOUS_LINK) == false);
  }

  if (m_vc_partition)
    assert(m_cpu_partition + m_gpu_partition == m_num_vc);
  assert(m_num_vc > 1);
  assert(m_cpu_partition > 1);
  assert(m_gpu_partition > 1);

  m_ivc_avail  = new bool*[m_num_port];
  m_ovc_avail  = new bool*[m_num_port];
  m_ivc_rc     = new int*[m_num_port];
  m_ivc_vc     = new int*[m_num_port];
  m_ivc_sw     = new int*[m_num_port];
  m_ivc_buffer = new list<flit_c*>*[m_num_port];
  m_sw_avail  = new bool*[m_num_port];
  m_sw_ip  = new int*[m_num_port];
  m_sw_vc  = new int*[m_num_port];

  for (int ii = 0; ii < m_num_port; ++ii) {
    m_sw_avail[ii] = new bool[m_num_switch];
    fill_n(m_sw_avail[ii], m_num_switch, true);
    
    m_sw_ip[ii] = new int[m_num_switch];
    fill_n(m_sw_ip[ii], m_num_switch, -1);
    
    m_sw_vc[ii] = new int[m_num_switch];
    fill_n(m_sw_vc[ii], m_num_switch, -1);
  }

//  fill_n(m_sw_ip, m_num_port, -1);
//  fill_n(m_sw_vc, m_num_port, -1);
  
  m_last_vc_winner = new int[m_num_port];
  m_last_sw_winner = new int[m_num_port];
  fill_n(m_last_vc_winner, m_num_port, 0);
  fill_n(m_last_sw_winner, m_num_port, 0);


  for (int ii = 0; ii < m_num_port; ++ii) {
    m_ivc_avail[ii] = new bool[m_num_vc];
//    m_ovc_avail[ii] = new bool[m_num_vc];
    fill_n(m_ivc_avail[ii], m_num_vc, true);
//    fill_n(m_ovc_avail[ii], m_num_vc, true);

    m_ivc_rc[ii] = new int[m_num_vc];
    fill_n(m_ivc_rc[ii], m_num_vc, -1);
    
    m_ivc_vc[ii] = new int[m_num_vc];
    fill_n(m_ivc_vc[ii], m_num_vc, -1);

    m_ivc_sw[ii] = new int[m_num_vc];
    fill_n(m_ivc_sw[ii], m_num_vc, -1);

    m_ivc_buffer[ii] = new list<flit_c*>[m_num_vc];
  }

  m_ovc_avail[0] = new bool[m_num_vc];
  fill_n(m_ovc_avail[0], m_num_vc, true);

  m_req_buffer = new queue<mem_req_s*>;

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
}


// destructor
router_c::~router_c()
{
  for (int ii = 0; ii < m_num_port; ++ii) {
    delete[] m_ivc_avail[ii];
    delete[] m_ivc_rc[ii];
    delete[] m_ivc_buffer[ii];
  }

  delete[] m_ivc_avail;
  delete[] m_ivc_rc;
  delete[] m_ivc_buffer;

  delete m_req_buffer;
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
  if (dir == LEFT) opposite_dir = RIGHT;
  else if (dir == RIGHT) opposite_dir = LEFT;
  else if (dir == UP) opposite_dir = DOWN;
  else if (dir == DOWN) opposite_dir = UP;

  m_ovc_avail[dir] = link->m_ivc_avail[opposite_dir]; 
}


// insert a packet from the network interface (NI)
bool router_c::send_packet(mem_req_s* req)
{
#if 0 // GLOBAL INJECTION CONTROL - not practical
  // to prevent deadlock, total packet < total router * num_vc * (num_port-1)
  int max_packet;
  int total_packet;

  if (m_vc_partition) {
    if (req->m_ptx) {
      max_packet   = m_total_router * m_gpu_partition * (m_num_port - 1) - 1;
      total_packet = g_total_gpu_packet;
    }
    else {
      max_packet   = m_total_router * m_cpu_partition * (m_num_port - 1) - 1;
      total_packet = g_total_cpu_packet;
    }
  }
  else {
    max_packet   = m_total_router * m_num_vc * (m_num_port - 1) - 1;
    total_packet = g_total_packet;
  }
  if (total_packet >= max_packet)
    return false;
#endif


  // check input buffer availability 
  int begin = 0;
  int end   = m_num_vc;
  if (m_vc_partition && (m_type == L3_ROUTER || m_type == MC_ROUTER)) {
    if (req->m_ptx) {
      begin = m_cpu_partition;
    }
    else {
      end = m_cpu_partition;
    }
  }



  // available local vc check
  int index = -1;
  for (int ii = begin; ii < end; ++ii) {
    if (m_ivc_avail[LOCAL][ii] == true) {
      index = ii;
      m_ivc_avail[LOCAL][ii] = false;
      break;
    } 
  }

  if (index == -1)
    return false;


  req->m_noc_cycle = CYCLE;
  STAT_EVENT(TOTAL_PACKET_CPU + req->m_ptx);

  
//  (*m_total_packet)++;
  ++g_total_packet;
//  cout << "total_packet:" << g_total_packet << " " << m_total_router * (m_num_port-1) * m_num_vc << "\n";
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
  

  // flit generation
  int num_flit = 1;
  if (req->m_msg_type == NOC_FILL) {
    num_flit += req->m_size / m_link_width; 
  } 

  for (int ii = 0; ii < num_flit; ++ii) {
    flit_c* new_flit = m_flit_pool->acquire_entry();
    new_flit->m_req = req;
    new_flit->m_src = req->m_msg_src;
    new_flit->m_dst = req->m_msg_dst;

    if (ii == 0) new_flit->m_head = true;
    else new_flit->m_head = false;
    
    if (ii == num_flit-1) new_flit->m_tail = true;
    else new_flit->m_tail = false;

    new_flit->m_state     = IB;
    new_flit->m_timestamp = CYCLE;
    new_flit->m_rdy_cycle = CYCLE;
    new_flit->m_rc_changed = CYCLE;
    new_flit->m_id = ii;

    // insert all flits to m_buffer[LOCAL][vc]
    m_ivc_buffer[LOCAL][index].push_back(new_flit);
  }

  DEBUG("node:%d IB req_id:%d src:%d dst:%d ip:%d vc:%d\n",
      m_id, req->m_id, req->m_msg_src, req->m_msg_dst, LOCAL, index);

  return true;
}


// Tick fuction
void router_c::run_a_cycle(void)
{
  check_starvation();
  link_traversal();
  sw_traversal();
  sw_allocation();
  vc_allocation();
  route_calculation();
  check_channel();
}

void router_c::check_channel(void)
{
  for (int op = 0; op < m_num_port; ++op) {
    for (int sw = 0; sw < m_num_switch; ++sw) {
      if (op != LOCAL && m_sw_avail[op][sw]) {
        STAT_EVENT(NOC_IDLE_CHANNEL);
        STAT_EVENT(NOC_IDLE_CHANNEL_CPU + m_type); 
      }
    }
  } 
}


// RC stage
void router_c::route_calculation(void)
{
  // RC (Route Calculation) stage
  for (int ii = 0; ii < m_num_port; ++ii) {
    for (int jj = 0; jj < m_num_vc; ++jj) {
      for (auto I = m_ivc_buffer[ii][jj].begin(), E = m_ivc_buffer[ii][jj].end(); I != E; ++I) { 
        if ((*I)->m_head == true && (*I)->m_state == IB && (*I)->m_rdy_cycle <= CYCLE) {
          if (m_id == (*I)->m_dst) {
            m_ivc_rc[ii][jj] = LOCAL;
          }
          // ------------------------------
          // to prevent ping-pong routing
          // ------------------------------
          else if ((*I)->m_dir != -1) {
            m_ivc_rc[ii][jj] = (*I)->m_dir;
          }
          // ------------------------------
          // shortest distance routing
          // ------------------------------
          else {
            // ------------------------------
            // Ring
            // ------------------------------
            int left;
            int right;
            if (m_id > (*I)->m_dst) {
              left = m_id - (*I)->m_dst;
              right = (*I)->m_dst - m_id + m_total_router;
            }
            else {
              left = m_id - (*I)->m_dst + m_total_router;
              right = (*I)->m_dst - m_id;
            }

            int delta;
            if (left < right) { 
              m_ivc_rc[ii][jj] = LEFT;
              delta = right - left;
            }
            else { 
              m_ivc_rc[ii][jj] = RIGHT;
              delta = left - right;
            }


#if 0
            if (*KNOB(KNOB_ENABLE_ADAPTIVE_RING_ROUTING) &&
                (*KNOB(KNOB_ARR_DELTA) == false || (delta < m_total_router/2))) %{
              int occupancy[5];
              occupancy[LEFT]  = get_ovc_occupancy(LEFT);
              occupancy[RIGHT] = get_ovc_occupancy(RIGHT);


              // ORIG >= threshold && OPP < threshold : change direction
              if (*KNOB(KNOB_ARR_ADV) &&
                  occupancy[m_ivc_rc[ii][jj]] >= *KNOB(KNOB_ARR_THRESHOLD) &&
                  occupancy[m_opposite_dir[m_ivc_rc[ii][jj]]] < *KNOB(KNOB_ARR_THRESHOLD) &&
                  delta < m_total_router/2) {
                m_ivc_rc[ii][jj] = m_opposite_dir[m_ivc_rc[ii][jj]];
              }
              // ORIG == 100% && OPP != 100%
              else if (*KNOB(KNOB_ARR_100) &&
                  occupancy[m_ivc_rc[ii][jj]] == 100 &&
                  occupancy[m_opposite_dir[m_ivc_rc[ii][jj]]] < 100) {
                m_ivc_rc[ii][jj] = m_opposite_dir[m_ivc_rc[ii][jj]];
              }
            }
#endif


            (*I)->m_dir = m_ivc_rc[ii][jj];
          }
          (*I)->m_state = RC;
          DEBUG("node:%d RC req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d rc:%d\n",
              m_id, (*I)->m_req->m_id, (*I)->m_id, (*I)->m_req->m_msg_src, (*I)->m_req->m_msg_dst,
              ii, jj, m_ivc_rc[ii][jj]);
        }
      }
    }
  }
}


// Get OVC occupancy
int router_c::get_ovc_occupancy(int port)
{
  int count = 0;
  for (int ii = 0; ii < m_num_vc; ++ii) {
    if (m_ovc_avail[port][ii]) 
      count++;
  }

  return count;
}


int router_c::get_ovc_occupancy(int port, bool type)
{
  if (m_vc_partition == false)
    return get_ovc_occupancy(port);

  int start;
  int end;

  if (type == false) { // CPU
    start = 0;
    end   = m_cpu_partition;
  }
  else{
    start = m_cpu_partition;
    end   = m_num_vc;
  }

  int count = 0;
  for (int ii = start; ii < end; ++ii) {
    if (m_ovc_avail[port][ii]) 
      count++;
  }

  return count;
}


void router_c::vc_allocation(void)
{
  // VCA (Virtual Channel Allocation) stage
  // for each output port
  for (int op = 0; op < m_num_port; ++op) {
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      // if there is available vc in the output port
      if (m_ovc_avail[op][ovc]) {
        int ip, ivc;
        pick_vc_winner(op, ovc, ip, ivc);
        if (ip != -1) {
          flit_c* flit = m_ivc_buffer[ip][ivc].front();
          flit->m_state = VCA;
          
          m_ivc_vc[ip][ivc] = ovc;
          m_ovc_avail[op][ovc] = false;

          DEBUG("node:%d VCA req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
              m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
              ip, ivc, m_ivc_rc[ip][ivc], ovc);
        }
      }
    }
  }
}


// 0 ~ m_cpu_partition-1 : CPU VC
// m_cpu_partition ~ m_num_Vc-1 : GPU VC
void router_c::pick_vc_winner(int op, int ovc, int& ip, int& ivc)
{
  // ------------------------------
  // vc partitioning
  // ------------------------------
  bool enable_vc[2];
  enable_vc[0] = false;
  enable_vc[1] = false;

  if (!m_vc_partition) {
    enable_vc[0] = true;
    enable_vc[1] = true;
  }
  else {
    if (ovc < m_cpu_partition) {
      enable_vc[0] = true;
    }
    else {
      enable_vc[1] = true;
    }
  }


  // Oldest-first arbitration
  if (m_arbitration_policy == OLDEST_FIRST) {
    // search all input ports for the winner
    Counter oldest_timestamp = ULLONG_MAX;
    ip = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      //    if (ii == op)
      //      continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_ivc_buffer[ii][jj].empty() || m_ivc_rc[ii][jj] != op)
          continue;

        flit_c* flit = m_ivc_buffer[ii][jj].front();

        // header && RC stage && oldest
        if (flit->m_head == true && flit->m_state == RC && flit->m_timestamp < oldest_timestamp) {
          if (enable_vc[flit->m_req->m_ptx]) {
            // ------------------------------
            // bubble routing
            // ------------------------------
            if (ii == LOCAL && get_ovc_occupancy(op, flit->m_req->m_type) < 2) { 
              continue;
            }

            oldest_timestamp = flit->m_timestamp;
            ip  = ii;
            ivc = jj;
          }
        }
      }
    }
  }
  // Round-robin arbitration
  else if (m_arbitration_policy == ROUND_ROBIN) {
    ip = -1;
    for (int ii = m_last_vc_winner[op]; ii < m_last_vc_winner[op] + m_num_port * m_num_vc; ++ii) {
      int input_port = (ii / m_num_vc) % m_num_port;
      int input_vc = ii % m_num_vc;
      if (m_ivc_buffer[input_port][input_vc].empty() || m_ivc_rc[input_port][input_vc] != op)
        continue;

      flit_c* flit = m_ivc_buffer[input_port][input_vc].front();

      // header && RC stage && oldest
      if (flit->m_head == true && flit->m_state == RC) {
        if (enable_vc[flit->m_req->m_ptx]) {
          // ------------------------------
          // bubble routing
          // ------------------------------
          if (input_port == LOCAL && get_ovc_occupancy(op, flit->m_req->m_type) < 2) { 
            continue;
          }
          ip  = input_port;
          ivc = input_vc;
          m_last_vc_winner[op] = (ii + 1) % (m_num_port * m_num_vc);
          return ;
        }
      }
    }
  }
  else if (m_arbitration_policy == CPU_FIRST || m_arbitration_policy == GPU_FIRST) {
    ip = -1;
    
    int type_ip[2]; fill_n(type_ip, 2, -1);
    int type_ivc[2]; fill_n(type_ivc, 2, -1);
    Counter type_cycle[2]; fill_n(type_cycle, 2, ULLONG_MAX);;

    int oldest_ip[2]; fill_n(oldest_ip, 2, -1);
    int oldest_ivc[2]; fill_n(oldest_ivc, 2, -1);
    Counter oldest_cycle[2]; fill_n(oldest_cycle, 2, ULLONG_MAX);

    // count # batch
    int num_batch = 0;
    for (int ii = m_last_vc_winner[op]; ii < m_last_vc_winner[op] + m_num_port * m_num_vc; ++ii) {
      int input_port = (ii / m_num_vc) % m_num_port;
      int input_vc   = ii % m_num_vc;
      if (m_ivc_buffer[input_port][input_vc].empty() || m_ivc_rc[input_port][input_vc] != op)
        continue;

      flit_c* flit = m_ivc_buffer[input_port][input_vc].front();

      // header && RC stage && oldest
      if (flit->m_head == true && flit->m_state == RC) {
        if (enable_vc[flit->m_req->m_ptx]) {
          // ------------------------------
          // bubble routing
          // ------------------------------
          if (input_port == LOCAL && get_ovc_occupancy(op, flit->m_req->m_type) < 2) { 
            continue;
          }
          // get the first entry of each type
          if (type_ip[flit->m_req->m_ptx] == -1) {
            type_cycle[flit->m_req->m_ptx] = flit->m_timestamp;
            type_ip[flit->m_req->m_ptx]    = input_port;
            type_ivc[flit->m_req->m_ptx]   = input_vc;    
          }

          // get the oldest entry of each type
          if (flit->m_timestamp < oldest_cycle[flit->m_req->m_ptx]) {
            oldest_cycle[flit->m_req->m_ptx] = flit->m_timestamp;
            oldest_ip[flit->m_req->m_ptx]    = input_port; 
            oldest_ivc[flit->m_req->m_ptx]   = input_vc;
          }
        }
      }
    }

    
    int type = 0;
    if (m_arbitration_policy == GPU_FIRST) 
      type = 1;


    // prioritize the specified type
    if (type_ip[type] != -1) {
      // compare timestamp with oldest one
      // if > N, prevent starvation
      int time_delta;
      if (oldest_cycle[!type] == ULLONG_MAX)
        time_delta = 0;
      else
        time_delta = type_cycle[type] - oldest_cycle[!type];

      // prior type
      if (time_delta < 1000) {
        ip  = type_ip[type];
        ivc = type_ivc[type];
        m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
        return ;
      }
      // starvation
      else {
        ip  = oldest_ip[!type];
        ivc = oldest_ivc[!type];
        m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
        return ;
      }
    }

    // no request
    type = !type;
    if (type_ip[type] != -1) {
      ip  = type_ip[type];
      ivc = type_ivc[type];
      m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
      return ;
    }
  }
}


// SA (Switch Allocation) stage
void router_c::sw_allocation(void)
{
  // search all output port switch availability
  for (int op = 0; op < m_num_port; ++op) {
    for (int sw = 0; sw < m_num_switch; ++sw) {
      if (!m_sw_avail[op][sw]) 
        continue;

      int ip, ivc;
      pick_swa_winner(op, ip, ivc, sw); 

      if (ip != -1) {
        m_ivc_sw[ip][ivc] = sw;
        m_sw_avail[op][sw] = false;
        m_sw_ip[op][sw] = ip;
        m_sw_vc[op][sw] = ivc;

        flit_c* flit = m_ivc_buffer[ip][ivc].front();
        flit->m_state = SA;

        DEBUG("node:%d SA req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
            m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
            ip, ivc, m_ivc_rc[ip][ivc], m_ivc_vc[ip][ivc]);
      }
    }
  }
}


// SW Arbitration
void router_c::pick_swa_winner(int op, int& ip, int& ivc, int sw_id)
{
  bool type[2];
  type[0] = true;
  type[1] = true;
  if (m_channel_partition) {
    if (sw_id < m_cpu_ch_partition)
      type[1] = false;
    else
      type[0] = false;
  }
  if (m_arbitration_policy == OLDEST_FIRST) {
    Counter oldest_timestamp = ULLONG_MAX;
    ip = -1;
    for (int ii = 0; ii < m_num_port; ++ii) {
      //    if (ii == op)
      //      continue;

      for (int jj = 0; jj < m_num_vc; ++jj) {
        if (m_ivc_rc[ii][jj] == op && m_ivc_vc[ii][jj] != -1 && m_ivc_sw[ii][jj] == -1) {
          assert(!m_ivc_buffer[ii][jj].empty());

          flit_c* flit = m_ivc_buffer[ii][jj].front();
          if (type[flit->m_req->m_ptx] == false)
            continue ;

          // header && RC stage && oldest
          if (flit->m_head == true && flit->m_state == VCA && flit->m_timestamp < oldest_timestamp) {
            oldest_timestamp = flit->m_timestamp;
            ip = ii;
            ivc = jj;
          }
        }
      }
    }
  }
  else if (m_arbitration_policy == ROUND_ROBIN) {
    ip = -1;
    for (int ii = m_last_sw_winner[op]; ii < m_last_sw_winner[op] + m_num_port * m_num_vc; ++ii) {
      int input_port = (ii / m_num_vc) % m_num_port;
      int input_vc = ii % m_num_vc;
      if (m_ivc_buffer[input_port][input_vc].empty() || m_ivc_rc[input_port][input_vc] != op)
        continue;

      flit_c* flit = m_ivc_buffer[input_port][input_vc].front();

      // header && VCA stage
      if (flit->m_head == true && flit->m_state == VCA) {
        ip  = input_port;
        ivc = input_vc;
        m_last_sw_winner[op] = (ii + 1) % (m_num_port * m_num_vc);
        return ;
      }
    }
  }
  else if (m_arbitration_policy == CPU_FIRST || m_arbitration_policy == GPU_FIRST) {
    ip = -1;
    
    int type_ip[2]; fill_n(type_ip, 2, -1);
    int type_ivc[2]; fill_n(type_ivc, 2, -1);
    Counter type_cycle[2]; fill_n(type_cycle, 2, ULLONG_MAX);;

    int oldest_ip[2]; fill_n(oldest_ip, 2, -1);
    int oldest_ivc[2]; fill_n(oldest_ivc, 2, -1);
    Counter oldest_cycle[2]; fill_n(oldest_cycle, 2, ULLONG_MAX);

    // count # batch
    int num_batch = 0;
    for (int ii = m_last_vc_winner[op]; ii < m_last_vc_winner[op] + m_num_port * m_num_vc; ++ii) {
      int input_port = (ii / m_num_vc) % m_num_port;
      int input_vc   = ii % m_num_vc;
      if (m_ivc_buffer[input_port][input_vc].empty() || m_ivc_rc[input_port][input_vc] != op)
        continue;

      flit_c* flit = m_ivc_buffer[input_port][input_vc].front();

      // header && RC stage && oldest
      if (flit->m_head == true && flit->m_state == VCA) {
        // get the first entry of each type
        if (type_ip[flit->m_req->m_ptx] == -1) {
          type_cycle[flit->m_req->m_ptx] = flit->m_timestamp;
          type_ip[flit->m_req->m_ptx]    = input_port;
          type_ivc[flit->m_req->m_ptx]   = input_vc;    
        }

        // get the oldest entry of each type
        if (flit->m_timestamp < oldest_cycle[flit->m_req->m_ptx]) {
          oldest_cycle[flit->m_req->m_ptx] = flit->m_timestamp;
          oldest_ip[flit->m_req->m_ptx]    = input_port; 
          oldest_ivc[flit->m_req->m_ptx]   = input_vc;
        }
      }
    }


    int type = 0;
    if (m_arbitration_policy == GPU_FIRST) 
      type = 1;


    // prioritize the specified type
    if (type_ip[type] != -1) {
      // compare timestamp with oldest one
      // if > N, prevent starvation
      int time_delta;
      if (oldest_cycle[!type] == ULLONG_MAX)
        time_delta = 0;
      else
        time_delta = type_cycle[type] - oldest_cycle[!type];

      // prior type
      if (time_delta < 1000) {
        ip  = type_ip[type];
        ivc = type_ivc[type];
        m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
        return ;
      }
      // starvation
      else {
        ip  = oldest_ip[!type];
        ivc = oldest_ivc[!type];
        m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
        return ;
      }
    }

    // no request
    type = !type;
    if (type_ip[type] != -1) {
      ip  = type_ip[type];
      ivc = type_ivc[type];
      m_last_vc_winner[op] = ((ip * m_num_vc + ivc) + 1) % (m_num_port * m_num_vc);
      return ;
    }
  }
}


// ST-stage
void router_c::sw_traversal(void)
{
  // ST (Switch Traversal) stage
  for(int sw_iter = 0; sw_iter < m_sw_iter; ++sw_iter) {
  for (int op = 0; op < m_num_port; ++op) {
    for (int sw = 0; sw < m_num_switch; ++sw) {
        if (m_sw_avail[op][sw] == false) {
          int ip = m_sw_ip[op][sw];
          int ivc = m_sw_vc[op][sw];
          for (auto I = m_ivc_buffer[ip][ivc].begin(), E = m_ivc_buffer[ip][ivc].end(); I != E; ++I) {
            flit_c* flit = (*I);
            if (m_ivc_sw[ip][ivc] != sw)
              continue;

            if (flit->m_state == SA || flit->m_state == IB) {
              flit->m_state = ST;
              DEBUG("node:%d ST req_id:%d flit_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
                  m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
                  ip, ivc, m_ivc_rc[ip][ivc], m_ivc_vc[ip][ivc]);
              break;
            }
          }
        }
      }
    }
  }
}


// LT stage
void router_c::link_traversal(void)
{
  // LT (Link Traversal) stage
  // model link latency here
  for (int ip = 0; ip < m_num_port; ++ip) {
    for (int ivc = 0; ivc < m_num_vc; ++ivc) {
      if (m_ivc_buffer[ip][ivc].empty())
        continue;

      flit_c* flit = m_ivc_buffer[ip][ivc].front();
      Counter timestamp = flit->m_timestamp;
      if (flit->m_state == ST) {
        // insert to next router
        int op = m_ivc_rc[ip][ivc];
        int ovc = m_ivc_vc[ip][ivc];
        if (op != LOCAL) {
          m_link[op]->insert_packet(flit, m_opposite_dir[op], ovc);
          flit->m_rdy_cycle = CYCLE + m_link_latency;
        }

        // delete flit in the buffer
        m_ivc_buffer[ip][ivc].pop_front();

        // free 1) switch, 2) ivc_avail[ip][ivc], 3) rc, 4) vc
        if (flit->m_tail) {
          STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE);
          STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER, CYCLE - timestamp);
          
          STAT_EVENT(NOC_AVG_WAIT_IN_ROUTER_BASE_CPU + m_type);
          STAT_EVENT_N(NOC_AVG_WAIT_IN_ROUTER_CPU + m_type, CYCLE - timestamp);

          m_sw_avail[op][m_ivc_sw[ip][ivc]] = true;
          m_ivc_rc[ip][ivc] = -1;
          m_ivc_vc[ip][ivc] = -1;
          m_ivc_sw[ip][ivc] = -1;
          m_ivc_avail[ip][ivc] = true;

          if (op == LOCAL) {
//            (*m_total_packet)--;
            --g_total_packet;
            if (flit->m_req->m_ptx) {
              --g_total_gpu_packet;
            }
            else {
              --g_total_cpu_packet;
            }
            m_req_buffer->push(flit->m_req);
            DEBUG("node:%d LOCAL req_id:%d flit_id:%d src:%d dst:%d ip:%d vc:%d\n",
                m_id, flit->m_req->m_id, flit->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst, 
                ip, ivc);

            STAT_EVENT(NOC_AVG_LATENCY_BASE);
            STAT_EVENT_N(NOC_AVG_LATENCY, CYCLE - flit->m_req->m_noc_cycle);
            
            STAT_EVENT(NOC_AVG_LATENCY_BASE_CPU + flit->m_req->m_ptx);
            STAT_EVENT_N(NOC_AVG_LATENCY_CPU + flit->m_req->m_ptx, CYCLE - flit->m_req->m_noc_cycle);
          }
        }

        if (op == LOCAL) {
          flit->init();
          m_flit_pool->release_entry(flit);
          m_ovc_avail[LOCAL][ovc] = true;
        }
      }
    }
  }
}


void router_c::insert_packet(flit_c* flit, int ip, int ivc)
{
  // IB (Input Buffering) stage
  if (flit->m_head)
    DEBUG("node:%d IB req_id:%d src:%d dst:%d ip:%d vc:%d\n",
        m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst, ip, ivc);

  m_ivc_buffer[ip][ivc].push_back(flit);
  flit->m_state = IB;
  flit->m_timestamp = CYCLE;
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

void router_c::init(int total_router, int* total_packet, pool_c<flit_c>* flit_pool)
{
  m_total_router = total_router;
  m_total_packet = total_packet;
  m_flit_pool = flit_pool;
}


void router_c::print(ofstream& out)
{
#if 0
  out << "router:" << m_id << "\n";
  for (int ip = 0; ip < m_num_port; ++ip) {
    out << "----- input port " << ip << " -----\n";
    for (int ivc = 0; ivc < m_num_vc; ++ivc) {
      out << "vc:" << ivc << "\n";
      out << "buffer size:" << m_ivc_buffer[ip][ivc].size() << "\n";
      out << "ivc_avail:" << m_ivc_avail[ip][ivc] << "\n";
      out << "ivc_rc:" << m_ivc_rc[ip][ivc] << "\n";
      out << "ivc_vc:" << m_ivc_vc[ip][ivc] << "\n";
      if (m_ivc_buffer[ip][ivc].size() > 0)
        out << "req_id:" << m_ivc_buffer[ip][ivc].front()->m_req->m_id 
          << " src:" << m_ivc_buffer[ip][ivc].front()->m_src
          << " dst:" << m_ivc_buffer[ip][ivc].front()->m_dst << "\n";
    }
  }
  out << "\n";

  for (int op = 0; op < m_num_port; ++op) {
    out << "----- output port " << op << " -----\n";
    for (int ii = 0; ii < m_num_switch; ++ii)
      out << "m_sw_avail " << m_sw_avail[op][ii] << "\n";
    out << "m_sw_ip    " << m_sw_ip[op] << "\n";
    out << "m_sw_vc    " << m_sw_vc[op] << "\n";
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      out << "ovc_avail[" << ovc << "] " << m_ovc_avail[op][ovc] << "\n";
    }
  }
  out << "\n";
#endif
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
          DEBUG("node:%d req_id:%d rc changed %d count:%d\n", m_id, flit->m_req->m_id, m_ivc_rc[ip][ivc], count);

#if 0
          if (CYCLE = 38029 && flit->m_req->m_id == 9181) {
            print();
            assert(0);
          }
#endif
        }
        

#if 0
        for (int ii = 0; ii < m_num_vc; ++ii) {
          DEBUG("node:%d req_id:%d ovc[LEFT][%d]:%d\n", m_id, flit->m_req->m_id, ii, m_ovc_avail[LEFT][ii]);
          DEBUG("node:%d req_id:%d ovc[RIGHT][%d]:%d\n", m_id, flit->m_req->m_id, ii, m_ovc_avail[RIGHT][ii]);
        }
#endif
      }
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////


router_wrapper_c::router_wrapper_c(macsim_c* simBase)
  : m_simBase(simBase)
{
  m_total_packet = 0;

  m_flit_pool  = new pool_c<flit_c>(100, "flit");
}

router_wrapper_c::~router_wrapper_c()
{
  delete m_flit_pool;
}


// run one cycle for all routers
void router_wrapper_c::run_a_cycle(void)
{
  // randomized tick function
  int index = CYCLE % m_num_router;
  for (int ii = index; ii < index + m_num_router; ++ii) {
    m_router[ii % m_num_router]->run_a_cycle();
  }
}

// create a router
router_c* router_wrapper_c::create_router(int type)
{
  report("router:" << m_num_router << " type:" << type << " creatd");

  router_c* new_router = new router_c(m_simBase, type, m_num_router++);
  m_router.push_back(new_router);

  return new_router;
}

void router_wrapper_c::init(void)
{
  // topology - setting router links
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &g_total_packet, m_flit_pool);
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
