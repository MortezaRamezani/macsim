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
* File         : ei_power.cc
* Author       : Jieun Lim 
* Date         : 9/6/2011
* SVN          :   
* Description  : main file to use Energy Introspector to compute power 
*********************************************************************************************/

#include <iostream>
#include <stdio.h>
#include <string>
#include <math.h>

#include "ei_power.h"

#include "all_knobs.h"
#include "all_stats.h"

#include "energy_introspector/energy_introspector.h"	

#define CONFIG_FILE_NAME "ei.config"

#define GET_CORE_STAT(coreID, Event)	\
  m_simBase->m_ProcessorStats->core(coreID)[Event-PER_CORE_STATS_ENUM_FIRST].getCount() \

#define GET_STAT(Event) \
  (*m_simBase->m_ProcessorStats)[Event].getCount() \

#define GET_KNOB(Event) \
  m_simBase->m_knobs->KNOB_##Event->getValue() \

#define LOG2(x) (int)(log((double)x)/log((double)2))
#define OVERHEAD 5

using namespace std;


// ei_power_c constructor
ei_power_c::ei_power_c(macsim_c* simBase)
{
  m_simBase = simBase;
}


//ei_power_c destructor 
ei_power_c::~ei_power_c()
{
}


// generate technology parameters for a large core
void ei_power_c::ei_config_gen_large_tech(FILE* fp, int core_id)
{
  fprintf(fp, "# large core\n");
  fprintf(fp, "# technology\n");
  fprintf(fp, "-technology.ID                           LARGE_CORE\n");	
  fprintf(fp, "-technology.clock_frequency              %.3fe9 \n", GET_KNOB(CLOCK_CPU));
  fprintf(fp, "-technology.feature_size                 %de-9\n", GET_KNOB(FEATURE_SIZE));
  fprintf(fp, "-technology.component_type               core\n");
  fprintf(fp, "-technology.core_type                    %s\n", 
	       (GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")? "ooo" : "inorder");
  fprintf(fp, "-technology.wire_type                    global\n");
  fprintf(fp, "-technology.device_type                  hp\n");
  fprintf(fp, "-technology.interconnect_projection      aggressive\n");
  fprintf(fp, "-technology.wiring_type                  global\n");
  fprintf(fp, "-technology.opt_local                    false\n");
  fprintf(fp, "-technology.embedded                     false\n");	
  fprintf(fp, "-technology.longer_channel_device        false\n");
  fprintf(fp, "-technology.temperature                  %d\n", GET_KNOB(TEMPERATURE));
  fprintf(fp, "-technology.end\n");
  fprintf(fp, "\n");
}


// generate module parameters for a large core
void ei_power_c::ei_config_gen_large_mod(FILE* fp, int core_id )
{
  int pipeline_total = GET_KNOB(LARGE_CORE_FETCH_LATENCY)+GET_KNOB(LARGE_CORE_ALLOC_LATENCY)+2;
  
  fprintf(fp, "#undifferentiated core -- fetch partition\n");
  fprintf(fp, "-module.ID                               core%d:fetch\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE   \n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(LARGE_CORE_FETCH_LATENCY) - 1)/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- decode partition\n");
  fprintf(fp, "-module.ID                               core%d:decode\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(LARGE_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.4);	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- renaming partition\n");
  fprintf(fp, "-module.ID                               core%d:renaming\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(LARGE_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.2);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- schedule partition\n");
  fprintf(fp, "-module.ID                               core%d:schedule\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(LARGE_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.4);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- memory partition\n");
  fprintf(fp, "-module.ID                               core%d:memory\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 2/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- execute partition\n");
  fprintf(fp, "-module.ID                               core%d:execute\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 2/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#program counter\n");
  fprintf(fp, "-module.ID                               core%d:program_counter\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#fetch queue\n");
  fprintf(fp, "-module.ID                               core%d:byteQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/2);	
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(LARGE_WIDTH) * 4);
  fprintf(fp, "-module.rd_ports                         %d\n", 1);
  fprintf(fp, "-module.wr_ports                         1\n");
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: fetch queue to predecoder\n");
  fprintf(fp, "-module.ID                               core%d:BQ2PD\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 320\n");
  fprintf(fp, "-module.scaling                          1\n" );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: predecoder to instruction queue\n");
  fprintf(fp, "-module.ID                               core%d:PD2IQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 184\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction queue\n");
  fprintf(fp, "-module.ID                               core%d:instQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       15\n");
  fprintf(fp, "-module.sets                             %d\n", 
	        (GET_KNOB(LARGE_WIDTH) * GET_KNOB(LARGE_CORE_ALLOC_LATENCY) + 1) / 2); // +1: for ceiling
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));	
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH) + 2); 
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#branch predictor\n");
  fprintf(fp, "-module.ID                               core%d:bpred1\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       1 # 2-bit saturation counter\n");
  fprintf(fp, "-module.sets                             %d\n", (int)pow(2, GET_KNOB(BP_HIST_LENGTH)));
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#branch target buffer\n");
  fprintf(fp, "-module.ID                               core%d:BTB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.assoc                            %d\n", 4);
  fprintf(fp, "-module.sets                             %d\n", 128);
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#return address stack\n");
  fprintf(fp, "-module.ID                               core%d:RAS\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.sets                             %d\n", 64);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction TLB\n");
  fprintf(fp, "-module.ID                               core%d:ITLB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.assoc                            %d\n", 4);
  fprintf(fp, "-module.sets                             %d\n", 128);
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_time                      2\n");	 
  fprintf(fp, "-module.cycle_time                       %d\n", 2);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache\n");
  fprintf(fp, "-module.ID                               core%d:ICache\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(ICACHE_LARGE_LINE_SIZE) );
  fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(ICACHE_LARGE_ASSOC));
  fprintf(fp, "-module.banks                            %d\n", GET_KNOB(ICACHE_LARGE_BANKS));
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ICACHE_LARGE_NUM_SET));
  fprintf(fp, "-module.tag_width                        %d\n", 
	        GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD - LOG2(GET_KNOB(ICACHE_LARGE_LINE_SIZE)) \
					- LOG2(GET_KNOB(ICACHE_LARGE_NUM_SET)));
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(ICACHE_READ_PORTS));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(ICACHE_WRITE_PORTS));
  fprintf(fp, "-module.access_time                      2\n");
  fprintf(fp, "-module.cycle_time                       %d\n", GET_KNOB(ICACHE_LARGE_CYCLES));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache missbuffer\n");
  fprintf(fp, "-module.ID                               core%d:ICache:missbuffer\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", 
	        GET_KNOB(PHY_ADDR_WIDTH)/8 + GET_KNOB(ICACHE_LARGE_LINE_SIZE));
  fprintf(fp, "-module.assoc                            0\n");
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.search_ports                     1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction queue to decoder\n");
  fprintf(fp, "-module.ID                               core%d:IQ2ID\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 184\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction decoder\n");
  fprintf(fp, "-module.ID                               core%d:instruction_decoder\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           16\n");
  fprintf(fp, "-module.area_scaling                     %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#micro-op sequencer\n");
  fprintf(fp, "-module.ID                               core%d:uop_sequencer\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           8\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#operand decoder\n");
  fprintf(fp, "-module.ID                               core%d:operand_decoder\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           8\n" );
  fprintf(fp, "-module.area_scaling                     %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction decoder to uopQ\n");
  fprintf(fp, "-module.ID                               core%d:ID2uQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 40\n");
  fprintf(fp, "-module.scaling                          %d\n", 
	        GET_KNOB(EI_DECODE_WIDTH) * GET_KNOB(GPU_WIDTH));	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#uop queue\n");
  fprintf(fp, "-module.ID                               core%d:uopQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       5\n" );
  fprintf(fp, "-module.sets                             %d\n", 7 * GET_KNOB(LARGE_WIDTH));
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: uopQ to register renaming\n");
  fprintf(fp, "-module.ID                               core%d:uQ2RR\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 32 \n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#register renaming table\n");
  fprintf(fp, "-module.ID                               core%d:RAT\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d \n", 
	       (int)(LOG2(GET_KNOB(ROB_LARGE_SIZE)) * (1 + 16)/8));	// 16: checkpoints
  fprintf(fp, "-module.sets                             32\n");
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.rd_ports                         %d \n", 2 * GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.search_ports                     %d \n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#dependency check logic\n");
  fprintf(fp, "-module.ID                               core%d:operand_dependency\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     dependency_check_logic\n");
  fprintf(fp, "-module.compare_bits                     %d\n", LOG2(GET_KNOB(ROB_LARGE_SIZE)));
  fprintf(fp, "-module.decode_width                     %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.scaling                          2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#freelist\n");
  fprintf(fp, "-module.ID                               core%d:freelist\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", 1);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ROB_LARGE_SIZE));
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", 2 * GET_KNOB(EI_COMMIT_WIDTH) - 1);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#integer register file\n");
  fprintf(fp, "-module.ID                               core%d:IRF\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.size                             %d\n", 
	        GET_KNOB(INT_REGFILE_SIZE)*GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 3 * GET_KNOB(EI_ISSUE_WIDTH));	
  fprintf(fp, "-module.wr_ports                         %d\n", (int)(GET_KNOB(EI_ISSUE_WIDTH) * 1.5));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(MAX_THREADS_PER_LARGE_CORE));
  fprintf(fp, "-module.area_scaling                     1.1 # cdb_overhead\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "#fp register file\n");
  fprintf(fp, "-module.ID                               core%d:FRF\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram \n");
  fprintf(fp, "-module.size                             %d\n", 
	        GET_KNOB(FP_REGFILE_SIZE)*GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(MAX_THREADS_PER_LARGE_CORE));
  fprintf(fp, "-module.area_scaling                     1.1 # cdb_overhead\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "#general purpose\n");
  fprintf(fp, "-module.ID                               core%d:GPREG\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#segment registers\n");
  fprintf(fp, "-module.ID                               core%d:SEGREG\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             8 \n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#control registers\n");
  fprintf(fp, "-module.ID                               core%d:CREG\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             4\n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#flags registers\n");
  fprintf(fp, "-module.ID                               core%d:FLAGREG\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             1 \n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#floating-point registers\n");
  fprintf(fp, "-module.ID                               core%d:FPREG\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8 + 2);
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: register renaming to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:RR2RS\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 40\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: architecture registers to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:ARF2RS\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 64\n");
  fprintf(fp, "-module.scaling                          %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: physical registers to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:ROB2RS\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 71\n");
  fprintf(fp, "-module.scaling                          %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  if ((GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "#reservation station\n");
    fprintf(fp, "-module.ID                               core%d:RS\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.assoc                            0\n");		
    fprintf(fp, "-module.line_width                       20 \n");
    fprintf(fp, "-module.tag_width                        12 \n");
    fprintf(fp, "-module.sets                             %d\n", 
		        GET_KNOB(ISCHED_LARGE_SIZE) + GET_KNOB(MSCHED_LARGE_SIZE) + GET_KNOB(FSCHED_LARGE_SIZE));
    fprintf(fp, "-module.rd_ports                         %d\n", 
		        GET_KNOB(ISCHED_LARGE_RATE) + GET_KNOB(MSCHED_LARGE_RATE) + GET_KNOB(FSCHED_LARGE_RATE));
    fprintf(fp, "-module.wr_ports                         %d\n", 
		        GET_KNOB(ISCHED_LARGE_RATE) + GET_KNOB(MSCHED_LARGE_RATE) + GET_KNOB(FSCHED_LARGE_RATE));
    fprintf(fp, "-module.search_ports                     %d\n", GET_KNOB(EI_ISSUE_WIDTH));
    fprintf(fp, "-module.access_time                      2\n");	 
    fprintf(fp, "-module.cycle_time                       2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#instruction issue selection logic\n");
    fprintf(fp, "-module.ID                               core%d:issue_select\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     selection_logic\n");
    fprintf(fp, "-module.selection_input                  %d\n", 
		        GET_KNOB(ISCHED_LARGE_SIZE) + GET_KNOB(MSCHED_LARGE_SIZE) + GET_KNOB(FSCHED_LARGE_SIZE));
    fprintf(fp, "-module.selection_output                 %d\n", 
		        GET_KNOB(ISCHED_LARGE_RATE) + GET_KNOB(MSCHED_LARGE_RATE)+ GET_KNOB(FSCHED_LARGE_RATE));
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "#latch: payload RAM\n");
  fprintf(fp, "-module.ID                               core%d:payload\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 151\n");
  fprintf(fp, "-module.scaling                          %d \n", GET_KNOB(EI_EXEC_WIDTH) );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - FPU\n");
  fprintf(fp, "-module.ID                               core%d:FPU\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  fpu\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - ALU\n");
  fprintf(fp, "-module.ID                               core%d:ALU\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  alu\n");
  fprintf(fp, "-module.area_scaling                     3\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - MUL \n");
  fprintf(fp, "-module.ID                               core%d:MUL\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  mul\n");
  fprintf(fp, "-module.area_scaling                     3\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: functional unit to ROB\n");
  fprintf(fp, "-module.ID                               core%d:FU2ROB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 87\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_EXEC_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#reorder buffer\n");
  fprintf(fp, "-module.ID                               core%d:ROB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", 
	       (int)((GET_KNOB(32_64_ISA) * 2 + OVERHEAD) / 8));
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ROB_LARGE_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 
	        GET_KNOB(EI_ISSUE_WIDTH) + GET_KNOB(EI_COMMIT_WIDTH) );
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_EXEC_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction commit selection logic\n");
  fprintf(fp, "-module.ID                               core%d:commit_select\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     selection_logic\n");
  fprintf(fp, "-module.selection_input                  %d\n", GET_KNOB(ROB_LARGE_SIZE));
  fprintf(fp, "-module.selection_output                 %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: ROB to commit\n");
  fprintf(fp, "-module.ID                               core%d:ROB2CM\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 133\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#load queue\n");
  fprintf(fp, "-module.ID                               core%d:loadQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8 );
  fprintf(fp, "-module.assoc                            0\n");	
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA)+OVERHEAD + 16);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(LOAD_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 2);
  fprintf(fp, "-module.wr_ports                         %d\n", 2);
  fprintf(fp, "-module.search_ports                     %d\n", 2);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to ROB\n");
  fprintf(fp, "-module.ID                               core%d:LQ2ROB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA) + 6);
  fprintf(fp, "-module.scaling                          2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to DCache\n");
  fprintf(fp, "-module.ID                               core%d:LQ2L1\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#store queue\n");
  fprintf(fp, "-module.ID                               core%d:storeQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", 32 / 8);
  fprintf(fp, "-module.assoc                            0\n");
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD + 16);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(STORE_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 1);
  fprintf(fp, "-module.wr_ports                         %d\n", 1);
  fprintf(fp, "-module.search_ports                     %d\n", 1);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to DCache\n");
  fprintf(fp, "-module.ID                               core%d:SQ2L1\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to load queue\n");
  fprintf(fp, "-module.ID                               core%d:SQ2LQ\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#data TLB\n");
  fprintf(fp, "-module.ID                               core%d:DTLB\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d \n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.assoc                            4\n");
  fprintf(fp, "-module.sets                             128\n");
  fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         2\n");
  fprintf(fp, "-module.search_ports                     2\n");
  fprintf(fp, "-module.access_time                      2\n");
  fprintf(fp, "-module.cycle_time                       2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");

  if (!GET_KNOB(L1_LARGE_BYPASS)) {
    fprintf(fp, "#data cache\n");
    fprintf(fp, "-module.ID                               core%d:DCache\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(L1_LARGE_ASSOC));
    fprintf(fp, "-module.banks                            %d\n", GET_KNOB(L1_LARGE_NUM_BANK));
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(L1_LARGE_NUM_SET));
    fprintf(fp, "-module.tag_width                        %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD - LOG2(GET_KNOB(L1_LARGE_LINE_SIZE)) \
						- LOG2(GET_KNOB(L1_LARGE_NUM_SET)));
    fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(L1_WRITE_PORTS));
    fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L1_LARGE_LATENCY));
    fprintf(fp, "-module.cycle_time                       3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache missbuffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:missbuffer\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) / 8 + GET_KNOB(L1_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(MEM_MSHR_SIZE) / 4);
    fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:linefill\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");
    fprintf(fp, "-module.sets                             16\n");
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:writeback\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             16\n");
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#latch: DCache to load queue\n");
    fprintf(fp, "-module.ID                               core%d:L12LQ\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     pipeline\n");
    fprintf(fp, "-module.pipeline_stages                  1\n");
    fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }

  if (!GET_KNOB(L2_LARGE_BYPASS)) {
    fprintf(fp, "#L2 cache\n");
    fprintf(fp, "-module.ID                               core%d:L2\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(L2_LARGE_ASSOC));
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(L2_LARGE_NUM_SET));
    fprintf(fp, "-module.tag_width                        %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) + LOG2(GET_KNOB(L2_LARGE_LINE_SIZE)) \
						- LOG2(GET_KNOB(L2_LARGE_NUM_SET)) + OVERHEAD);
    fprintf(fp, "-module.banks                            %d\n", GET_KNOB(L2_LARGE_NUM_BANK));
    fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(L2_READ_PORTS));
    fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(L2_WRITE_PORTS));
    fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L2_LARGE_LATENCY));
    fprintf(fp, "-module.cycle_time                       3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                               core%d:L2:linefill\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             %d\n", 16);
    fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                               core%d:L2:writeback\n", core_id);
    fprintf(fp, "-module.technology                       LARGE_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_LARGE_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");
    fprintf(fp, "-module.sets                             %d\n", 16);
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.cycle_time                       8\n");
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                               core%d:exec_bypass\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     interconnect\n");
  fprintf(fp, "-module.data                             %d\n", GET_KNOB(32_64_ISA) + 8);	// 8: tag
  if ((GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "-module.connect                          core%d:RS\n", core_id);
  }
  fprintf(fp, "-module.connect                          core%d:payload\n", core_id);
  fprintf(fp, "-module.connect                          core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ROB\n", core_id);
  fprintf(fp, "-module.connect                          core%d:FPU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ALU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:MUL\n", core_id);
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_EXEC_WIDTH));	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                               core%d:load_bypass\n", core_id);
  fprintf(fp, "-module.technology                       LARGE_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     interconnect\n");
  fprintf(fp, "-module.data                             %d\n", GET_KNOB(32_64_ISA) + 8);	// 8: tag
  if ((GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "-module.connect                          core%d:RS\n", core_id);
  }
  fprintf(fp, "-module.connect                          core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:FPU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ALU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:MUL\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ROB\n", core_id);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
}


// generate technology parameters for a medium core
void ei_power_c::ei_config_gen_medium_tech(FILE* fp, int core_id)
{
  fprintf(fp, "# medium core technology\n");
  fprintf(fp, "-technology.ID                           MEDIUM_CORE\n");	
  fprintf(fp, "-technology.clock_frequency              %.3fe9 \n", GET_KNOB(CLOCK_CPU));
  fprintf(fp, "-technology.feature_size                 %de-9\n", GET_KNOB(FEATURE_SIZE));
  fprintf(fp, "-technology.component_type               core\n");
  fprintf(fp, "-technology.core_type                    %s\n", 
	       (GET_KNOB(MEDIUM_CORE_SCHEDULE) == "ooo")? "ooo" : "inorder");
  fprintf(fp, "-technology.wire_type                    global\n");
  fprintf(fp, "-technology.device_type                  hp	\n");
  fprintf(fp, "-technology.interconnect_projection      aggressive\n");
  fprintf(fp, "-technology.wiring_type                  global\n");
  fprintf(fp, "-technology.opt_local                    false\n");
  fprintf(fp, "-technology.embedded                     false\n");	
  fprintf(fp, "-technology.longer_channel_device        false\n");
  fprintf(fp, "-technology.temperature                  %d\n", GET_KNOB(TEMPERATURE));
  fprintf(fp, "-technology.end\n");
  fprintf(fp, "\n");

}


// generate module parameters for a medium core
void ei_power_c::ei_config_gen_medium_mod(FILE* fp, int core_id)
{
  fprintf(fp, "# medium core module\n");
  int pipeline_total = GET_KNOB(MEDIUM_CORE_FETCH_LATENCY)+GET_KNOB(MEDIUM_CORE_ALLOC_LATENCY)+2;
  
  fprintf(fp, "#undifferentiated core -- fetch partition\n");
  fprintf(fp, "-module.ID                               core%d:fetch\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE   \n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(MEDIUM_CORE_FETCH_LATENCY) - 1)/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- decode partition\n");
  fprintf(fp, "-module.ID                               core%d:decode\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(MEDIUM_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.4);	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- renaming partition\n");
  fprintf(fp, "-module.ID                               core%d:renaming\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(MEDIUM_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.2);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- schedule partition\n");
  fprintf(fp, "-module.ID                               core%d:schedule\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)(GET_KNOB(MEDIUM_CORE_ALLOC_LATENCY) - 1)/(double)pipeline_total * 0.4);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- memory partition\n");
  fprintf(fp, "-module.ID                               core%d:memory\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 2/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- execute partition\n");
  fprintf(fp, "-module.ID                               core%d:execute\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 2/(double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#program counter\n");
  fprintf(fp, "-module.ID                               core%d:program_counter\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#fetch queue\n");
  fprintf(fp, "-module.ID                               core%d:byteQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/2);	
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(MEDIUM_WIDTH) * 4);
  fprintf(fp, "-module.rd_ports                         %d\n", 1);
  fprintf(fp, "-module.wr_ports                         1\n");
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: fetch queue to predecoder\n");
  fprintf(fp, "-module.ID                               core%d:BQ2PD\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 320\n");
  fprintf(fp, "-module.scaling                          1\n" );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: predecoder to instruction queue\n");
  fprintf(fp, "-module.ID                               core%d:PD2IQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 184\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction queue\n");
  fprintf(fp, "-module.ID                               core%d:instQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       15\n");
  fprintf(fp, "-module.sets                             %d\n", \
	        (GET_KNOB(MEDIUM_WIDTH) * GET_KNOB(MEDIUM_CORE_ALLOC_LATENCY)+1) / 2); // 1: for ceiling
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));	
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH) + 2); 
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#branch predictor\n");
  fprintf(fp, "-module.ID                               core%d:bpred1\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       1 # 2-bit saturation counter\n");
  fprintf(fp, "-module.sets                             %d\n", (int)pow(2, GET_KNOB(BP_HIST_LENGTH)));
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#branch target buffer\n");
  fprintf(fp, "-module.ID                               core%d:BTB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.assoc                            %d\n", 4);
  fprintf(fp, "-module.sets                             %d\n", 128);
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#return address stack\n");
  fprintf(fp, "-module.ID                               core%d:RAS\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.sets                             %d\n", 64);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction TLB\n");
  fprintf(fp, "-module.ID                               core%d:ITLB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8);
  fprintf(fp, "-module.assoc                            %d\n", 4);
  fprintf(fp, "-module.sets                             %d\n", 128);
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.access_time                      2\n");	 
  fprintf(fp, "-module.cycle_time                       %d\n", 2);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache\n");
  fprintf(fp, "-module.ID                               core%d:ICache\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(ICACHE_MEDIUM_LINE_SIZE) );
  fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(ICACHE_MEDIUM_ASSOC));
  fprintf(fp, "-module.banks                            %d\n", GET_KNOB(ICACHE_MEDIUM_BANKS));
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ICACHE_MEDIUM_NUM_SET));
  fprintf(fp, "-module.tag_width                        %d\n", 
	        GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD - LOG2(GET_KNOB(ICACHE_MEDIUM_LINE_SIZE)) \
					- LOG2(GET_KNOB(ICACHE_MEDIUM_NUM_SET)));
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(ICACHE_READ_PORTS));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(ICACHE_WRITE_PORTS));
  fprintf(fp, "-module.access_time                      2\n");
  fprintf(fp, "-module.cycle_time                       %d\n", GET_KNOB(ICACHE_MEDIUM_CYCLES));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache missbuffer\n");
  fprintf(fp, "-module.ID                               core%d:ICache:missbuffer\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", 
	        GET_KNOB(PHY_ADDR_WIDTH)/8 + GET_KNOB(ICACHE_MEDIUM_LINE_SIZE));
  fprintf(fp, "-module.assoc                            0\n");
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.search_ports                     1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction queue to decoder\n");
  fprintf(fp, "-module.ID                               core%d:IQ2ID\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 184\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction decoder\n");
  fprintf(fp, "-module.ID                               core%d:instruction_decoder\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           16\n");
  fprintf(fp, "-module.area_scaling                     %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#micro-op sequencer\n");
  fprintf(fp, "-module.ID                               core%d:uop_sequencer\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           8\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#operand decoder\n");
  fprintf(fp, "-module.ID                               core%d:operand_decoder\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     instruction_decoder\n");
  fprintf(fp, "-module.x86                              true\n");
  fprintf(fp, "-module.opcode                           8\n" );
  fprintf(fp, "-module.area_scaling                     %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction decoder to uopQ\n");
  fprintf(fp, "-module.ID                               core%d:ID2uQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 40\n");
  fprintf(fp, "-module.scaling                          %d\n", 
	        GET_KNOB(EI_DECODE_WIDTH) * GET_KNOB(GPU_WIDTH));	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#uop queue\n");
  fprintf(fp, "-module.ID                               core%d:uopQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       5\n" );
  fprintf(fp, "-module.sets                             %d\n", 7 * GET_KNOB(MEDIUM_WIDTH));
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: uopQ to register renaming\n");
  fprintf(fp, "-module.ID                               core%d:uQ2RR\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 32 \n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#register renaming table\n");
  fprintf(fp, "-module.ID                               core%d:RAT\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d \n", 
	       (int)(LOG2(GET_KNOB(ROB_MEDIUM_SIZE)) * (1 + 16)/8));	// 16: checkpoints
  fprintf(fp, "-module.sets                             32\n");
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.rd_ports                         %d \n", 2 * GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.search_ports                     %d \n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#dependency check logic\n");
  fprintf(fp, "-module.ID                               core%d:operand_dependency\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     dependency_check_logic\n");
  fprintf(fp, "-module.compare_bits                     %d\n", LOG2(GET_KNOB(ROB_MEDIUM_SIZE)));
  fprintf(fp, "-module.decode_width                     %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.scaling                          2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#freelist\n");
  fprintf(fp, "-module.ID                               core%d:freelist\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", 1);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ROB_MEDIUM_SIZE));
  fprintf(fp, "-module.rw_ports                         %d\n", 1);
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", 2 * GET_KNOB(EI_COMMIT_WIDTH) - 1);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#integer register file\n");
  fprintf(fp, "-module.ID                               core%d:IRF\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.size                             %d\n", 
	        GET_KNOB(INT_REGFILE_SIZE)*GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 3 * GET_KNOB(EI_ISSUE_WIDTH));	
  fprintf(fp, "-module.wr_ports                         %d\n", (int)(GET_KNOB(EI_ISSUE_WIDTH) * 1.5));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(MAX_THREADS_PER_MEDIUM_CORE));
  fprintf(fp, "-module.area_scaling                     1.1 # cdb_overhead\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "#fp register file\n");
  fprintf(fp, "-module.ID                               core%d:FRF\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram \n");
  fprintf(fp, "-module.size                             %d\n", 
	        GET_KNOB(FP_REGFILE_SIZE)*GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(MAX_THREADS_PER_MEDIUM_CORE));
  fprintf(fp, "-module.area_scaling                     1.1 # cdb_overhead\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "#general purpose\n");
  fprintf(fp, "-module.ID                               core%d:GPREG\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#segment registers\n");
  fprintf(fp, "-module.ID                               core%d:SEGREG\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             8 \n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#control registers\n");
  fprintf(fp, "-module.ID                               core%d:CREG\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             4\n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#flags registers\n");
  fprintf(fp, "-module.ID                               core%d:FLAGREG\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.sets                             1 \n");
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#floating-point registers\n");
  fprintf(fp, "-module.ID                               core%d:FPREG\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA)/8 + 2);
  fprintf(fp, "-module.sets                             16\n");
  fprintf(fp, "-module.rd_ports                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.energy_scaling                   1.1 # McPAT overhead number\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: register renaming to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:RR2RS\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 40\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: architecture registers to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:ARF2RS\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 64\n");
  fprintf(fp, "-module.scaling                          %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: physical registers to reservation station\n");
  fprintf(fp, "-module.ID                               core%d:ROB2RS\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 71\n");
  fprintf(fp, "-module.scaling                          %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  if ((GET_KNOB(MEDIUM_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "#reservation station\n");
    fprintf(fp, "-module.ID                               core%d:RS\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.assoc                            0\n");		
    fprintf(fp, "-module.line_width                       20 \n");
    fprintf(fp, "-module.tag_width                        12 \n");
    fprintf(fp, "-module.sets                             %d\n", 
		        GET_KNOB(ISCHED_MEDIUM_SIZE) + GET_KNOB(MSCHED_MEDIUM_SIZE) + GET_KNOB(FSCHED_MEDIUM_SIZE));
    fprintf(fp, "-module.rd_ports                         %d\n", 
		        GET_KNOB(ISCHED_MEDIUM_RATE) + GET_KNOB(MSCHED_MEDIUM_RATE) + GET_KNOB(FSCHED_MEDIUM_RATE));
    fprintf(fp, "-module.wr_ports                         %d\n", 
		        GET_KNOB(ISCHED_MEDIUM_RATE) + GET_KNOB(MSCHED_MEDIUM_RATE) + GET_KNOB(FSCHED_MEDIUM_RATE));
    fprintf(fp, "-module.search_ports                     %d\n", GET_KNOB(EI_ISSUE_WIDTH));
    fprintf(fp, "-module.access_time                      2\n");	 
    fprintf(fp, "-module.cycle_time                       2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#instruction issue selection logic\n");
    fprintf(fp, "-module.ID                               core%d:issue_select\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     selection_logic\n");
    fprintf(fp, "-module.selection_input                  %d\n", 
		        GET_KNOB(ISCHED_MEDIUM_SIZE) + GET_KNOB(MSCHED_MEDIUM_SIZE) + GET_KNOB(FSCHED_MEDIUM_SIZE));
    fprintf(fp, "-module.selection_output                 %d\n", 
		        GET_KNOB(ISCHED_MEDIUM_RATE) + GET_KNOB(MSCHED_MEDIUM_RATE)+ GET_KNOB(FSCHED_MEDIUM_RATE));
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "#latch: payload RAM\n");
  fprintf(fp, "-module.ID                               core%d:payload\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 151\n");
  fprintf(fp, "-module.scaling                          %d \n", GET_KNOB(EI_EXEC_WIDTH) );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - FPU\n");
  fprintf(fp, "-module.ID                               core%d:FPU\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  fpu\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - ALU\n");
  fprintf(fp, "-module.ID                               core%d:ALU\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  alu\n");
  fprintf(fp, "-module.area_scaling                     3\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - MUL \n");
  fprintf(fp, "-module.ID                               core%d:MUL\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     functional_unit\n");
  fprintf(fp, "-module.energy_submodel                  mul\n");
  fprintf(fp, "-module.area_scaling                     3\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: functional unit to ROB\n");
  fprintf(fp, "-module.ID                               core%d:FU2ROB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 87\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_EXEC_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#reorder buffer\n");
  fprintf(fp, "-module.ID                               core%d:ROB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  ram\n");
  fprintf(fp, "-module.line_width                       %d\n", 
	       (int)((GET_KNOB(32_64_ISA) * 2 + OVERHEAD) / 8));
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(ROB_MEDIUM_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 
	        GET_KNOB(EI_ISSUE_WIDTH) + GET_KNOB(EI_COMMIT_WIDTH) );
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(EI_EXEC_WIDTH));
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction commit selection logic\n");
  fprintf(fp, "-module.ID                               core%d:commit_select\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     selection_logic\n");
  fprintf(fp, "-module.selection_input                  %d\n", GET_KNOB(ROB_MEDIUM_SIZE));
  fprintf(fp, "-module.selection_output                 %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: ROB to commit\n");
  fprintf(fp, "-module.ID                               core%d:ROB2CM\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 133\n");
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#load queue\n");
  fprintf(fp, "-module.ID                               core%d:loadQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(32_64_ISA) / 8 );
  fprintf(fp, "-module.assoc                            0\n");	
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA)+OVERHEAD + 16);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(LOAD_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 2);
  fprintf(fp, "-module.wr_ports                         %d\n", 2);
  fprintf(fp, "-module.search_ports                     %d\n", 2);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to ROB\n");
  fprintf(fp, "-module.ID                               core%d:LQ2ROB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA) + 6);
  fprintf(fp, "-module.scaling                          2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to DCache\n");
  fprintf(fp, "-module.ID                               core%d:LQ2L1\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#store queue\n");
  fprintf(fp, "-module.ID                               core%d:storeQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", 32 / 8);
  fprintf(fp, "-module.assoc                            0\n");
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(32_64_ISA) + OVERHEAD + 16);
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(STORE_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                         %d\n", 1);
  fprintf(fp, "-module.wr_ports                         %d\n", 1);
  fprintf(fp, "-module.search_ports                     %d\n", 1);
  fprintf(fp, "-module.access_mode                      sequential\n");
  fprintf(fp, "-module.area_scaling                     1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to DCache\n");
  fprintf(fp, "-module.ID                               core%d:SQ2L1\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to load queue\n");
  fprintf(fp, "-module.ID                               core%d:SQ2LQ\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     pipeline\n");
  fprintf(fp, "-module.pipeline_stages                  1\n");
  fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#data TLB\n");
  fprintf(fp, "-module.ID                               core%d:DTLB\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d \n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.assoc                            4\n");
  fprintf(fp, "-module.sets                             128\n");
  fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                         2\n");
  fprintf(fp, "-module.search_ports                     2\n");
  fprintf(fp, "-module.access_time                      2\n");
  fprintf(fp, "-module.cycle_time                       2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");

  if (!GET_KNOB(L1_MEDIUM_BYPASS)) {
    fprintf(fp, "#data cache\n");
    fprintf(fp, "-module.ID                               core%d:DCache\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(L1_MEDIUM_ASSOC));
    fprintf(fp, "-module.banks                            %d\n", GET_KNOB(L1_MEDIUM_NUM_BANK));
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(L1_MEDIUM_NUM_SET));
    fprintf(fp, "-module.tag_width                        %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD - LOG2(GET_KNOB(L1_MEDIUM_LINE_SIZE)) \
						- LOG2(GET_KNOB(L1_MEDIUM_NUM_SET)));
    fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(L1_WRITE_PORTS));
    fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L1_MEDIUM_LATENCY));
    fprintf(fp, "-module.cycle_time                       3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache missbuffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:missbuffer\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) / 8 + GET_KNOB(L1_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(MEM_MSHR_SIZE) / 4);
    fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:linefill\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");
    fprintf(fp, "-module.sets                             16\n");
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                               core%d:DCache:writeback\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L1_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             16\n");
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#latch: DCache to load queue\n");
    fprintf(fp, "-module.ID                               core%d:L12LQ\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     pipeline\n");
    fprintf(fp, "-module.pipeline_stages                  1\n");
    fprintf(fp, "-module.per_stage_vector                 %d\n", 2 * GET_KNOB(32_64_ISA));
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }

  if (!GET_KNOB(L2_MEDIUM_BYPASS)) {
    fprintf(fp, "#L2 cache\n");
    fprintf(fp, "-module.ID                               core%d:L2\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(L2_MEDIUM_ASSOC));
    fprintf(fp, "-module.sets                             %d\n", GET_KNOB(L2_MEDIUM_NUM_SET));
    fprintf(fp, "-module.tag_width                        %d\n", 
		        GET_KNOB(PHY_ADDR_WIDTH) + LOG2(GET_KNOB(L2_MEDIUM_LINE_SIZE)) \
						- LOG2(GET_KNOB(L2_MEDIUM_NUM_SET)) + OVERHEAD);
    fprintf(fp, "-module.banks                            %d\n", GET_KNOB(L2_MEDIUM_NUM_BANK));
    fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(L2_READ_PORTS));
    fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(L2_WRITE_PORTS));
    fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L2_MEDIUM_LATENCY));
    fprintf(fp, "-module.cycle_time                       3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                               core%d:L2:linefill\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");	
    fprintf(fp, "-module.sets                             %d\n", 16);
    fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                               core%d:L2:writeback\n", core_id);
    fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
    fprintf(fp, "-module.energy_library                   McPAT\n");
    fprintf(fp, "-module.energy_model                     array\n");
    fprintf(fp, "-module.energy_submodel                  cache\n");
    fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L2_MEDIUM_LINE_SIZE));
    fprintf(fp, "-module.assoc                            0\n");
    fprintf(fp, "-module.sets                             %d\n", 16);
    fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.cycle_time                       8\n");
    fprintf(fp, "-module.rw_ports                         1\n");
    fprintf(fp, "-module.search_ports                     1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                               core%d:exec_bypass\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     interconnect\n");
  fprintf(fp, "-module.data                             %d\n", GET_KNOB(32_64_ISA) + 8);	// 8: tag
  if ((GET_KNOB(MEDIUM_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "-module.connect                          core%d:RS\n", core_id);
  }
  fprintf(fp, "-module.connect                          core%d:payload\n", core_id);
  fprintf(fp, "-module.connect                          core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ROB\n", core_id);
  fprintf(fp, "-module.connect                          core%d:FPU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ALU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:MUL\n", core_id);
  fprintf(fp, "-module.scaling                          %d\n", GET_KNOB(EI_EXEC_WIDTH));	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                               core%d:load_bypass\n", core_id);
  fprintf(fp, "-module.technology                       MEDIUM_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     interconnect\n");
  fprintf(fp, "-module.data                             %d\n", GET_KNOB(32_64_ISA) + 8);	// 8: tag
  if ((GET_KNOB(MEDIUM_CORE_SCHEDULE) == "ooo")) {
    fprintf(fp, "-module.connect                          core%d:RS\n", core_id);
  }
  fprintf(fp, "-module.connect                          core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                          core%d:FPU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ALU\n", core_id);
  fprintf(fp, "-module.connect                          core%d:MUL\n", core_id);
  fprintf(fp, "-module.connect                          core%d:ROB\n", core_id);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");

}


// generate technology parameters for a small core
void ei_power_c::ei_config_gen_small_tech(FILE* fp, int core_id)
{
  fprintf(fp, "# small core technology\n");
  fprintf(fp, "-technology.ID                           SMALL_CORE\n");	
  fprintf(fp, "-technology.clock_frequency              %.3fe9 \n", GET_KNOB(CLOCK_GPU));
  fprintf(fp, "-technology.feature_size                 %de-9\n", GET_KNOB(FEATURE_SIZE));
  fprintf(fp, "-technology.component_type               core\n");
  fprintf(fp, "-technology.core_type                    %s\n", 
	       (GET_KNOB(SCHEDULE) == "ooo")? "ooo" : "inorder");
  fprintf(fp, "-technology.wire_type                    global\n");
  fprintf(fp, "-technology.device_type                  hp\n");
  fprintf(fp, "-technology.interconnect_projection      aggressive\n");
  fprintf(fp, "-technology.wiring_type                  global\n");
  fprintf(fp, "-technology.opt_local                    false\n");
  fprintf(fp, "-technology.embedded                     false\n");
  fprintf(fp, "-technology.longer_channel_device        false\n");
  fprintf(fp, "-technology.temperature                  %d\n", GET_KNOB(TEMPERATURE));
  fprintf(fp, "-technology.end\n");
  fprintf(fp, "\n");
}


// generate module parameters for a small core
void ei_power_c::ei_config_gen_small_mod(FILE* fp, int core_id)
{

  fprintf(fp, "# small core modules\n");

  int mc_i;
  int l3_i;

  int pipeline_total = 3*GET_KNOB(FETCH_LATENCY)+GET_KNOB(ALLOC_LATENCY);

  fprintf(fp, "#undifferentiated core -- fetch partition\n");
  fprintf(fp, "-module.ID                               core%d:fetch\n", core_id);
  fprintf(fp, "-module.technology                       SMALL_CORE   \n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)GET_KNOB(FETCH_LATENCY) / (double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- decode partition\n");
  fprintf(fp, "-module.ID                               core%d:decode\n", core_id);
  fprintf(fp, "-module.technology                       SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)GET_KNOB(ALLOC_LATENCY) / (double)pipeline_total * 0.4 * 1.2);	// 1.2: SIMT overhead
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- renaming partition\n");
  fprintf(fp, "-module.ID                               core%d:renaming\n", core_id);
  fprintf(fp, "-module.technology                       SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                  %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                          %f\n", 
	       (double)GET_KNOB(ALLOC_LATENCY)/(double)pipeline_total * 0.2 * 1.2);	// 1.2: SIMT overhead
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- schedule partition\n");
  fprintf(fp, "-module.ID                              core%d:schedule\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                 %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                         %f\n", 
	       (double)GET_KNOB(ALLOC_LATENCY) / (double)pipeline_total * 0.4);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- memory partition\n");
  fprintf(fp, "-module.ID                              core%d:memory\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                 %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                         %f\n", 
	       (double)GET_KNOB(FETCH_LATENCY) / (double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#undifferentiated core -- execute partition\n");
  fprintf(fp, "-module.ID                              core%d:execute\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    undifferentiated_core\n");
  fprintf(fp, "-module.pipeline_stages                 %d\n", GET_KNOB(ORIG_PIPELINE_STAGES));
  fprintf(fp, "-module.scaling                         %f\n", 
	       (double)GET_KNOB(FETCH_LATENCY) / (double)pipeline_total);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#Block/Warp states\n");
  fprintf(fp, "-module.ID                              core%d:block_states\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      %d\n", 8);	// 8B/warp
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(MAX_WARP_PER_SM));
  fprintf(fp, "-module.rd_ports                        %d\n", 1);
  fprintf(fp, "-module.wr_ports                        %d\n", 1);
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#program counter\n");
  fprintf(fp, "-module.ID                              core%d:program_counter\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                %d\n", 2*GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#fetch queue\n");
  fprintf(fp, "-module.ID                              core%d:byteQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(32_64_ISA) / 2);	
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(WIDTH) * 4);	
  fprintf(fp, "-module.rd_ports                        %d\n", 1);
  fprintf(fp, "-module.wr_ports                        1\n");
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  // FIXME(Jieun, 01-07-2012) - need only one latch between fetch queue and decoder/iQ
  fprintf(fp, "#latch: fetch queue to predecoder\n");	
  fprintf(fp, "-module.ID                              core%d:BQ2PD\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                320\n");
  fprintf(fp, "-module.scaling                         1\n" );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: predecoder to instruction queue\n");
  fprintf(fp, "-module.ID                              core%d:PD2IQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                184\n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction queue\n");
  fprintf(fp, "-module.ID                              core%d:instQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      15\n");
  fprintf(fp, "-module.sets                            %d\n", 
	        (GET_KNOB(WIDTH) * GET_KNOB(ALLOC_LATENCY) + 1) / 2);	// +1: for ceiling
  fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(EI_DECODE_WIDTH));	
  fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(EI_DECODE_WIDTH) + 2);	
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction TLB\n");
  fprintf(fp, "-module.ID                              core%d:ITLB\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.assoc                           %d\n", 4);
  fprintf(fp, "-module.sets                            %d\n", 128);
  fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                        %d\n", 1);
  fprintf(fp, "-module.access_time                     2\n");	 
  fprintf(fp, "-module.cycle_time                      %d\n", 2);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache\n");
  fprintf(fp, "-module.ID                              core%d:ICache\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(ICACHE_LINE_SIZE) );
  fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(ICACHE_ASSOC));
  fprintf(fp, "-module.banks                           %d\n", GET_KNOB(ICACHE_BANKS));
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(ICACHE_NUM_SET));
  fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
	        OVERHEAD - LOG2(GET_KNOB(ICACHE_LINE_SIZE)) - LOG2(GET_KNOB(ICACHE_NUM_SET)));
  fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(ICACHE_READ_PORTS));
  fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(ICACHE_WRITE_PORTS));
  fprintf(fp, "-module.access_time                     2\n");	
  fprintf(fp, "-module.cycle_time                      %d\n", GET_KNOB(ICACHE_CYCLES));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction cache missbuffer\n");
  fprintf(fp, "-module.ID                              core%d:ICache:missbuffer\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(PHY_ADDR_WIDTH) / 8 + \
	        GET_KNOB(ICACHE_LINE_SIZE));
  fprintf(fp, "-module.assoc                           0\n");
  fprintf(fp, "-module.sets                            16\n");
  fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                        1\n");
  fprintf(fp, "-module.search_ports                    1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction queue to decoder\n");
  fprintf(fp, "-module.ID                              core%d:IQ2ID\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                184\n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction decoder\n");
  fprintf(fp, "-module.ID                              core%d:instruction_decoder\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    instruction_decoder\n");
  fprintf(fp, "-module.x86                             false\n");
  fprintf(fp, "-module.opcode                          16\n");
  fprintf(fp, "-module.area_scaling                    %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: instruction decoder to uopQ\n");
  fprintf(fp, "-module.ID                              core%d:ID2uQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                32\n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(GPU_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#uop queue\n");
  fprintf(fp, "-module.ID                              core%d:uopQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      5\n" );
  fprintf(fp, "-module.sets                            %d\n", 7 * GET_KNOB(WIDTH));
  fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: uopQ to register renaming\n");
  fprintf(fp, "-module.ID                              core%d:uQ2RR\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                32 \n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#register renaming table\n");
  fprintf(fp, "-module.ID                              core%d:RAT\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d \n", 
	        (int)(LOG2(GET_KNOB(ROB_SIZE)) * (1 + 16) / 8));  // 16: checkpoints
  fprintf(fp, "-module.sets                            32\n");
  fprintf(fp, "-module.rw_ports                        %d\n", 1);
  fprintf(fp, "-module.rd_ports                        %d \n", 2 * GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                        %d \n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.search_ports                    %d \n", GET_KNOB(EI_COMMIT_WIDTH));
  fprintf(fp, "-module.access_mode                     fast\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#dependency check logic\n");
  fprintf(fp, "-module.ID                              core%d:operand_dependency\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    dependency_check_logic\n");
  fprintf(fp, "-module.compare_bits                    %d\n", LOG2(GET_KNOB(ROB_SIZE)));
  fprintf(fp, "-module.decode_width                    %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.scaling                         2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#freelist\n");
  fprintf(fp, "-module.ID                              core%d:freelist\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      %d\n", 1);
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(ROB_SIZE));
  fprintf(fp, "-module.rw_ports                        %d\n", 1);
  fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(EI_DECODE_WIDTH));
  fprintf(fp, "-module.wr_ports                        %d\n", 2 * GET_KNOB(EI_COMMIT_WIDTH) - 1);
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#register file\n");
  fprintf(fp, "-module.ID                              core%d:RF\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 ram\n");
  fprintf(fp, "-module.line_width                      %d\n", 128 / 8);
  fprintf(fp, "-module.size                            %d\n", GET_KNOB(INT_REGFILE_SIZE) * 8);
  fprintf(fp, "-module.banks                           %d\n", 32);
  fprintf(fp, "-module.rd_ports                        %d\n", 2);
  fprintf(fp, "-module.wr_ports                        %d\n", 1);	
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "#latch: register renaming to reservation station\n");
  fprintf(fp, "-module.ID                              core%d:RR2RS\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                40\n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: architecture registers to reservation station\n");
  fprintf(fp, "-module.ID                              core%d:ARF2RS\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                64\n");
  fprintf(fp, "-module.scaling                         %d\n", 2 * GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#scoreboarding\n");
  fprintf(fp, "-module.ID                              core%d:SB\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.assoc                           0\n");		
  fprintf(fp, "-module.line_width                      20 \n");
  fprintf(fp, "-module.tag_width                       12 \n");
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(ISCHED_SIZE) + \
	        GET_KNOB(MSCHED_SIZE) + GET_KNOB(FSCHED_SIZE));
  fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(ISCHED_RATE) + \
	        GET_KNOB(MSCHED_RATE) + GET_KNOB(FSCHED_RATE));
  fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(ISCHED_RATE) + \
	        GET_KNOB(MSCHED_RATE) + GET_KNOB(FSCHED_RATE));
  fprintf(fp, "-module.search_ports                    %d\n", GET_KNOB(EI_ISSUE_WIDTH));
  fprintf(fp, "-module.access_time                     2\n");	 
  fprintf(fp, "-module.cycle_time                      2\n");
  fprintf(fp, "-module.scaling                         2\n");	// dual scheduler
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#instruction issue selection logic\n");
  fprintf(fp, "-module.ID                              core%d:issue_select\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    selection_logic\n");
  fprintf(fp, "-module.selection_input                 %d\n", GET_KNOB(ISCHED_SIZE) + \
	        GET_KNOB(MSCHED_SIZE) + GET_KNOB(FSCHED_SIZE));
  fprintf(fp, "-module.selection_output                %d\n", GET_KNOB(ISCHED_RATE) + \
	        GET_KNOB(MSCHED_RATE) + GET_KNOB(FSCHED_RATE));
  fprintf(fp, "-module.scaling                         2\n");	// dual scheduler
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: payload RAM\n");
  fprintf(fp, "-module.ID                              core%d:payload\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                151\n");
  fprintf(fp, "-module.scaling                         %d \n", GET_KNOB(EI_EXEC_WIDTH) );
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - SP - alu\n");
  fprintf(fp, "-module.ID                              core%d:SP_alu\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    functional_unit\n");
  fprintf(fp, "-module.energy_submodel                 alu\n");
  fprintf(fp, "-module.scaling                         32\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - SP - fpu\n");
  fprintf(fp, "-module.ID                              core%d:SP_fpu\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    functional_unit\n");
  fprintf(fp, "-module.energy_submodel                 fpu\n");
  fprintf(fp, "-module.scaling                         32\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - SFU\n");
  fprintf(fp, "-module.ID                              core%d:SFU\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    functional_unit\n");
  fprintf(fp, "-module.energy_submodel                 fpu\n");
  fprintf(fp, "-module.scaling                         4\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#execution unit - Ld/St unit\n");
  fprintf(fp, "-module.ID                              core%d:ldst\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    functional_unit\n");
  fprintf(fp, "-module.energy_submodel                 alu\n");
  fprintf(fp, "-module.scaling                         16\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: functional unit to ROB\n");
  fprintf(fp, "-module.ID                              core%d:FU2ROB\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                87\n");
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_EXEC_WIDTH));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#load queue\n");
  fprintf(fp, "-module.ID                              core%d:loadQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.assoc                           0\n");		
  fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(32_64_ISA) + OVERHEAD + 16);  // 16: opcode
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(LOAD_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                        %d\n", 2);
  fprintf(fp, "-module.wr_ports                        %d\n", 2);
  fprintf(fp, "-module.search_ports                    %d\n", 2);
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.area_scaling                    1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to ROB\n");
  fprintf(fp, "-module.ID                              core%d:LQ2ROB\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                %d\n", 2 * GET_KNOB(32_64_ISA) + 6);
  fprintf(fp, "-module.scaling                         2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: load queue to DCache\n");
  fprintf(fp, "-module.ID                              core%d:LQ2L1\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#store queue\n");
  fprintf(fp, "-module.ID                              core%d:storeQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d\n", 32 / 8);
  fprintf(fp, "-module.assoc                           0\n");	
  fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(32_64_ISA) + OVERHEAD + 16);
  fprintf(fp, "-module.sets                            %d\n", GET_KNOB(STORE_BUF_SIZE));
  fprintf(fp, "-module.rd_ports                        %d\n", 1);
  fprintf(fp, "-module.wr_ports                        %d\n", 1);
  fprintf(fp, "-module.search_ports                    %d\n", 1);
  fprintf(fp, "-module.access_mode                     sequential\n");
  fprintf(fp, "-module.area_scaling                    1.1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to DCache\n");
  fprintf(fp, "-module.ID                              core%d:SQ2L1\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#latch: store queue to load queue\n");
  fprintf(fp, "-module.ID                              core%d:SQ2LQ\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    pipeline\n");
  fprintf(fp, "-module.pipeline_stages                 1\n");
  fprintf(fp, "-module.per_stage_vector                %d\n", 2 * GET_KNOB(32_64_ISA));
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#data TLB\n");
  fprintf(fp, "-module.ID                              core%d:DTLB\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    array\n");
  fprintf(fp, "-module.energy_submodel                 cache\n");
  fprintf(fp, "-module.line_width                      %d \n", GET_KNOB(32_64_ISA) / 8);
  fprintf(fp, "-module.assoc                           4\n");
  fprintf(fp, "-module.sets                            128\n");
  fprintf(fp, "-module.tag_width                       %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.rw_ports                        2\n");
  fprintf(fp, "-module.search_ports                    2\n");
  fprintf(fp, "-module.access_time                     2\n");
  fprintf(fp, "-module.cycle_time                      2\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");

  if (!GET_KNOB(L1_SMALL_BYPASS)) {
    fprintf(fp, "#data cache\n");
    fprintf(fp, "-module.ID                              core%d:DCache\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(L1_SMALL_ASSOC));
    fprintf(fp, "-module.banks                           %d\n", GET_KNOB(L1_SMALL_NUM_BANK));
    fprintf(fp, "-module.sets                            %d\n", GET_KNOB(L1_SMALL_NUM_SET));
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
		        OVERHEAD - LOG2(GET_KNOB(L1_SMALL_LINE_SIZE)) - LOG2(GET_KNOB(L1_SMALL_NUM_SET)));
    fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(L1_WRITE_PORTS));
    fprintf(fp, "-module.access_time                     %d\n", GET_KNOB(L1_SMALL_LATENCY));
    fprintf(fp, "-module.cycle_time	                     3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache missbuffer\n");
    fprintf(fp, "-module.ID                              core%d:DCache:missbuffer\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(PHY_ADDR_WIDTH) / 8 + \
		        GET_KNOB(L1_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           0\n");	
    fprintf(fp, "-module.sets                            %d\n", GET_KNOB(MEM_MSHR_SIZE) / 4);
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH)+OVERHEAD);
    fprintf(fp, "-module.rw_ports                        1\n");
    fprintf(fp, "-module.search_ports                    1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                              core%d:DCache:linefill\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           0\n");
    fprintf(fp, "-module.sets                            16\n");
    fprintf(fp, "-module.tag_width                       %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                        1\n");
    fprintf(fp, "-module.search_ports                    2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                              core%d:DCache:writeback\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           0\n");	
    fprintf(fp, "-module.sets                            16\n");
    fprintf(fp, "-module.tag_width                       %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                        1\n");
    fprintf(fp, "-module.search_ports                    1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#latch: DCache to load queue\n");
    fprintf(fp, "-module.ID                              core%d:L12LQ\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    pipeline\n");
    fprintf(fp, "-module.pipeline_stages                 1\n");
    fprintf(fp, "-module.per_stage_vector                %d\n", 2 * GET_KNOB(32_64_ISA));
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }

  if (!GET_KNOB(L2_SMALL_BYPASS)) {
    fprintf(fp, "#L2 cache\n");
    fprintf(fp, "-module.ID                              core%d:L2\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L2_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(L2_SMALL_ASSOC));
    fprintf(fp, "-module.sets                            %d\n", GET_KNOB(L2_SMALL_NUM_SET));
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
		        LOG2(GET_KNOB(L2_SMALL_LINE_SIZE)) - LOG2(GET_KNOB(L2_SMALL_NUM_SET)) + OVERHEAD);
    fprintf(fp, "-module.banks                           %d\n", GET_KNOB(L2_SMALL_NUM_BANK));
    fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(L2_READ_PORTS));
    fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(L2_WRITE_PORTS));
    fprintf(fp, "-module.access_time                     %d\n", GET_KNOB(L2_SMALL_LATENCY));
    fprintf(fp, "-module.cycle_time                      3\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache linefill buffer\n");
    fprintf(fp, "-module.ID                              core%d:L2:linefill\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L2_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           0\n");	
    fprintf(fp, "-module.sets                            %d\n", 16);
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.rw_ports                        1\n");
    fprintf(fp, "-module.search_ports                    1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#data cache writeback buffer\n");
    fprintf(fp, "-module.ID                              core%d:L2:writeback\n", core_id);
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(L2_SMALL_LINE_SIZE));
    fprintf(fp, "-module.assoc                           0\n");	
    fprintf(fp, "-module.sets                            %d\n", 16);
    fprintf(fp, "-module.tag_width                       %d \n", GET_KNOB(PHY_ADDR_WIDTH) + OVERHEAD);
    fprintf(fp, "-module.cycle_time                      8\n");
    fprintf(fp, "-module.rw_ports                        1\n");
    fprintf(fp, "-module.search_ports                    1\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
  }
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                              core%d:exec_bypass\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    interconnect\n");
  fprintf(fp, "-module.data                            %d\n", GET_KNOB(32_64_ISA) + 8); // 8: tag
  fprintf(fp, "-module.connect                         core%d:SB\n", core_id);
  fprintf(fp, "-module.connect                         core%d:payload\n", core_id);
  fprintf(fp, "-module.connect                         core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                         core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SP_alu\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SP_fpu\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SFU\n", core_id);
  fprintf(fp, "-module.connect                         core%d:ldst\n", core_id);
  fprintf(fp, "-module.scaling                         %d\n", GET_KNOB(EI_EXEC_WIDTH));	
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#bypass interconnect\n");
  fprintf(fp, "-module.ID                              core%d:load_bypass\n", core_id);
  fprintf(fp, "-module.technology                      SMALL_CORE\n");
  fprintf(fp, "-module.energy_library                  McPAT\n");
  fprintf(fp, "-module.energy_model                    interconnect\n");
  fprintf(fp, "-module.data                            %d\n", GET_KNOB(32_64_ISA) + 8); // 8: tag
  fprintf(fp, "-module.connect                         core%d:SB\n", core_id);
  fprintf(fp, "-module.connect                         core%d:loadQ\n", core_id);
  fprintf(fp, "-module.connect                         core%d:storeQ\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SP_alu\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SP_fpu\n", core_id);
  fprintf(fp, "-module.connect                         core%d:SFU\n", core_id);
  fprintf(fp, "-module.connect                         core%d:ldst\n", core_id);
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");

  if (GET_KNOB(CORE_TYPE) == "ptx") {
    fprintf(fp, "#shared memory\n");
    fprintf(fp, "-module.ID                              core%d:SharedMem\n", core_id );
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(SHARED_MEM_LINE_SIZE));
    fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(SHARED_MEM_ASSOC));
    fprintf(fp, "-module.banks                           %d\n", GET_KNOB(SHARED_MEM_BANKS));
    fprintf(fp, "-module.size                            %d\n", GET_KNOB(SHARED_MEM_SIZE));
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
		        OVERHEAD - LOG2(GET_KNOB(SHARED_MEM_LINE_SIZE)) - LOG2(GET_KNOB(SHARED_MEM_SIZE) / \
						GET_KNOB(SHARED_MEM_LINE_SIZE) / GET_KNOB(SHARED_MEM_ASSOC)));
    fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.wr_ports                        %d\n", GET_KNOB(L1_WRITE_PORTS));
    fprintf(fp, "-module.access_time                     %d\n", GET_KNOB(L1_SMALL_LATENCY));
    fprintf(fp, "-module.cycle_time                      2\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#const cache\n");
    fprintf(fp, "-module.ID                              core%d:ConstCache\n", core_id );
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(CONST_CACHE_LINE_SIZE));
    fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(CONST_CACHE_ASSOC));
    fprintf(fp, "-module.banks                           %d\n", GET_KNOB(CONST_CACHE_BANKS));
    fprintf(fp, "-module.sets                            %d\n", GET_KNOB(CONST_CACHE_SIZE) / \
		        GET_KNOB(CONST_CACHE_LINE_SIZE) / GET_KNOB(CONST_CACHE_ASSOC));
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
		        OVERHEAD - LOG2(GET_KNOB(CONST_CACHE_LINE_SIZE)) - LOG2(GET_KNOB(CONST_CACHE_SIZE) / \
						GET_KNOB(CONST_CACHE_LINE_SIZE) / GET_KNOB(CONST_CACHE_ASSOC)));
    fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.access_time                     %d\n", GET_KNOB(L3_LATENCY));
    fprintf(fp, "-module.cycle_time                      10\n");
    fprintf(fp, "-module.end\n");
    fprintf(fp, "\n");
    fprintf(fp, "#texture cache\n");
    fprintf(fp, "-module.ID                              core%d:TextureCache\n", core_id );
    fprintf(fp, "-module.technology                      SMALL_CORE\n");
    fprintf(fp, "-module.energy_library                  McPAT\n");
    fprintf(fp, "-module.energy_model                    array\n");
    fprintf(fp, "-module.energy_submodel                 cache\n");
    fprintf(fp, "-module.line_width                      %d\n", GET_KNOB(TEXTURE_CACHE_LINE_SIZE));
    fprintf(fp, "-module.assoc                           %d\n", GET_KNOB(TEXTURE_CACHE_ASSOC));
    fprintf(fp, "-module.banks                           %d\n", GET_KNOB(TEXTURE_CACHE_BANKS));
    fprintf(fp, "-module.sets                            %d\n", GET_KNOB(TEXTURE_CACHE_SIZE) / \
		        GET_KNOB(TEXTURE_CACHE_LINE_SIZE) / GET_KNOB(TEXTURE_CACHE_ASSOC));
    fprintf(fp, "-module.tag_width                       %d\n", GET_KNOB(PHY_ADDR_WIDTH) + \
		        OVERHEAD - LOG2(GET_KNOB(TEXTURE_CACHE_LINE_SIZE)) - LOG2(GET_KNOB(TEXTURE_CACHE_SIZE) / \
						GET_KNOB(TEXTURE_CACHE_LINE_SIZE) / GET_KNOB(TEXTURE_CACHE_ASSOC)));
    fprintf(fp, "-module.rd_ports                        %d\n", GET_KNOB(L1_READ_PORTS));
    fprintf(fp, "-module.access_time                     %d\n", GET_KNOB(L3_LATENCY));
    fprintf(fp, "-module.cycle_time                      10\n");
    fprintf(fp, "-module.end\n");
  }
}


// generate technology parameters for a last level cache 
void ei_power_c::ei_config_gen_llc_tech(FILE* fp)
{
  fprintf(fp, "# llc technology\n");
  fprintf(fp, "-technology.ID                           L3\n");	
  fprintf(fp, "-technology.clock_frequency              %.3e6 \n", 3206.0);	
  fprintf(fp, "-technology.feature_size                 %de-9\n", GET_KNOB(FEATURE_SIZE));
  fprintf(fp, "-technology.component_type               llc\n");
  fprintf(fp, "-technology.wire_type                    global\n");
  fprintf(fp, "-technology.device_type                  hp\n");
  fprintf(fp, "-technology.interconnect_projection      aggressive\n");
  fprintf(fp, "-technology.wiring_type                  global\n");
  fprintf(fp, "-technology.embedded                     false\n");
  fprintf(fp, "-technology.longer_channel_device        false\n");
  fprintf(fp, "-technology.temperature                  %d\n", GET_KNOB(TEMPERATURE));
  fprintf(fp, "-technology.end\n");
  fprintf(fp, "\n");
}


// generate technology parameters for a memory controller 
void ei_power_c::ei_config_gen_mc_tech(FILE* fp )
{
  fprintf(fp, "# llc technology\n");
  fprintf(fp, "-technology.ID                           MemoryController\n");	
  fprintf(fp, "-technology.clock_frequency              %.3fe9 \n", GET_KNOB(CLOCK_CPU));
  fprintf(fp, "-technology.feature_size                 %de-9\n", GET_KNOB(FEATURE_SIZE));
  fprintf(fp, "-technology.component_type               uncore\n");
  fprintf(fp, "-technology.wire_type                    global\n");
  fprintf(fp, "-technology.device_type                  hp\n");
  fprintf(fp, "-technology.interconnect_projection      aggressive\n");
  fprintf(fp, "-technology.wiring_type                  global\n");
  fprintf(fp, "-technology.embedded                     false\n");	
  fprintf(fp, "-technology.longer_channel_device        false\n");
  fprintf(fp, "-technology.temperature                  %d\n", GET_KNOB(TEMPERATURE));
  fprintf(fp, "-technology.end\n");
  fprintf(fp, "\n");
}


// generate module parameters for a last level cache
void ei_power_c::ei_config_gen_llc_mod(FILE* fp, int l3_i)
{
  fprintf(fp, "#data cache L%d\n", l3_i);
  fprintf(fp, "-module.ID                               L3:%d\n", l3_i);
  fprintf(fp, "-module.technology                       L3\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L3_LINE_SIZE));
  fprintf(fp, "-module.assoc                            %d\n", GET_KNOB(L3_ASSOC));
  fprintf(fp, "-module.banks                            %d\n", GET_KNOB(L3_NUM_BANK));
  fprintf(fp, "-module.sets                             %d\n", GET_KNOB(L3_NUM_SET));
  fprintf(fp, "-module.tag_width                        %d\n", GET_KNOB(PHY_ADDR_WIDTH) \
	        + LOG2(GET_KNOB(L3_LINE_SIZE)) - LOG2(GET_KNOB(L3_NUM_SET)) + OVERHEAD);
  fprintf(fp, "-module.rd_ports                         %d\n", GET_KNOB(L3_READ_PORTS));
  fprintf(fp, "-module.wr_ports                         %d\n", GET_KNOB(L3_WRITE_PORTS));
  fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L3_LATENCY));
  fprintf(fp, "-module.cycle_time                       16\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#data cache linefill buffer\n");
  fprintf(fp, "-module.ID                               L3:linefill:%d\n", l3_i );
  fprintf(fp, "-module.technology                       L3\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L3_LINE_SIZE));
  fprintf(fp, "-module.assoc                            0\n");	
  fprintf(fp, "-module.sets                             %d\n", 16);
  fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.cycle_time                       8\n");
  fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L3_LATENCY));
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.search_ports                     1\n");
  fprintf(fp, "-module.end\n");
  fprintf(fp, "\n");
  fprintf(fp, "#data cache writeback buffer\n");
  fprintf(fp, "-module.ID                               L3:writeback:%d\n", l3_i);
  fprintf(fp, "-module.technology                       L3\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     array\n");
  fprintf(fp, "-module.energy_submodel                  cache\n");
  fprintf(fp, "-module.line_width                       %d\n", GET_KNOB(L3_LINE_SIZE));
  fprintf(fp, "-module.assoc                            0\n");	
  fprintf(fp, "-module.sets                             %d\n", 16);
  fprintf(fp, "-module.tag_width                        %d \n", GET_KNOB(32_64_ISA) + OVERHEAD);
  fprintf(fp, "-module.cycle_time                       8\n");
  fprintf(fp, "-module.access_time                      %d\n", GET_KNOB(L3_LATENCY));
  fprintf(fp, "-module.rw_ports                         1\n");
  fprintf(fp, "-module.search_ports                     1\n");
  fprintf(fp, "-module.end\n");
}


// generate module parameters for a memory controller
void ei_power_c::ei_config_gen_mc_mod(FILE* fp, int mc_i)
{
  fprintf(fp, "#memory controller #%d\n", mc_i);
  fprintf(fp, "-module.ID                               MemCon:%d\n", mc_i);
  fprintf(fp, "-module.technology                       MemoryController\n");
  fprintf(fp, "-module.energy_library                   McPAT\n");
  fprintf(fp, "-module.energy_model                     memory_controller\n");
  fprintf(fp, "-module.energy_submodel                  memory_controller\n");
  fprintf(fp, "-module.line                             64\n");	// xml_interface.sys.mc.llc_line_length
  fprintf(fp, "-module.request_window                   32\n");	// req_window_size_per_channel
  fprintf(fp, "-module.io_buffer                        32\n");	// IO_buffer_size_per_channer
  fprintf(fp, "-module.memory_channels                  1 \n");	// memory_channers_per_mc
  fprintf(fp, "-module.transfer_rate                    3200\n");	// peak_transfer_rate
  fprintf(fp, "-module.ranks                            2\n");	// number_ranks
  fprintf(fp, "-module.data_bus                         128\n");	// databus_width
  fprintf(fp, "-module.address_bus                      51\n");	// addressbus_width
  fprintf(fp, "-module.lvds                             true\n");	// LVDS
  fprintf(fp, "-module.with_phy                         false\n");	// withPHY
  fprintf(fp, "-module.model                            high_performance\n");
  fprintf(fp, "-module.end\n");
}


// top function to generate an input parameter file according to core type and module type 
void ei_power_c::ei_config_gen_top()
{

  FILE* fp = fopen(CONFIG_FILE_NAME, "w");

  int i;

  int num_large_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
  int num_medium_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
  int num_small_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;
  int num_l3 = *m_simBase->m_knobs->KNOB_NUM_L3;
  int num_mc = *m_simBase->m_knobs->KNOB_DRAM_NUM_MC;


  // generate technology parameters
  if (num_large_cores > 0) {
    ei_config_gen_large_tech(fp, i);
  }
  if (num_medium_cores > 0) {
    ei_config_gen_medium_tech(fp, i);
  }
  if (num_small_cores > 0) {
    ei_config_gen_small_tech(fp, i);
  }
  if (num_l3 > 0) {
    ei_config_gen_llc_tech(fp);
  }
  if (num_mc > 0) {
    ei_config_gen_mc_tech(fp);
  }

  // generate module parameters
  for (i = 0; i < num_large_cores; ++i) {
    ei_config_gen_large_mod(fp, i);
  }
  for (i = 0; i < num_medium_cores; ++i) {
    ei_config_gen_medium_mod(fp, i+num_large_cores);
  }
  for (i = 0; i < num_small_cores; ++i) {
    ei_config_gen_small_mod(fp, i+num_large_cores+num_medium_cores );
  }
  for (i = 0; i < num_l3; ++i) {
    ei_config_gen_llc_mod(fp, i);
  }
  for (i = 0; i < num_mc; ++i) {
    ei_config_gen_mc_mod(fp, i);
  }

  fclose(fp);
}


// macro to compute power, need two counters, per-core 
#define COMPUTE_POWER(module_name, num_read, num_write, indent ) do {\
  counters.reset(); \
  counters.read=num_read; \
  counters.write=num_write; \
  energy_introspector->compute_power(current_time, period, get_name(module_name, core_id), counters); \
  area = energy_introspector->pull_data<dimension_t>(current_time,"module", get_name(module_name, core_id),"dimension").area; \
  power = energy_introspector->pull_data<power_t>(current_time, "module", get_name(module_name, core_id), "power"); \
  area_stage = area_stage + area; \
  area_total = area_total + area; \
  power_block = power_block + power; \
  power_stage = power_stage + power; \
} while (0) \


// macro to compute cache power, need five counters, per-core
#define COMPUTE_POWER_CACHE(module_name, num_read, num_write, num_read_tag, num_write_tag, num_search, indent ) do {\
  counters.reset(); \
  counters.read=num_read; \
  counters.write=num_write; \
  counters.read_tag=num_read_tag; \
  counters.write_tag=num_write_tag; \
  counters.search=num_search; \
  energy_introspector->compute_power(current_time, period, get_name(module_name, core_id), counters); \
  area = energy_introspector->pull_data<dimension_t>(current_time,"module", get_name(module_name, core_id),"dimension").area; \
  power = energy_introspector->pull_data<power_t>(current_time, "module", get_name(module_name, core_id), "power"); \
  area_stage = area_stage + area; \
  area_total = area_total + area; \
  power_block = power_block + power; \
  power_stage = power_stage + power; \
} while (0) \


// macro to compute llc power, need five counters, per-chip
#define COMPUTE_POWER_LLC(module_name, num_read, num_write, num_read_tag, num_write_tag, num_search, indent ) do {\
  counters.reset(); \
  counters.read=num_read; \
  counters.write=num_write; \
  counters.read_tag=num_read_tag; \
  counters.write_tag=num_write_tag; \
  counters.search=num_search; \
  energy_introspector->compute_power(current_time, period, module_name, counters); \
  area = energy_introspector->pull_data<dimension_t>(current_time,"module", module_name,"dimension").area; \
  power = energy_introspector->pull_data<power_t>(current_time, "module", module_name, "power"); \
  area_stage = area_stage + area; \
  area_total = area_total + area; \
  power_block = power_block + power; \
  power_stage = power_stage + power; \
} while (0) \


// macro to compute mc power, need two counters, per-chip
#define COMPUTE_POWER_MC(module_name, num_read, num_write, indent ) do {\
  counters.reset(); \
  counters.read=num_read; \
  counters.write=num_write; \
  energy_introspector->compute_power(current_time, period, module_name, counters); \
  area = energy_introspector->pull_data<dimension_t>(current_time,"module", module_name,"dimension").area; \
  power = energy_introspector->pull_data<power_t>(current_time, "module", module_name, "power"); \
  area_stage = area_stage + area; \
  area_total = area_total + area; \
  power_block = power_block + power; \
  power_stage = power_stage + power; \
} while (0) \


// macro to show results at coarse-grain level, consists of a couple of modules
#define COMPUTE_POWER_BLOCK(block_name) do { \
  power_block.reset(); \
} while (0) \


#define COMPUTE_POWER_STAGE(stage_name, scaling, area_stage_total, power_stage_total) do { \
  power_stage = power_stage*(double)scaling; \
  power_stage.leakage = power_stage.leakage / (double)scaling; \
  power_stage.total = power_stage.total - power_stage.leakage*(scaling-1); \
  power_stage_total = power_stage_total + power_stage; \
  power_core = power_core + power_stage; \
  power_total = power_total + power_stage; \
  area_stage_total = area_stage_total + area_stage*1e6; \
  fprintf(fp, "%s;", stage_name); \
  fprintf(fp, "%f;%f;%f;%f;%f\n", area_stage*1e6, power_stage.total - power_stage.leakage, power_stage.leakage, power_stage.total, power_stage.total/area_stage/1e6); \
  area_stage = 0; \
  power_stage.reset(); \
} while (0) \


// concatenate strings to make a module name 
string ei_power_c::get_name(string module_name, int core_id)
{
  string str = "core";
  std::stringstream id;
  id << core_id;
  str.append(id.str());
  str.append(":");
  str.append(module_name);

  return str.data();
}


// main function to compute power
void ei_power_c::ei_main()
{
	// Power results will be printed in this file.
	// STAT file format are not used yet
  FILE* fp = fopen("power_result.stat.out", "w");

	// call energy_introspector for parsing
  energy_introspector_t *energy_introspector = new energy_introspector_t(CONFIG_FILE_NAME);

  power_t power;
  power_t power_block;
  power_t power_stage;
  power_t power_core;
  power_t power_total;
  power_t pow_stage_fet;
  power_t pow_stage_dec;
  power_t pow_stage_ren;
  power_t pow_stage_sch;
  power_t pow_stage_rf;
  power_t pow_stage_ex;
  power_t pow_stage_mmu;
  power_t pow_stage_smem;
  power_t pow_stage_cmem;
  power_t pow_stage_tmem;
  power_t pow_stage_l1;
  power_t pow_stage_l2;
  power_t pow_stage_l3;
  power_t pow_stage_mc;
  power_t pow_stage_dram;
  
  double area = 0.0;
  double area_stage = 0.0;
  double area_total = 0.0;
  double area_stage_fet = 0.0;
  double area_stage_dec = 0.0;
  double area_stage_ren = 0.0;
  double area_stage_sch = 0.0;
  double area_stage_rf = 0.0;
  double area_stage_ex = 0.0;
  double area_stage_mmu = 0.0;
  double area_stage_smem = 0.0;
  double area_stage_cmem = 0.0;
  double area_stage_tmem = 0.0;
  double area_stage_l1 = 0.0;
  double area_stage_l2 = 0.0;
  double area_stage_l3 = 0.0;
  double area_stage_mc = 0.0;
  double area_stage_dram = 0.0;

  double period = 0.0;
	double current_time = 0.0;
 
  Core_Type core_type;
  ISA_Type isa_type;
	Schedule_Type schedule_type;

	bool l1_bypass;
	bool l2_bypass;

  int core_id;
  int l3_id;
  int mc_i;

  counters_t counters;

  int num_large_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
  int num_medium_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
  int num_small_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;

	// to show results in a hierarchical fashion - not implemented yet
  string level(2, ' ');

  power.reset();
  power_block.reset();
  power_stage.reset();
  power_total.reset();

  pow_stage_fet.reset();
  pow_stage_dec.reset();
  pow_stage_ren.reset();
  pow_stage_sch.reset();
  pow_stage_rf.reset();
  pow_stage_ex.reset();
  pow_stage_mmu.reset();
  pow_stage_l1.reset();
  pow_stage_smem.reset();
  pow_stage_cmem.reset();
  pow_stage_tmem.reset();
  pow_stage_l2.reset();
  pow_stage_l3.reset();
  pow_stage_mc.reset();
  pow_stage_dram.reset();


  // compute power for each core 
  for (core_id = 0; core_id < *m_simBase->m_knobs->KNOB_NUM_SIM_CORES; ++core_id) {
    area = 0.0;
		area_stage = 0.0;
    power.reset();
    power_block.reset();
    power_stage.reset();
    power_core.reset();

    // set a couple of types
    if (core_id < num_large_cores) {
      core_type = LARGE_CORE;
      isa_type = (GET_KNOB(LARGE_CORE_TYPE)=="x86")?X86:PTX;
      schedule_type = (GET_KNOB(LARGE_CORE_SCHEDULE)=="ooo")?OUT_OF_ORDER:IN_ORDER;
      l1_bypass = GET_KNOB(L1_LARGE_BYPASS);
      l2_bypass = GET_KNOB(L2_LARGE_BYPASS);
    }
    else if (core_id < num_large_cores + num_medium_cores) {
      core_type = MEDIUM_CORE;
      isa_type = (GET_KNOB(MEDIUM_CORE_TYPE)=="x86")?X86:PTX;
      schedule_type = (GET_KNOB(MEDIUM_CORE_SCHEDULE)=="ooo")?OUT_OF_ORDER:IN_ORDER;
      l1_bypass = GET_KNOB(L1_MEDIUM_BYPASS);
      l2_bypass = GET_KNOB(L2_MEDIUM_BYPASS);
    }
    else {
      core_type = SMALL_CORE;
      isa_type = (GET_KNOB(CORE_TYPE)=="x86")?X86:PTX;
      schedule_type = (GET_KNOB(SCHEDULE)=="ooo")?OUT_OF_ORDER:IN_ORDER;
      l1_bypass = GET_KNOB(L1_SMALL_BYPASS);
      l2_bypass = GET_KNOB(L2_SMALL_BYPASS);
    }

    // print IPC and MPKI
    if (core_id == 0) {
      fprintf(fp, "\nIPC; ; ; ;%f\n", (double)GET_STAT(INST_COUNT_TOT)/(double)GET_STAT(CYC_COUNT_TOT));
      if (isa_type == X86) {
        fprintf(fp, "MPKI_CPU; ; ; ;%f\n\n", (double)GET_STAT(L3_MISS_CPU)/((double)GET_STAT(INST_COUNT_TOT)/1000.0));
      }
      else {
        fprintf(fp, "MPKI_GPU; ; ; ;%f\n\n", (double)GET_STAT(L3_MISS_GPU)/((double)GET_STAT(INST_COUNT_TOT)/1000.0));
      }
    }

    // set current_time and period 
    if (isa_type == X86) {
      current_time = (double)GET_CORE_STAT(core_id, CYC_COUNT)/GET_KNOB(CLOCK_CPU)/1e9; 
      period = (double)GET_CORE_STAT(core_id, CYC_COUNT)/GET_KNOB(CLOCK_CPU)/1e9; 
    }
    else if (isa_type == PTX) {
      current_time = (double)(GET_CORE_STAT(core_id, CYC_COUNT))/GET_KNOB(CLOCK_GPU)/1e9; 
      period = (double)(GET_CORE_STAT(core_id, CYC_COUNT))/GET_KNOB(CLOCK_GPU)/1e9; 

      if (GET_CORE_STAT(core_id, CYC_COUNT) == 0) {
        period = 1.0/GET_KNOB(CLOCK_GPU)/1e9;
      }
    }


    ////////////////////////////////////////////////////////////////////////////////////
    // Fetch Stage	
    ////////////////////////////////////////////////////////////////////////////////////
    if (isa_type == PTX) {
      COMPUTE_POWER("block_states", 
			    GET_CORE_STAT(core_id, POWER_BLOCK_STATES_R), 
			    GET_CORE_STAT(core_id, POWER_BLOCK_STATES_W), level);
    }
    COMPUTE_POWER("program_counter", 
         GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), 
         0, level);
    COMPUTE_POWER("byteQ", 
         GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), 
         GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_W), level);
    COMPUTE_POWER("BQ2PD", 
         GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), 
         0, level);
    COMPUTE_POWER("PD2IQ", 
         GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), 
         0, level);
    COMPUTE_POWER_CACHE("ICache", 
         GET_CORE_STAT(core_id, POWER_ICACHE_R), 
         GET_CORE_STAT(core_id, POWER_ICACHE_W), 
         GET_CORE_STAT(core_id, POWER_ICACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_ICACHE_W), 0, level);
    COMPUTE_POWER_CACHE("ICache:missbuffer", 
         GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_R), 
         GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_W), 
         GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_W), 
         GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_R_TAG), level);

    if (isa_type == X86) {
      COMPUTE_POWER("bpred1", 
         GET_CORE_STAT(core_id, POWER_BR_PRED_R), 
         GET_CORE_STAT(core_id, POWER_BR_PRED_W), level);
      COMPUTE_POWER("BTB", 
         GET_CORE_STAT(core_id, POWER_BR_PRED_R), 
         GET_CORE_STAT(core_id, POWER_BR_PRED_W), level);
      COMPUTE_POWER("RAS", GET_CORE_STAT(core_id, POWER_RAS_R), 
         GET_CORE_STAT(core_id, POWER_RAS_W), level);
    }

    COMPUTE_POWER("instQ", 
         GET_CORE_STAT(core_id, POWER_INST_QUEUE_R), 
         GET_CORE_STAT(core_id, POWER_INST_QUEUE_W), level);
    COMPUTE_POWER("IQ2ID", 
         GET_CORE_STAT(core_id, POWER_INST_QUEUE_R), 0, level);
    COMPUTE_POWER("fetch", 0, 0, level);
    
		COMPUTE_POWER_STAGE("Instruction Fetch Unit", 1, area_stage_fet, pow_stage_fet);


    ////////////////////////////////////////////////////////////////////////////////////
    // Decode Stage	
    ////////////////////////////////////////////////////////////////////////////////////
    COMPUTE_POWER("instruction_decoder", 
         GET_CORE_STAT(core_id, POWER_INST_DECODER_R), 
         GET_CORE_STAT(core_id, POWER_INST_DECODER_W), level);
    COMPUTE_POWER("ID2uQ", 
         GET_CORE_STAT(core_id, POWER_INST_DECODER_R), 
         0, level);
    if (isa_type == X86) {
      COMPUTE_POWER("uop_sequencer", 
         GET_CORE_STAT(core_id, POWER_MICRO_OP_SEQ_R), 
         GET_CORE_STAT(core_id, POWER_MICRO_OP_SEQ_W), level);
      COMPUTE_POWER("operand_decoder", 
         GET_CORE_STAT(core_id, POWER_OPERAND_DECODER_R), 
         GET_CORE_STAT(core_id, POWER_OPERAND_DECODER_W), level);
    }
    COMPUTE_POWER("uopQ", 
         GET_CORE_STAT(core_id, POWER_UOP_QUEUE_R), 
         GET_CORE_STAT(core_id, POWER_UOP_QUEUE_W), level);
    COMPUTE_POWER("uQ2RR", 
         GET_CORE_STAT(core_id, POWER_MICRO_OP_SEQ_R), 
         0, level);
    COMPUTE_POWER("decode", 0, 0, level);
    
		COMPUTE_POWER_STAGE("Instruction Decoder Unit", 1, area_stage_dec, pow_stage_dec);


    ////////////////////////////////////////////////////////////////////////////////////
    // Renaming Stage	
    ////////////////////////////////////////////////////////////////////////////////////
    if (1) {
      //if(isa_type == X86)  // FIXME (Jieun 02-08-2012): should be added?
      COMPUTE_POWER("RAT", 
         GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_R), 
         GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_W), level);
      COMPUTE_POWER("RR2RS", 
         GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_R), 
         0, level);
      COMPUTE_POWER("operand_dependency", 
         GET_CORE_STAT(core_id, POWER_DEP_CHECK_LOGIC_R), 
         0, level);
      COMPUTE_POWER("freelist", 
         GET_CORE_STAT(core_id, POWER_FREELIST_R), 
         GET_CORE_STAT(core_id, POWER_FREELIST_W), level);
      COMPUTE_POWER("renaming", 0, 0, level);
      
			COMPUTE_POWER_STAGE("Renaming Stage", 1, area_stage_ren, pow_stage_ren);
    }


    ////////////////////////////////////////////////////////////////////////////////////
    // Schedule Stage	
    ////////////////////////////////////////////////////////////////////////////////////
    if (isa_type == X86) {
      if(schedule_type == OUT_OF_ORDER){
        COMPUTE_POWER_CACHE("RS", 
           GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R), 
           GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_W), 
           0, 
           0, 
           GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R_TAG), level);
      }
      COMPUTE_POWER("ROB", 
         GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), 
         GET_CORE_STAT(core_id, POWER_REORDER_BUF_W), level);
      COMPUTE_POWER("ROB2RS", 
         GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), 
         0, level);
      COMPUTE_POWER("ROB2CM", 
         GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), 
         0, level);
      COMPUTE_POWER("commit_select", 
         GET_CORE_STAT(core_id, POWER_INST_COMMIT_SEL_LOGIC_R), 
         GET_CORE_STAT(core_id, POWER_INST_COMMIT_SEL_LOGIC_W), level);
    }
    else {
      COMPUTE_POWER_CACHE("SB", 
         GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R), 
         GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_W), 
         0, 
         0, 
         GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R_TAG), level);
    }
    if(schedule_type == OUT_OF_ORDER){
      COMPUTE_POWER("issue_select", 
           GET_CORE_STAT(core_id, POWER_INST_ISSUE_SEL_LOGIC_R), 
           GET_CORE_STAT(core_id, POWER_INST_ISSUE_SEL_LOGIC_W), level);
    }
    COMPUTE_POWER("schedule", 0, 0, level);
    
		COMPUTE_POWER_STAGE("Instruction Scheduler", 1, area_stage_sch, pow_stage_sch);


    ////////////////////////////////////////////////////////////////////////////////////
    // Register File	
    ////////////////////////////////////////////////////////////////////////////////////
    if (isa_type == X86) {
      COMPUTE_POWER("IRF", 
         GET_CORE_STAT(core_id, POWER_INT_REGFILE_R), 
         GET_CORE_STAT(core_id, POWER_INT_REGFILE_W), level);
      COMPUTE_POWER("FRF", 
         GET_CORE_STAT(core_id, POWER_FP_REGFILE_R), 
         GET_CORE_STAT(core_id, POWER_FP_REGFILE_W), level);
    }
    else {
      COMPUTE_POWER("RF", 
         GET_CORE_STAT(core_id, POWER_INT_REGFILE_R)+GET_CORE_STAT(core_id, POWER_FP_REGFILE_R), 
         GET_CORE_STAT(core_id, POWER_INT_REGFILE_W)+GET_CORE_STAT(core_id, POWER_FP_REGFILE_W), 
         level);
    }
    COMPUTE_POWER("ARF2RS", 
         GET_CORE_STAT(core_id, POWER_INT_REGFILE_W) + GET_CORE_STAT(core_id, POWER_FP_REGFILE_W), 
         0,  level);
    
		COMPUTE_POWER_STAGE("Register File Unit", 1, area_stage_rf, pow_stage_rf);


    ////////////////////////////////////////////////////////////////////////////////////
    // Execution Stage 
    ////////////////////////////////////////////////////////////////////////////////////
    COMPUTE_POWER("payload", GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_R), 0, level);
    if (isa_type == X86) {
      COMPUTE_POWER("ALU", GET_CORE_STAT(core_id, POWER_EX_ALU_R), 0, level);	
      COMPUTE_POWER("FPU", GET_CORE_STAT(core_id, POWER_EX_FPU_R), 0, level);	
      COMPUTE_POWER("MUL", GET_CORE_STAT(core_id, POWER_EX_MUL_R), 0, level);	
    }
    else {
      COMPUTE_POWER("SP_alu", 
         GET_CORE_STAT(core_id, POWER_EX_ALU_R) + GET_CORE_STAT(core_id, POWER_EX_MUL_R), 
         0, level);	
      COMPUTE_POWER("SP_fpu", GET_CORE_STAT(core_id, POWER_EX_FPU_R), 0, level);	
      COMPUTE_POWER("SFU", 0, 0, level);	
      COMPUTE_POWER("ldst", GET_CORE_STAT(core_id, POWER_SEGMENT_REGISTER_R), 0, level);	
    }
    COMPUTE_POWER("FU2ROB", 
         GET_CORE_STAT(core_id, POWER_EX_ALU_R) + GET_CORE_STAT(core_id, POWER_EX_FPU_R) + \
         GET_CORE_STAT(core_id, POWER_EX_MUL_R), 
         0, level);

    COMPUTE_POWER("exec_bypass", GET_CORE_STAT(core_id, POWER_EXEC_BYPASS), 0, level);
    COMPUTE_POWER("load_bypass", GET_CORE_STAT(core_id, POWER_LOAD_BYPASS), 0, level);
    COMPUTE_POWER("execute", 0, 0, level);
    
		COMPUTE_POWER_STAGE("Execution Unit", 1, area_stage_ex, pow_stage_ex);


    ////////////////////////////////////////////////////////////////////////////////////
    // Memory Management Units
    ////////////////////////////////////////////////////////////////////////////////////
    COMPUTE_POWER_CACHE("loadQ", 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R), 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R_TAG), level);
    COMPUTE_POWER("LQ2ROB", 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R) + GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R_TAG), 
         0, level);
    COMPUTE_POWER("LQ2L1", 
         GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R) + GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R_TAG), 
         0, level);
    COMPUTE_POWER_CACHE("storeQ", 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R), 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R_TAG), level);
    COMPUTE_POWER("SQ2L1", 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R) + GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R_TAG),
         0, level);
    COMPUTE_POWER("SQ2LQ", 
         GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R) + GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R_TAG),
         0, level);
    COMPUTE_POWER_CACHE("ITLB", 
         GET_CORE_STAT(core_id, POWER_ICACHE_R), 
         GET_CORE_STAT(core_id, POWER_ICACHE_W), 
         GET_CORE_STAT(core_id, POWER_ICACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_ICACHE_W), 
         0, level);
    COMPUTE_POWER_CACHE("DTLB", 
         GET_CORE_STAT(core_id, POWER_DATA_TLB_R), 
         GET_CORE_STAT(core_id, POWER_DATA_TLB_W), 
         GET_CORE_STAT(core_id, POWER_DATA_TLB_R_TAG), 
         GET_CORE_STAT(core_id, POWER_DATA_TLB_W), 
         0, level);
    COMPUTE_POWER("memory", 0, 0, level);
    
		COMPUTE_POWER_STAGE("Memory Management Unit", 1, area_stage_mmu, pow_stage_mmu);


    ////////////////////////////////////////////////////////////////////////////////////
    // L1 cache 
    ////////////////////////////////////////////////////////////////////////////////////
    if (!l1_bypass) {
      COMPUTE_POWER_CACHE("DCache", 
         GET_CORE_STAT(core_id, POWER_DCACHE_R), 
         GET_CORE_STAT(core_id, POWER_DCACHE_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_DCACHE_W), 
         0, level);
      COMPUTE_POWER("L12LQ", 
         GET_CORE_STAT(core_id, POWER_DCACHE_R), 0, level);
      COMPUTE_POWER_CACHE("DCache:missbuffer", 
         GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_R), 
         GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_R_TAG), level);
      COMPUTE_POWER_CACHE("DCache:linefill", 
         GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R), 
         GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R_TAG),level);
      COMPUTE_POWER_CACHE("DCache:writeback", 
         GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_R), 
         GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_W), 
         GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_R_TAG), level);
		
      COMPUTE_POWER_STAGE("L1 Cache", 1, area_stage_l1, pow_stage_l1);
    }
    

    ////////////////////////////////////////////////////////////////////////////////////
    // L2 cache 
    ////////////////////////////////////////////////////////////////////////////////////
    if (!l2_bypass) {
      COMPUTE_POWER_CACHE("L2", 
         GET_CORE_STAT(core_id, POWER_L2CACHE_R), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_W), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_W), 
					0, level);
      COMPUTE_POWER_CACHE("L2:linefill", 
         GET_CORE_STAT(core_id, POWER_L2CACHE_LINEFILL_BUF_R), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_LINEFILL_BUF_W), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_LINEFILL_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_LINEFILL_BUF_W), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_LINEFILL_BUF_R_TAG), level);
      COMPUTE_POWER_CACHE("L2:writeback", 
         GET_CORE_STAT(core_id, POWER_L2CACHE_WB_BUF_R), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_WB_BUF_W), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_WB_BUF_R_TAG), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_WB_BUF_W), 
         GET_CORE_STAT(core_id, POWER_L2CACHE_WB_BUF_R_TAG), level);
      COMPUTE_POWER_STAGE("L2", 1, area_stage_l2, pow_stage_l2);
    }


    ////////////////////////////////////////////////////////////////////////////////////
    // GPU-specific caches
    ////////////////////////////////////////////////////////////////////////////////////
    if (isa_type == PTX) {
      COMPUTE_POWER_CACHE("SharedMem", 
         GET_CORE_STAT(core_id, POWER_SHARED_MEM_R), 
         GET_CORE_STAT(core_id, POWER_SHARED_MEM_W), 
         GET_CORE_STAT(core_id, POWER_SHARED_MEM_R_TAG), 
         GET_CORE_STAT(core_id, POWER_SHARED_MEM_W), 
         0, level);
      COMPUTE_POWER_STAGE("SharedMem", 1, area_stage_smem, pow_stage_smem);

      COMPUTE_POWER_CACHE("ConstCache", 
         GET_CORE_STAT(core_id, POWER_CONST_CACHE_R), 
         GET_CORE_STAT(core_id, POWER_CONST_CACHE_W), 
         GET_CORE_STAT(core_id, POWER_CONST_CACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_CONST_CACHE_W), 
         0, level);
      COMPUTE_POWER_STAGE("ConstCache", 1, area_stage_cmem, pow_stage_cmem);
      
			COMPUTE_POWER_CACHE("TextureCache", 
         GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_R), 
         GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_W), 
         GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_R_TAG), 
         GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_W), 
         0, level);
      COMPUTE_POWER_STAGE("TextureCache", 1, area_stage_tmem, pow_stage_tmem);
    }
  }

  // extra assignment needed if cores are not fully utilized 
	// and the last core has 0 cycle count
  // FIXME: Jieun 01-22-2012 : freq needs to be changed if hetero sim
  if (isa_type == X86) {
    current_time = (double)GET_STAT(CYC_COUNT_TOT)/GET_KNOB(CLOCK_CPU)/1e9; 
    period = (double)GET_STAT(CYC_COUNT_TOT)/GET_KNOB(CLOCK_CPU)/1e9; 
  }
  else if (isa_type == PTX) {
    current_time = (double)GET_STAT(CYC_COUNT_TOT)/GET_KNOB(CLOCK_GPU)/1e9; 
    period = (double)GET_STAT(CYC_COUNT_TOT)/GET_KNOB(CLOCK_GPU)/1e9; 
  }


  ////////////////////////////////////////////////////////////////////////////////////
  // L3 caches
  ////////////////////////////////////////////////////////////////////////////////////
  for (l3_id = 0; l3_id < GET_KNOB(NUM_L3); ++l3_id) {
    string str_l3 = "L3";
    str_l3.append(":");
    std::stringstream id_l3;
    id_l3 << l3_id;
    str_l3.append(id_l3.str());

    COMPUTE_POWER_LLC(str_l3.data(), 
         (int)(GET_STAT(POWER_L3CACHE_R)/GET_KNOB(NUM_L3)), 
         (int)(GET_STAT(POWER_L3CACHE_W)/GET_KNOB(NUM_L3)),
         (int)(GET_STAT(POWER_L3CACHE_R_TAG)/GET_KNOB(NUM_L3)), 
         (int)(GET_STAT(POWER_L3CACHE_W)/GET_KNOB(NUM_L3)), 
         0, level);

    string str_l3_2 = "L3:linefill";
    str_l3_2.append(":");
    str_l3_2.append(id_l3.str());

    COMPUTE_POWER_LLC(str_l3_2.data(), 
         GET_STAT(POWER_L3CACHE_LINEFILL_BUF_R), 
         GET_STAT(POWER_L3CACHE_LINEFILL_BUF_W), 
         GET_STAT(POWER_L3CACHE_LINEFILL_BUF_R_TAG), 
         GET_STAT(POWER_L3CACHE_LINEFILL_BUF_W), 
         GET_STAT(POWER_L3CACHE_LINEFILL_BUF_R_TAG), level);

    string str_l3_3 = "L3:writeback";
    str_l3_3.append(":");
    str_l3_3.append(id_l3.str());

    COMPUTE_POWER_LLC(str_l3_3.data(), 
         GET_STAT(POWER_L3CACHE_WB_BUF_R), 
         GET_STAT(POWER_L3CACHE_WB_BUF_W), 
         GET_STAT(POWER_L3CACHE_WB_BUF_R_TAG), 
         GET_STAT(POWER_L3CACHE_WB_BUF_W), 
         GET_STAT(POWER_L3CACHE_WB_BUF_R_TAG), level);
  }
  COMPUTE_POWER_STAGE("L3", 1, area_stage_l3, pow_stage_l3);


  ////////////////////////////////////////////////////////////////////////////////////
  // Memory Controller 
  ////////////////////////////////////////////////////////////////////////////////////
  for (mc_i = 0; mc_i < GET_KNOB(DRAM_NUM_MC); ++mc_i) {
    string str_mc = "MemCon";
    str_mc.append(":");
    std::stringstream id_mc;
    id_mc << mc_i;
    str_mc.append(id_mc.str());
    COMPUTE_POWER_MC(str_mc.data(), 
         (int)(GET_STAT(POWER_MC_R)/GET_KNOB(DRAM_NUM_MC)), 
         (int)(GET_STAT(POWER_MC_W)/GET_KNOB(DRAM_NUM_MC)), level);
  }
  COMPUTE_POWER_STAGE("MemCon", 1, area_stage_mc, pow_stage_mc);

  
  // print area/power of each stage 
	fprintf(fp, "%s;%f;%f;%f;%f;%f\n", "TOTAL", area_total*1e6, 
	        power_total.total - power_total.leakage, power_total.leakage, power_total.total, 
					power_total.total/area_total/1e6); 

  fprintf(fp, "\nIPC; ; ; ;%f\n", (double)GET_STAT(INST_COUNT_TOT)/(double)GET_STAT(CYC_COUNT_TOT));
  if (isa_type == X86) {
    fprintf(fp, "MPKI_CPU; ; ; ;%f\n\n", (double)GET_STAT(L3_MISS_CPU)/((double)GET_STAT(INST_COUNT_TOT)/1000.0));
  }
  else {
    fprintf(fp, "MPKI_GPU; ; ; ;%f\n\n", (double)GET_STAT(L3_MISS_GPU)/((double)GET_STAT(INST_COUNT_TOT)/1000.0));
  }

  fprintf(fp, "%s;%f;%f;%f;%f\n","Fetch", area_stage_fet, 
	        pow_stage_fet.total - pow_stage_fet.leakage, pow_stage_fet.leakage, pow_stage_fet.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Decoder ", area_stage_dec, 
	        pow_stage_dec.total - pow_stage_dec.leakage, pow_stage_dec.leakage, pow_stage_dec.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Renaming ", area_stage_ren, 
	        pow_stage_ren.total - pow_stage_ren.leakage, pow_stage_ren.leakage, pow_stage_ren.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Scheduler ", area_stage_sch, 
	        pow_stage_sch.total - pow_stage_sch.leakage, pow_stage_sch.leakage, pow_stage_sch.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Register file", area_stage_rf, 
	        pow_stage_rf.total - pow_stage_rf.leakage, pow_stage_rf.leakage, pow_stage_rf.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Execution Unit", area_stage_ex, 
	        pow_stage_ex.total - pow_stage_ex.leakage, pow_stage_ex.leakage, pow_stage_ex.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Memory Management Unit", area_stage_mmu, 
	        pow_stage_mmu.total - pow_stage_mmu.leakage, pow_stage_mmu.leakage, pow_stage_mmu.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","L1", area_stage_l1, 
	        pow_stage_l1.total - pow_stage_l1.leakage, pow_stage_l1.leakage, pow_stage_l1.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","L2", area_stage_l2, 
	        pow_stage_l2.total - pow_stage_l2.leakage, pow_stage_l2.leakage, pow_stage_l2.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","L3", area_stage_l3, 
	        pow_stage_l3.total - pow_stage_l3.leakage, pow_stage_l3.leakage, pow_stage_l3.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","MC", area_stage_mc, 
	        pow_stage_mc.total - pow_stage_mc.leakage, pow_stage_mc.leakage, pow_stage_mc.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Shared Memory", area_stage_smem, 
	        pow_stage_smem.total - pow_stage_smem.leakage, pow_stage_smem.leakage, pow_stage_smem.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Const Cache ", area_stage_cmem, 
	        pow_stage_cmem.total - pow_stage_cmem.leakage, pow_stage_cmem.leakage, pow_stage_cmem.total); 
  fprintf(fp, "%s;%f;%f;%f;%f\n","Texture Cache ", area_stage_tmem, 
	        pow_stage_tmem.total - pow_stage_tmem.leakage, pow_stage_tmem.leakage, pow_stage_tmem.total); 
  fprintf(fp, "%s;%f;%f;%f;%f;%f\n", "TOTAL", area_total*1e6, 
	        power_total.total - power_total.leakage, power_total.leakage, power_total.total, power_total.total/area_total/1e6); 

  fclose(fp);
}
