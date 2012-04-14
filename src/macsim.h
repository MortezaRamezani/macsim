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
 * File         : macsim.h
 * Author       : Jaekyu Lee
 * Date         : 3/25/2011
 * SVN          : $Id: main.cc 911 2009-11-20 19:08:10Z kacear $:
 * Description  : MacSim Wrapper Class
 *********************************************************************************************/

#ifndef MACSIM_H_INCLUDED
#define MACSIM_H_INCLUDED


#include <unordered_map>

#include "global_defs.h"
#include "global_types.h"

#include "manifold/kernel/include/kernel/clock.h"

#ifndef DONT_INCLUDE_MANIFOLD // to prevent circular link dependencies
#include "manifold/models/iris/interfaces/topology.h"
#include "manifold/models/iris/interfaces/irisInterface.h"
#include "manifold/models/iris/interfaces/irisTerminal.h"
#include "manifold/models/iris/iris_srcs/components/manifoldProcessor.h"
#include "manifold/models/iris/iris_srcs/topology/ring.h"
#include "manifold/models/iris/iris_srcs/topology/torus.h"
#include "manifold/models/iris/iris_srcs/topology/mesh.h"
#include "manifold/kernel/include/kernel/manifold.h"
#include "manifold/kernel/include/kernel/component.h"
#include "manifold/models/iris/iris_srcs/components/ninterface.h"
#include "manifold/models/iris/iris_srcs/components/simpleRouter.h"
#endif


#define CYCLE m_simBase->m_simulation_cycle

class Topology;

///////////////////////////////////////////////////////////////////////////////////////////////
/// \brief macsim top-level class
///////////////////////////////////////////////////////////////////////////////////////////////
class macsim_c
{
	public:
    /**
     * Constructor
     */
		macsim_c();

    /**
     * Destructor
     */
		~macsim_c();

    /**
     * Initialization simulation (wrapper)
     */
		void initialize(int argc, char** argv);

    /**
     * Run a cycle (tick)
     */
		int  run_a_cycle();

    /**
     * Finalize simulation (wrapper)
     */
		void finalize();

    /**
     * Init knob variables
     */
		void init_knobs(int argc, char** argv);

    /**
     * Register functions (for different type of memory, cache, fetch policies, and etc.)
     */
		void register_functions(void);

    /**
     * Initialize simulation
     */
		void init_sim(void);

    /**
     * Initialize memory system
     */
		void init_memory(void);

    /**
     * Initialize clock domain for different components (CPU, GPU, L3, NOC, MC)
     */
    void init_clock_domain(void);

    /**
     * Initialize output streams
     */
		void init_output_streams(void);

    /**
     * Initialize cores
     */
		void init_cores(int num_core);

    /**
     * Initialize network
     */
		void init_network(void);

    /**
     * Open trace files
     */
		void open_traces(string trace_file);

    /**
     * Deallocate data structure
     */
		void deallocate_memory(void);

#ifdef POWER_EI
    /**
     * Compute power consumption
     */
		void compute_power(void);
#endif

    /**
     * Finialize simulation
     */
		void fini_sim(void);

    /**
     * Create routers
     */
    router_c* create_router(int type);

    /**
     * Initialize iris configuration
     */
		void init_iris_config(map<string, string> &params);

	public:
		int m_num_active_threads; /**< number of active threads */
		int m_num_waiting_dispatched_threads; /**< number of threads waiting for begin dispatched */
		int m_total_num_application; /**< total number of applications */
		int m_process_count; /**< number of processes */
		int m_process_count_without_repeat; /**< number of processes without repeat */
		int m_all_threads; /**< number of all threads */
		int m_no_threads_per_block; /**< number of threads per block */
		int m_total_retired_block; /**< number of retired blocks total */
		int m_num_running_core; /**< set to any non0 number before simulation start */
		bool m_repeat_done; /**< in trace repeat mode, indicate repetition done */
		bool m_gpu_paused;  /**< indicate whether GPU can start its execution */

		FILE *g_mystdout;	/**< default output stream */
		FILE *g_mystderr;	/**< default error stream */
		FILE *g_mystatus; /**< default status stream */
		Counter m_simulation_cycle; /**< simulation cycle count */
		Counter m_core0_inst_count; /**< core 0 inst count for debug/assert */
		Counter m_core_cycle[MAX_NUM_CORES];/**< core cycle count */
		int m_end_simulation; /**< flag to end simulation */

    // statistics
		all_stats_c*         m_allStats; /**< all statistics */
		ProcessorStatistics* m_ProcessorStats; /**< processor stats */
		CoreStatistics*      m_coreStatsTemplate; /**< core stat template */
		map<string, ofstream*> m_AllStatsOutputStreams; /**< stat output streams */
		ofstream *m_core_stat_files[MAX_NUM_CORES]; /**<  core statistics files */

    // knob variables
		KnobsContainer *m_knobsContainer; /**< knob container */
		all_knobs_c    *m_knobs; /**< all knob variables */

		bool m_core_end_trace[MAX_NUM_CORES]; /**< core end trace flag */
		bool m_sim_end[MAX_NUM_CORES]; /**< core sim end flag */
		bool m_core_started[MAX_NUM_CORES]; /**< core started flag */

		core_c *m_core_pointers[MAX_NUM_CORES]; /**< core pointers */
		memory_c* m_memory; /**< main memory */
    dram_controller_c** m_dram_controller; /**< dram controller */
    int m_num_mc; /**< number of memory controllers */
		noc_c* m_noc; /**<  interconnection network */
		trace_read_c* m_trace_reader; /**< trace reader */

    // clock handling
    int m_clock_lcm; /**< main clock period */
    int m_clock_divisor[5]; /**< clock divisor */
    int m_clock_internal; /**< internal macsim clock */
    bool m_termination_check[2];

    // bug detector
		bug_detector_c *m_bug_detector; /**< bug detector */

    // process manager
		process_manager_c* m_process_manager; /**< process manager */
		queue<int> m_x86_core_pool; /**< x86 cores pool */
		queue<int> m_ptx_core_pool; /**< GPU cores pool */
		multi_key_map_c* m_block_id_mapper; /**< block id mapper */

	
    // data structure pools (to reduce overhead of memory allocation)
		pool_c<thread_s>* m_thread_pool; /**<  thread data pool */
		pool_c<section_info_s>* m_section_pool; /**<  section data pool */
		pool_c<mem_map_entry_c>* m_mem_map_entry_pool; /**<  memory dependence data pool */
		pool_c<heartbeat_s>* m_heartbeat_pool; /**<  heartbeat data pool */
		pool_c<bp_recovery_info_c>* m_bp_recovery_info_pool; /**<  bp recovery information pool */
		pool_c<thread_trace_info_node_s>* m_trace_node_pool; /**<  trace node pool */
		pool_c<uop_c> *m_uop_pool; /**<  uop pool */
		uop_c *m_invalid_uop; /**<  invalide uop pointer (for uop pool maintenance) */

		unordered_map<int, hash_c<inst_info_s>*> m_inst_info_hash; /**< decoded instruction map */
		unordered_map<int, block_schedule_info_s*> m_block_schedule_info; /**< block schedule info */
		unordered_map<int, process_s*> m_sim_processes; /**< process map */
		unordered_map<int, thread_stat_s*> m_thread_stats; /**< thread stat map */

		struct timeval m_begin_sim; /**< simulation start time */
		struct timeval m_end_sim; /**< simulation termination time */

		// interconnect
    router_wrapper_c* m_router; /**< NOC routers */

    // IRIS
		vector<ManifoldProcessor*> m_macsim_terminals; /**< manifold terminals */
		manifold::kernel::Clock* master_clock; /**< manifold clock - has to be global or static */
		map<string, string> m_iris_params; /**< iris configurations */
		Topology* m_iris_network; /**< iris topology */
		Topology* tp; /**< topology */
		uint no_nodes; /**< number of network nodes */
		int Mytid; /**< tid used by IRIS */
		FILE* log_file; /**< log file used by IRIS */

    // cache partitioning
    cache_partition_framework_c* m_PCL; /**< cache partitioning framework */


#ifdef POWER_EI
		// power stats
		ei_power_c* m_ei_power; /**< energy introspector */
		double avg_power; /**< average power */
		double total_energy; /**< total energy consumption */
		int total_packets; /**< number of packets */
#endif

	private:
		macsim_c* m_simBase; /**< self-reference for macro usage */
};

#endif
