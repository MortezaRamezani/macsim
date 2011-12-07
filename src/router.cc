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

#define DEBUG(args...) _DEBUG(*m_simBase->m_knobs->KNOB_DEBUG_NOC, ## args)

/////////////////////////////////////////////////////////////////////////////////////////
// m_sw_avail : switch availability
//   allocated to a flit with VCA stage
//   deallocated when this flit finishes LT stage
/////////////////////////////////////////////////////////////////////////////////////////

flit_c::flit_c()
{
  init();
}

flit_c::~flit_c()
{
}

void flit_c::init(void)
{
  m_src  = -1;
  m_dst  = -1;
  m_dir = -1;
  m_head = false;
  m_tail = false;
  m_req  = NULL;
  m_state = INIT;
  m_rdy_cycle = ULLONG_MAX;
}

/////////////////////////////////////////////////////////////////////////////////////////

router_c::router_c(macsim_c* simBase, int id)
  : m_simBase(simBase), m_id(id)
{
  m_num_vc       = *KNOB(KNOB_NUM_VC); 
  m_num_port     = *KNOB(KNOB_NUM_PORT);
  m_link_latency = *KNOB(KNOB_LINK_LATENCY) + 1;
  m_link_width   = *KNOB(KNOB_LINK_WIDTH);

  m_ivc_avail  = new bool*[m_num_port];
  m_ovc_avail  = new bool*[m_num_port];
  m_ivc_rc     = new int*[m_num_port];
  m_ivc_vc     = new int*[m_num_port];
  m_ivc_buffer = new list<flit_c*>*[m_num_port];
  m_sw_avail  = new bool[m_num_port];
  m_sw_ip  = new int[m_num_port];
  m_sw_vc  = new int[m_num_port];

  fill_n(m_sw_avail, m_num_port, true);
  fill_n(m_sw_ip, m_num_port, -1);
  fill_n(m_sw_vc, m_num_port, -1);


  for (int ii = 0; ii < m_num_port; ++ii) {
    m_ivc_avail[ii] = new bool[m_num_vc];
//    m_ovc_avail[ii] = new bool[m_num_vc];
    fill_n(m_ivc_avail[ii], m_num_vc, true);
//    fill_n(m_ovc_avail[ii], m_num_vc, true);

    m_ivc_rc[ii] = new int[m_num_vc];
    fill_n(m_ivc_rc[ii], m_num_vc, -1);
    
    m_ivc_vc[ii] = new int[m_num_vc];
    fill_n(m_ivc_vc[ii], m_num_vc, -1);

    m_ivc_buffer[ii] = new list<flit_c*>[m_num_vc];
  }

  m_ovc_avail[0] = new bool[m_num_vc];
  fill_n(m_ovc_avail[0], m_num_vc, true);


  m_flit_pool  = new pool_c<flit_c>;
  m_req_buffer = new queue<mem_req_s*>;

  m_opposite_dir[LOCAL] = LOCAL;
  m_opposite_dir[LEFT]  = RIGHT;
  m_opposite_dir[RIGHT] = LEFT;
  m_opposite_dir[UP]    = DOWN;
  m_opposite_dir[DOWN]  = UP;
}

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

  delete m_flit_pool;
  delete m_req_buffer;
}


int router_c::get_id(void)
{
  return m_id;
}

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

bool router_c::send_packet(mem_req_s* req)
{
  if (*m_total_packet >= m_total_router * m_num_vc * (m_num_port - 1))
    return false;

  // available local vc check
  int index = -1;
  for (int ii = 0; ii < m_num_vc; ++ii) {
    if (m_ivc_avail[LOCAL][ii] == true) {
      index = ii;
      m_ivc_avail[LOCAL][ii] = false;
      break;
    } 
  }

  if (index == -1)
    return false;

  
  // flit generation
  int num_flit = 1;
  (*m_total_packet)++;

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

    // insert all flits to m_buffer[LOCAL][vc]
    m_ivc_buffer[LOCAL][index].push_back(new_flit);
  }

  DEBUG("node:%d IB req_id:%d src:%d dst:%d ip:%d vc:%d\n",
      m_id, req->m_id, req->m_msg_src, req->m_msg_dst, LOCAL, index);

  return true;
}


void router_c::run_a_cycle(void)
{
  check_starvation();
  link_traversal();
  sw_traversal();
  sw_allocation();
  vc_allocation();
  route_calculation();
}



void router_c::route_calculation(void)
{
  // RC (Route Calculation) stage
  for (int ii = 0; ii < m_num_port; ++ii) {
    for (int jj = 0; jj < m_num_vc; ++jj) {
      for (auto I = m_ivc_buffer[ii][jj].begin(), E = m_ivc_buffer[ii][jj].end(); I != E; ++I) { 
        if ((*I)->m_head == true && (*I)->m_state == IB && (*I)->m_rdy_cycle <= CYCLE) {
          // ring
          if (m_id == (*I)->m_dst) {
            m_ivc_rc[ii][jj] = LOCAL;
          }
          else if ((*I)->m_dir != -1) {
            m_ivc_rc[ii][jj] = (*I)->m_dir;
          }
          else {
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

            if (left < right) { 
              m_ivc_rc[ii][jj] = LEFT;
            }
            else { 
              m_ivc_rc[ii][jj] = RIGHT;
            }

            /*
            if (rand() % 100 < 20) {
              m_ivc_rc[ii][jj] = m_opposite_dir[m_ivc_rc[ii][jj]];
            }
            */
            (*I)->m_dir = m_ivc_rc[ii][jj];
          }
          (*I)->m_state = RC;
          DEBUG("node:%d RC req_id:%d src:%d dst:%d ip:%d vc:%d rc:%d\n",
              m_id, (*I)->m_req->m_id, (*I)->m_req->m_msg_src, (*I)->m_req->m_msg_dst,
              ii, jj, m_ivc_rc[ii][jj]);
        }
      }
    }
  }
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

          DEBUG("node:%d VCA req_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
              m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
              ip, ivc, m_ivc_rc[ip][ivc], ovc);
        }
      }
    }
  }
}

void router_c::pick_vc_winner(int op, int ovc, int& ip, int& ivc)
{
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
        oldest_timestamp = flit->m_timestamp;
        ip  = ii;
        ivc = jj;
      }
    }
  }
}


// SA (Switch Allocation) stage
void router_c::sw_allocation(void)
{
  // search all output port switch availability
  for (int op = 0; op < m_num_port; ++op) {
    if (!m_sw_avail[op]) 
      continue;

    int ip, ivc;
    pick_swa_winner(op, ip, ivc); 

    if (ip != -1) {
      m_sw_avail[op] = false;
      m_sw_ip[op] = ip;
      m_sw_vc[op] = ivc;
        
      flit_c* flit = m_ivc_buffer[ip][ivc].front();
      flit->m_state = SA;

      DEBUG("node:%d SA req_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
          m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
          ip, ivc, m_ivc_rc[ip][ivc], m_ivc_vc[ip][ivc]);
    }
  }
}


void router_c::pick_swa_winner(int op, int& ip, int& ivc)
{
  Counter oldest_timestamp = ULLONG_MAX;
  ip = -1;
  for (int ii = 0; ii < m_num_port; ++ii) {
//    if (ii == op)
//      continue;

    for (int jj = 0; jj < m_num_vc; ++jj) {
      if (m_ivc_rc[ii][jj] == op && m_ivc_vc[ii][jj] != -1) {
        assert(!m_ivc_buffer[ii][jj].empty());
      
        flit_c* flit = m_ivc_buffer[ii][jj].front();
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

void router_c::sw_traversal(void)
{
  // ST (Switch Traversal) stage
  for (int op = 0; op < m_num_port; ++op) {
    if (m_sw_avail[op] == false) {
      int ip = m_sw_ip[op];
      int ivc = m_sw_vc[op];
      for (auto I = m_ivc_buffer[ip][ivc].begin(), E = m_ivc_buffer[ip][ivc].end(); I != E; ++I) {
        flit_c* flit = (*I);
        if (flit->m_state == SA || flit->m_state == IB) {
          flit->m_state = ST;
          DEBUG("node:%d ST req_id:%d src:%d dst:%d ip:%d ic:%d op:%d oc:%d\n",
              m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst,
              ip, ivc, m_ivc_rc[ip][ivc], m_ivc_vc[ip][ivc]);
          break;
        }
      }
    }
  }
}

void router_c::link_traversal(void)
{
  // LT (Link Traversal) stage
  // model link latency here
  for (int ip = 0; ip < m_num_port; ++ip) {
    for (int ivc = 0; ivc < m_num_vc; ++ivc) {
      if (m_ivc_buffer[ip][ivc].empty())
        continue;

      flit_c* flit = m_ivc_buffer[ip][ivc].front();
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
          m_sw_avail[op] = true;
          m_ivc_rc[ip][ivc] = -1;
          m_ivc_vc[ip][ivc] = -1;
          m_ivc_avail[ip][ivc] = true;

          if (op == LOCAL) {
            (*m_total_packet)--;
            m_req_buffer->push(flit->m_req);
            DEBUG("node:%d LOCAL req_id:%d src:%d dst:%d ip:%d vc:%d\n",
                m_id, flit->m_req->m_id, flit->m_req->m_msg_src, flit->m_req->m_msg_dst, 
                ip, ivc);
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

void router_c::init(int total_router, int* total_packet)
{
  m_total_router = total_router;
  m_total_packet = total_packet;
}


void router_c::print(ofstream& out)
{
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
    out << "m_sw_avail " << m_sw_avail[op] << "\n";
    out << "m_sw_ip    " << m_sw_ip[op] << "\n";
    out << "m_sw_vc    " << m_sw_vc[op] << "\n";
    for (int ovc = 0; ovc < m_num_vc; ++ovc) {
      out << "ovc_avail[" << ovc << "] " << m_ovc_avail[op][ovc] << "\n";
    }
  }
  out << "\n";
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
}

router_wrapper_c::~router_wrapper_c()
{
}


// run one cycle for all routers
void router_wrapper_c::run_a_cycle(void)
{
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->run_a_cycle();
  }
}

// create a router
router_c* router_wrapper_c::create_router(void)
{
  report("router:" << m_num_router << " creatd");
  router_c* new_router = new router_c(m_simBase, m_num_router++);
  m_router.push_back(new_router);

  return new_router;
}

void router_wrapper_c::init(void)
{
  // topology - setting router links
  for (int ii = 0; ii < m_num_router; ++ii) {
    m_router[ii]->init(m_num_router, &m_total_packet);
  }

  for (int ii = 1; ii < m_num_router-1; ++ii) {
    m_router[ii]->set_link(LEFT, m_router[ii-1]);
    m_router[ii]->set_link(RIGHT, m_router[ii+1]);
  }

  m_router[0]->set_link(LEFT, m_router[m_num_router-1]);
  m_router[0]->set_link(RIGHT, m_router[1]);
  
  m_router[m_num_router-1]->set_link(LEFT, m_router[m_num_router-2]);
  m_router[m_num_router-1]->set_link(RIGHT, m_router[0]);
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
