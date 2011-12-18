/**********************************************************************************************
* File         : ei_power.cc
* Author       : Jieun Lim 
* Date         : 9/6/2011
* SVN          :   
* Description  :   
*********************************************************************************************/

#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>

#include "ei_power.h"

#include "all_knobs.h"
#include "all_stats.h"

// ****** EI ******
#include "energy_introspector/energy_introspector.h"

#define CONFIG_FILE_NAME "ei.config"

#define GET_CORE_STAT(coreID, Event)	\
	m_simBase->m_ProcessorStats->core(coreID)[Event-PER_CORE_STATS_ENUM_FIRST].getCount() \

#define GET_STAT(Event) \
	(*m_simBase->m_ProcessorStats)[Event].getCount() \

#define GET_KNOB(Event) \
	m_simBase->m_knobs->KNOB_##Event->getValue() \

using namespace std;

ei_power_c::ei_power_c(macsim_c* simBase)
{
	m_simBase = simBase;

}

ei_power_c::~ei_power_c()
{
}

void ei_power_c::ei_config_gen_large_tech(FILE* fp, int core_id)
{

	cout << "--- [CORE " << core_id << "] (large core)  generating EI configuaration file(ei.config) - technology " << endl;
	fprintf(fp, "\t# large core\n");
	fprintf(fp, "\t\t# technology\n");
    fprintf(fp, "-technology.ID							LARGE_CORE\n");	
    fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", GET_KNOB(CPU_FREQUENCY));
    fprintf(fp, "-technology.feature_size				%de-9\n", GET_KNOB(FEATURE_SIZE));
    fprintf(fp, "-technology.component_type				core		## done\n");
    fprintf(fp, "-technology.core_type					%s			\n", (GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")? "ooo" : "inorder");
    fprintf(fp, "-technology.wire_type					global		## default\n");
    fprintf(fp, "-technology.device_type					hp			## default\n");
    fprintf(fp, "-technology.interconnect_projection		aggressive	## default\n");
	fprintf(fp, "-technology.wiring_type					global		## default\n");
	fprintf(fp, "-technology.opt_local					false\n");
	fprintf(fp, "-technology.embeded						false		## default\n");	// FIXME nbl: true??
	fprintf(fp, "-technology.longer_channel_device		false		## default\n");
	fprintf(fp, "-technology.temperature				380\n");
	fprintf(fp, "-technology.end\n");
	fprintf(fp, "\n");
}

void ei_power_c::ei_config_gen_large_mod(FILE* fp, int core_id )
{
	int mc_i;
	int l3_i;

	/*
	cout << "--- [CORE " << core_id << "] (large core) generating ei config file - modules " << core_id <<  endl;
	fprintf(fp, "# pseudo package 	## to calculate temperature\n");
	fprintf(fp, "-package.ID							  	test_package%d	\n", core_id);
	fprintf(fp, "-package.technology					 	LARGE_CORE	\n");
	fprintf(fp, "-package.thermal_library				HotSpot\n");
	fprintf(fp, "-package.ambient_temperature			315\n");
	fprintf(fp, "-package.interface_temperature			315\n");
	fprintf(fp, "-package.spreader_temperature			315\n");
	fprintf(fp, "-package.heatsink_temperature			315\n");
	fprintf(fp, "-package.model							grid\n");
	//fprintf(fp, "-package.grid_rows						32\n");
	//fprintf(fp, "-package.grid_columns					32\n");
	fprintf(fp, "-package.partition						core%d:fetch\n", core_id);
	fprintf(fp, "-package.partition						core%d:decode\n", core_id);
	fprintf(fp, "-package.partition						core%d:schedule\n", core_id);
	fprintf(fp, "-package.partition						core%d:execute\n", core_id);
	fprintf(fp, "-package.partition						core%d:memory\n", core_id);
	fprintf(fp, "-package.end\n");
	fprintf(fp, "\n");

	fprintf(fp, "-partition.ID						core%d:fetch\n", core_id);
	fprintf(fp, "-partition.x_left					0.0\n");
	fprintf(fp, "-partition.y_bottom					0.\n");
	fprintf(fp, "-partition.width					2e-3\n");
	fprintf(fp, "-partition.length					2e-3\n");
	fprintf(fp, "-partition.layer					0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module                  core%d:fetch\n", core_id);
	fprintf(fp, "-partition.module					core%d:program_counter\n", core_id);
	fprintf(fp, "-partition.module					core%d:byteQ\n", core_id);
	fprintf(fp, "-partition.module                  core%d:BQ2IQ\n", core_id);
	fprintf(fp, "-partition.module					core%d:instQ\n", core_id);
	fprintf(fp, "-partition.module                  core%d:IQ2ID\n", core_id);
	fprintf(fp, "-partition.module					core%d:bpred1\n", core_id);
	fprintf(fp, "-partition.module					core%d:ICache\n", core_id);
	fprintf(fp, "-partition.module					core%d:ICache:missbuffer\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID							core%d:decode\n", core_id);
	fprintf(fp, "-partition.x_left				0.0\n");
	fprintf(fp, "-partition.y_bottom				2e-3\n");
	fprintf(fp, "-partition.width				2e-3\n");
	fprintf(fp, "-partition.length				2e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module              core%d:decode\n", core_id);
	fprintf(fp, "-partition.module				core%d:instruction_decoder\n", core_id);
	fprintf(fp, "-partition.module				core%d:uop_sequencer\n", core_id);
	fprintf(fp, "-partition.module				core%d:operand_decoder\n", core_id);
	fprintf(fp, "-partition.module				core%d:ID2uQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:uopQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:uQ2RR\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:schedule\n", core_id);
	fprintf(fp, "-partition.x_left				2e-3\n");
	fprintf(fp, "-partition.y_bottom				0.0\n");
	fprintf(fp, "-partition.width				2e-3\n");
	fprintf(fp, "-partition.length				2e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module              core%d:schedule\n", core_id);
	fprintf(fp, "-partition.module				core%d:RAT\n", core_id);
	fprintf(fp, "-partition.module				core%d:operand_dependency\n", core_id);
	fprintf(fp, "-partition.module				core%d:freelist\n", core_id);
	fprintf(fp, "-partition.module				core%d:GPREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:SEGREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:CREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:FLAGREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:FPREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:RR2RS\n", core_id);
	fprintf(fp, "-partition.module				core%d:ARF2RS\n", core_id);
	fprintf(fp, "-partition.module				core%d:ROB2RS\n", core_id);
	fprintf(fp, "-partition.module				core%d:RS\n", core_id);
	fprintf(fp, "-partition.module				core%d:RS2PR\n", core_id);
	fprintf(fp, "-partition.module				core%d:issue_select\n", core_id);
	fprintf(fp, "-partition.module				core%d:ROB\n", core_id);
	fprintf(fp, "-partition.module				core%d:commit_select\n", core_id);
	fprintf(fp, "-partition.module				core%d:ROB2CM\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:memory\n", core_id);
	fprintf(fp, "-partition.x_left				2e-3\n");
	fprintf(fp, "-partition.y_bottom				2e-3\n");
	fprintf(fp, "-partition.width				2e-3\n");
	fprintf(fp, "-partition.length				2e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module              core%d:memory\n", core_id);
	fprintf(fp, "-partition.module				core%d:loadQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:LQ2ROB\n", core_id);
	fprintf(fp, "-partition.module				core%d:LQ2L1\n", core_id);
	fprintf(fp, "-partition.module				core%d:L12LQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:storeQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:SQ2L1\n", core_id);
	fprintf(fp, "-partition.module				core%d:SQ2LQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:DTLB\n", core_id);
	//fprintf(fp, "-partition.module				core%d:DTLB2\n", core_id);
	fprintf(fp, "-partition.module				core%d:DCache\n", core_id);
	fprintf(fp, "-partition.module				core%d:DCache:missbuffer\n", core_id);
	//fprintf(fp, "-partition.module				core%d:DCache:prefetch\n", core_id);
	fprintf(fp, "-partition.module				core%d:DCache:linefill\n", core_id);
	fprintf(fp, "-partition.module				core%d:DCache:writeback\n", core_id);
	fprintf(fp, "-partition.module				core%d:L2\n", core_id);
	for(mc_i=0; mc_i < GET_KNOB(DRAM_NUM_MC); mc_i++)
	{
		fprintf(fp, "-partition.module				core%d:MemCon%d\n", core_id, mc_i);
	}
	for(l3_i=0; l3_i < GET_KNOB(NUM_L3); l3_i++)
	{
		fprintf(fp, "-partition.module				core%d:L3:%d\n", core_id, l3_i);
	}
	//fprintf(fp, "-partition.module				core%d:DRAM\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:execute\n", core_id);
	fprintf(fp, "-partition.x_left				4e-3\n");
	fprintf(fp, "-partition.y_bottom				0.0\n");
	fprintf(fp, "-partition.width				4e-3\n");
	fprintf(fp, "-partition.length				4e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module              core%d:execute\n", core_id);
	fprintf(fp, "-partition.module				core%d:payload\n", core_id);
	fprintf(fp, "-partition.module				core%d:PR2FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port0:FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port1:FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port2:FU\n", core_id);
	fprintf(fp, "-partition.module              core%d:exec_bypass\n", core_id);
	fprintf(fp, "-partition.module              core%d:load_bypass\n", core_id);
	fprintf(fp, "-partition.module				core%d:FU2ROB\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- fetch partition\n");
	fprintf(fp, "-module.ID                 core%d:fetch\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE   \n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                3\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- decode partition\n");
	fprintf(fp, "-module.ID                 core%d:decode\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- schedule partition\n");
	fprintf(fp, "-module.ID                 core%d:schedule\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                3\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- memory partition\n");
	fprintf(fp, "-module.ID                 core%d:memory\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- execute partition\n");
	fprintf(fp, "-module.ID                 core%d:execute\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	*/
	fprintf(fp, "#program counter\n");
	fprintf(fp, "-module.ID                 core%d:program_counter\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#fetch queue\n");
	fprintf(fp, "-module.ID					core%d:byteQ\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/4);
	fprintf(fp, "-module.sets					%d\n", 3 * GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: fetch queue to instruction queue\n");
	fprintf(fp, "-module.ID                 core%d:BQ2IQ\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           160\n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction queue\n");
	fprintf(fp, "-module.ID					core%d:instQ\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				4		\n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(LARGE_WIDTH) * GET_KNOB(LARGE_CORE_ALLOC_LATENCY));	// jieun  width * depth
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));	// jieun  width 
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));	// jieun  width 
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction queue to decoder\n");
	fprintf(fp, "-module.ID                 core%d:IQ2ID\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           152\n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#branch predictor\n");
	fprintf(fp, "-module.ID					core%d:bpred1\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				1 # 2-bit saturation counter\n");
	fprintf(fp, "-module.sets					%d\n", (int)pow(2, GET_KNOB(BP_HIST_LENGTH)));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				fast\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache\n");
	fprintf(fp, "-module.ID					core%d:ICache\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(ICACHE_LARGE_LINE_SIZE) );
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(ICACHE_LARGE_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(ICACHE_LARGE_BANKS));
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ICACHE_LARGE_NUM_SET));
	fprintf(fp, "-module.tag_width				43 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(ICACHE_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(ICACHE_WRITE_PORTS));
	fprintf(fp, "-module.access_time				3\n");	 // jieun
	fprintf(fp, "-module.cycle_time				%d\n", GET_KNOB(ICACHE_LARGE_CYCLES));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:ICache:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				40\n");
	fprintf(fp, "-module.assoc					1\n");	// FIXME
	fprintf(fp, "-module.sets					16\n");
	fprintf(fp, "-module.tag_width				57 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction decoder\n");
	fprintf(fp, "-module.ID					core%d:instruction_decoder\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					%d \n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#micro-op sequencer\n");
	fprintf(fp, "-module.ID					core%d:uop_sequencer\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#operand decoder\n");
	fprintf(fp, "-module.ID					core%d:operand_decoder\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					4 \n" );
	fprintf(fp, "-module.scaling					%d \n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction decoder to uopQ\n");
	fprintf(fp, "-module.ID                 core%d:ID2uQ\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#uop queue\n");
	fprintf(fp, "-module.ID					core%d:uopQ\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel		ram\n");
	fprintf(fp, "-module.line_width				15\n" );
	fprintf(fp, "-module.sets					%d\n", 3 * GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: uopQ to register renaming\n");
	fprintf(fp, "-module.ID                 core%d:uQ2RR\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#register renaming table\n");
	fprintf(fp, "-module.ID					core%d:RAT\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				33 \n");
	fprintf(fp, "-module.assoc					1 \n");		// FIXME: 0?
	fprintf(fp, "-module.sets					16\n");
	fprintf(fp, "-module.rd_ports				%d \n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d \n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.rw_ports				%d\n", 1);
	fprintf(fp, "-module.access_mode				fast\n");
	fprintf(fp, "-module.scaling					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#dependency check logic\n");
	fprintf(fp, "-module.ID					core%d:operand_dependency\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				dependency_check_logic\n");
	fprintf(fp, "-module.compare_bits				%d\n", (int)ceil(log((double)GET_KNOB(ROB_SIZE))/log((double)2)));
	fprintf(fp, "-module.decode_width				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.scaling					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#freelist\n");
	fprintf(fp, "-module.ID                 core%d:freelist\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               array\n");
	fprintf(fp, "-module.energy_submodel                ram\n");
	fprintf(fp, "-module.line_width             %d\n", (int)ceil(log((double)GET_KNOB(ROB_SIZE))/log((double)2)/8));
	fprintf(fp, "-module.sets                   %d\n", GET_KNOB(ROB_SIZE));
	fprintf(fp, "-module.rd_ports               %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports               %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode                sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");


	fprintf(fp, "-module.ID							core%d:IRF\n", core_id);
	fprintf(fp, "-module.technology                  LARGE_CORE\n");
	fprintf(fp, "-module.energy_library              McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.size						%d\n", GET_KNOB(INT_REGFILE_SIZE)*GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.line_width					%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.rw_ports					1\n");
	fprintf(fp, "-module.rd_ports					%d\n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode					sequential\n");
	fprintf(fp, "-module.scaling						%d\n", GET_KNOB(MAX_THREADS_PER_LARGE_CORE));
	fprintf(fp, "-module.area_scaling				1.1 # cdb_overhead\n");
	fprintf(fp, "-module.end\n");

	fprintf(fp, "-module.ID							core%d:FRF\n", core_id);
	fprintf(fp, "-module.technology                  LARGE_CORE\n");
	fprintf(fp, "-module.energy_library              McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram \n");
	fprintf(fp, "-module.size						%d\n", GET_KNOB(FP_REGFILE_SIZE)*GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.line_width					%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.rw_ports					1\n");
	fprintf(fp, "-module.rd_ports					%d\n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode					sequential\n");
	fprintf(fp, "-module.scaling						%d\n", GET_KNOB(MAX_THREADS_PER_LARGE_CORE));
	fprintf(fp, "-module.area_scaling				1.1 # cdb_overhead\n");
	fprintf(fp, "-module.end\n");

	/*
	fprintf(fp, "#general purpose\n");
	fprintf(fp, "-module.ID					core%d:GPREG\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#segment registers\n");
	fprintf(fp, "-module.ID					core%d:SEGREG\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					6 \n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#control registers\n");
	fprintf(fp, "-module.ID					core%d:CREG\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#flags registers\n");
	fprintf(fp, "-module.ID					core%d:FLAGREG\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.sets					1 \n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#floating-point registers\n");
	fprintf(fp, "-module.ID					core%d:FPREG\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	*/
	fprintf(fp, "#latch: register renaming to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:RR2RS\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: architecture registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ARF2RS\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    6 \n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: physical registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ROB2RS\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    %d\n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reservation station\n");
	fprintf(fp, "-module.ID					core%d:RS\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				27 \n");
	fprintf(fp, "-module.tag_width				11 \n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ISCHED_LARGE_SIZE)+ GET_KNOB(MSCHED_LARGE_SIZE)+ GET_KNOB(FSCHED_LARGE_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(ISCHED_LARGE_RATE)+ GET_KNOB(MSCHED_LARGE_RATE)+ GET_KNOB(FSCHED_LARGE_RATE));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(ISCHED_LARGE_RATE)+ GET_KNOB(MSCHED_LARGE_RATE)+ GET_KNOB(FSCHED_LARGE_RATE));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction issue selection logic\n");
	fprintf(fp, "-module.ID					core%d:issue_select\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input			%d\n", GET_KNOB(ISCHED_LARGE_SIZE)+ GET_KNOB(MSCHED_LARGE_SIZE)+ GET_KNOB(FSCHED_LARGE_SIZE));
	fprintf(fp, "-module.selection_output			%d\n", GET_KNOB(ISCHED_LARGE_RATE) + GET_KNOB(MSCHED_LARGE_RATE)+ GET_KNOB(FSCHED_LARGE_RATE));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: reservation station to payload RAM\n");
	fprintf(fp, "-module.ID                 core%d:RS2PR\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#payload RAM\n");
	fprintf(fp, "-module.ID					core%d:payload\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				26 \n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: payload RAM output latch (to FU or snatch back)\n");
	fprintf(fp, "-module.ID                 core%d:PR2FU\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d \n", GET_KNOB(LARGE_WIDTH) );
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port0 FU\n");
	fprintf(fp, "-module.ID					core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				fpu\n");
	fprintf(fp, "-module.scaling                    2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port1 FU\n");
	fprintf(fp, "-module.ID					core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				alu\n");
	fprintf(fp, "-module.scaling                    6\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port2 FU\n");
	fprintf(fp, "-module.ID					core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				mul\n");
	fprintf(fp, "-module.scaling                    1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: functional unit to ROB\n");
	fprintf(fp, "-module.ID                 core%d:FU2ROB\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reorder buffer\n");
	fprintf(fp, "-module.ID					core%d:ROB\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", (int) 117/8);
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ROB_LARGE_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(LARGE_WIDTH) * 3);
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction commit selection logic\n");
	fprintf(fp, "-module.ID					core%d:commit_select\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input				%d\n", GET_KNOB(ROB_LARGE_SIZE));
	fprintf(fp, "-module.selection_output			%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: ROB to commit\n");
	fprintf(fp, "-module.ID                 core%d:ROB2CM\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#load queue\n");
	fprintf(fp, "-module.ID					core%d:loadQ\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA) /8 );
	fprintf(fp, "-module.assoc					1 \n");		// FIXME : 0?
	fprintf(fp, "-module.tag_width				%d\n", 86);
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(LOAD_BUF_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.search_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to ROB\n");
	fprintf(fp, "-module.ID                 core%d:LQ2ROB\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA)+6);
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to DCache\n");
	fprintf(fp, "-module.ID                 core%d:LQ2L1\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: DCache to load queue\n");
	fprintf(fp, "-module.ID                 core%d:L12LQ\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#store queue\n");
	fprintf(fp, "-module.ID					core%d:storeQ\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA) /8);
	fprintf(fp, "-module.assoc					1\n");	// FIXME: 0?
	fprintf(fp, "-module.tag_width				%d\n", 86);
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(STORE_BUF_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.search_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to DCache\n");
	fprintf(fp, "-module.ID                 core%d:SQ2L1\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to load queue\n");
	fprintf(fp, "-module.ID                 core%d:SQ2LQ\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data TLB\n");
	fprintf(fp, "-module.ID					core%d:DTLB\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				5 \n");
	fprintf(fp, "-module.assoc					1\n");
	fprintf(fp, "-module.sets					128\n");
	fprintf(fp, "-module.tag_width				58 \n");
	fprintf(fp, "-module.wr_ports				2\n");
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.access_time				2\n");
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache\n");
	fprintf(fp, "-module.ID					core%d:DCache\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L1_LARGE_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(L1_LARGE_NUM_BANK));
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(L1_LARGE_NUM_SET));
	fprintf(fp, "-module.tag_width				45 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L1_LARGE_LATENCY));
	fprintf(fp, "-module.cycle_time				3\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(LARGE_WIDTH)*4);
	fprintf(fp, "-module.tag_width				57 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				2\n");
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache linefill buffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:linefill\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME : 0?
	fprintf(fp, "-module.sets					16\n");
	fprintf(fp, "-module.tag_width				57 \n");
	fprintf(fp, "-module.rw_ports				2\n");
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache writeback buffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:writeback\n", core_id);
	fprintf(fp, "-module.technology				LARGE_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_LARGE_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME: 0?
	fprintf(fp, "-module.sets					16\n");
	fprintf(fp, "-module.tag_width				57 \n");
	fprintf(fp, "-module.rw_ports				2\n");
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");

	if(!GET_KNOB(L2_LARGE_BYPASS))
	{
		fprintf(fp, "#L2 cache\n");
		fprintf(fp, "-module.ID					core%d:L2\n", core_id);
		fprintf(fp, "-module.technology				LARGE_CORE\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				array\n");
		fprintf(fp, "-module.energy_submodel				cache\n");
		fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L2_LARGE_LINE_SIZE));
		fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L2_LARGE_ASSOC));
		fprintf(fp, "-module.bank					%d\n", GET_KNOB(L2_LARGE_NUM_BANK));
		fprintf(fp, "-module.sets					%d\n", GET_KNOB(L2_LARGE_NUM_SET));
		fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
		fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L2_READ_PORTS));
		fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L2_WRITE_PORTS));
		fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L2_LARGE_LATENCY));
		fprintf(fp, "-module.cycle_time				8\n");
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}
	/*
	fprintf(fp, "#bypass interconnect\n");
	fprintf(fp, "-module.ID                 core%d:exec_bypass\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               interconnect\n");
	fprintf(fp, "-module.data                   86 # data(80bit) + tag(6bit)\n");
  fprintf(fp, "-module.wire_length                6688.11e-6\n"); //nagesh
	fprintf(fp, "-module.connect                    core%d:RS\n", core_id);
	fprintf(fp, "-module.connect                    core%d:RS2PR\n", core_id);
	fprintf(fp, "-module.connect                    core%d:payload\n", core_id);
	fprintf(fp, "-module.connect                    core%d:PR2FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:loadQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:storeQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:ROB\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.scaling                    5\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#bypass interconnect\n");
	fprintf(fp, "-module.ID                 core%d:load_bypass\n", core_id);
	fprintf(fp, "-module.technology             LARGE_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               interconnect\n");
	fprintf(fp, "-module.data                   38 # data(32bit) + tag(6bit)\n");
  fprintf(fp, "-module.wire_length                6688.11e-6\n"); //nagesh
	fprintf(fp, "-module.connect                    core%d:RS\n", core_id);
	fprintf(fp, "-module.connect                    core%d:loadQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:storeQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:ROB\n", core_id);
	fprintf(fp, "-module.scaling                    5\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
*/

	for(l3_i=0; l3_i < GET_KNOB(NUM_L3); l3_i++)
	{
		fprintf(fp, "#data cache %d\n", l3_i);
		fprintf(fp, "-module.ID					core%d:L3:%d\n", core_id, l3_i);
		fprintf(fp, "-module.technology				L3\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				array\n");
		fprintf(fp, "-module.energy_submodel				cache\n");
		fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L3_LINE_SIZE));
		fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L3_ASSOC));
		fprintf(fp, "-module.bank					%d\n", GET_KNOB(L3_NUM_BANK));
		fprintf(fp, "-module.sets					%d\n", GET_KNOB(L3_NUM_SET));
		fprintf(fp, "-module.tag_width				37\n");
		fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L3_READ_PORTS));
		fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L3_WRITE_PORTS));
		fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L3_LATENCY));
		fprintf(fp, "-module.cycle_time				16\n");
		fprintf(fp, "-module.end\n");
		//fprintf(fp, "\n");
	}

	/*
	for(mc_i=0; mc_i < GET_KNOB(DRAM_NUM_MC); mc_i++)
	{

		fprintf(fp, "#memory controller #%d\n", mc_i);
		fprintf(fp, "-module.ID				core%d:MemCon%d\n", core_id, mc_i);
		fprintf(fp, "-module.technology				LARGE_CORE\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				memory_controller\n");
		fprintf(fp, "-module.energy_submodel			memory_controller\n");
		fprintf(fp, "-module.line					64\n");	// xml_interface.sys.mc.llc_line_length
		fprintf(fp, "-module.request_window			32\n");	// req_window_size_per_channel
		fprintf(fp, "-module.io_buffer				32\n");	// IO_buffer_size_per_channer
		fprintf(fp, "-module.memory_channels		1 \n");	// memory_channers_per_mc
		fprintf(fp, "-module.transfer_rate			3200\n");	// peak_transfer_rate
		fprintf(fp, "-module.ranks					2\n");	// number_ranks
		fprintf(fp, "-module.data_bus				128\n");	// databus_width
		fprintf(fp, "-module.address_bus			51\n");	// addressbus_width
		fprintf(fp, "-module.lvds					true\n");	// LVDS
		fprintf(fp, "-module.with_phy				false\n");	// withPHY
		fprintf(fp, "-module.model					high_performance\n");	// withPHY
		fprintf(fp, "-module.type					memory_controller\n");	// withPHY
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}

	fprintf(fp, "-module.ID							network\n");
	fprintf(fp, "-module.technology				 LARGE_CORE	\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				network\n");
	fprintf(fp, "-module.energy_submodel				bus\n");
	fprintf(fp, "-module.flit_bits					256\n");
	fprintf(fp, "-module.duty_cycle					1\n");
	fprintf(fp, "-module.link_throughput				1\n");
	fprintf(fp, "-module.link_latency				1\n");
	fprintf(fp, "-module.chip_area					415.758e-6 # 415mm^2\n");
	fprintf(fp, "-module.chip_coverage				1\n");
	fprintf(fp, "-module.route_over_percentage		0.5\n");
	fprintf(fp, "-module.end\n");
	*/
}

void ei_power_c::ei_config_gen_llc_tech(FILE* fp )
{

	cout << "--- [Last Level Cache] generating EI configuaration file(ei.config) - technology " << endl;
	fprintf(fp, "\t\t# llc technology\n");
    fprintf(fp, "-technology.ID							L3\n");	
    fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", GET_KNOB(CPU_FREQUENCY)/4);	// FIXME
    fprintf(fp, "-technology.feature_size				65e-9\n");
    fprintf(fp, "-technology.component_type				llc ## done\n");
    fprintf(fp, "-technology.wire_type					global		## default\n");
    fprintf(fp, "-technology.device_type					hp			## default\n");
    fprintf(fp, "-technology.interconnect_projection		aggressive	## default\n");
	fprintf(fp, "-technology.wiring_type					global		## default\n");
	fprintf(fp, "-technology.opt_local					false\n");
	fprintf(fp, "-technology.embeded						false		## default\n");
	fprintf(fp, "-technology.longer_channel_device		false		## default\n");
	fprintf(fp, "-technology.temperature				315\n");
	fprintf(fp, "-technology.end\n");
	fprintf(fp, "\n");
}

void ei_power_c::ei_config_gen_medium_tech(FILE* fp, int core_id)
{

	fprintf(fp, "# medium core\n");
	fprintf(fp, "# technology\n");
	fprintf(fp, "-technology.ID							LARGE_CORE\n");	
	fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", GET_KNOB(CPU_FREQUENCY));
	fprintf(fp, "-technology.feature_size				32e-9		## 32nm\n");
	fprintf(fp, "-technology.component_type				core		## done\n");
	fprintf(fp, "-technology.core_type					%s			\n", (GET_KNOB(LARGE_CORE_SCHEDULE) == "ooo")? "ooo" : "inorder");
	fprintf(fp, "-technology.wire_type					global		## default\n");
	fprintf(fp, "-technology.device_type					hp			## default\n");
	fprintf(fp, "-technology.interconnect_projection		aggressive	## default\n");
	fprintf(fp, "-technology.wiring_type					global		## default\n");
	fprintf(fp, "-technology.opt_local					false\n");
	fprintf(fp, "-technology.embeded						false		## default\n");
	fprintf(fp, "-technology.longer_channel_device		false		## default\n");
	fprintf(fp, "-technology.end\n");
	fprintf(fp, "\n");
}

void ei_power_c::ei_config_gen_medium_mod(FILE* fp, int core_id)
{
	fprintf(fp, "# medium core module\n");

}

void ei_power_c::ei_config_gen_small_tech(FILE* fp, int core_id)
{

	cout << "--- [Small Core] generating EI configuaration file(ei.config) - technology " << endl;
	fprintf(fp, "# small core\n");
	fprintf(fp, "# technology\n");
	fprintf(fp, "-technology.ID							SMALL_CORE\n");	
	fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", GET_KNOB(GPU_FREQUENCY));
	fprintf(fp, "-technology.feature_size				32e-9		## 32nm\n");
	fprintf(fp, "-technology.component_type				core		## done\n");
	fprintf(fp, "-technology.core_type					%s			\n", (GET_KNOB(SCHEDULE) == "ooo")? "ooo" : "inorder");
	fprintf(fp, "-technology.wire_type					global		## default\n");
	fprintf(fp, "-technology.device_type					hp			## default\n");
	fprintf(fp, "-technology.interconnect_projection		aggressive	## default\n");
	fprintf(fp, "-technology.wiring_type					global		## default\n");
	fprintf(fp, "-technology.opt_local					false\n");
	fprintf(fp, "-technology.embeded						false		## default\n");
	fprintf(fp, "-technology.longer_channel_device		false		## default\n");
	fprintf(fp, "-technology.temperature				315		## default\n");
	fprintf(fp, "-technology.end\n");
	fprintf(fp, "\n");
}

void ei_power_c::ei_config_gen_small_mod(FILE* fp, int core_id)
{

	fprintf(fp, "# small core modules\n");


	int mc_i;
	int l3_i;

	fprintf(fp, "#program counter\n");
	fprintf(fp, "-module.ID                 core%d:program_counter\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#fetch queue\n");
	fprintf(fp, "-module.ID					core%d:byteQ\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/4);
	fprintf(fp, "-module.sets					%d\n", 3 * GET_KNOB(WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: fetch queue to instruction queue\n");
	fprintf(fp, "-module.ID                 core%d:BQ2IQ\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           160\n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction queue\n");
	fprintf(fp, "-module.ID					core%d:instQ\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				4		\n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(WIDTH) * GET_KNOB(ALLOC_LATENCY));	// jieun  width * depth
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));	// jieun  width 
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));	// jieun  width 
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction queue to decoder\n");
	fprintf(fp, "-module.ID                 core%d:IQ2ID\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           152\n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#branch predictor\n");
	fprintf(fp, "-module.ID					core%d:bpred1\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				1 # 2-bit saturation counter\n");
	fprintf(fp, "-module.sets					%d\n", (int)pow(2, GET_KNOB(BP_HIST_LENGTH)));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				fast\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache\n");
	fprintf(fp, "-module.ID					core%d:ICache\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(ICACHE_LINE_SIZE) );
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(ICACHE_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(ICACHE_BANKS));
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ICACHE_NUM_SET));
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(ICACHE_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(ICACHE_WRITE_PORTS));
	fprintf(fp, "-module.access_time				2\n");	 // jieun
	fprintf(fp, "-module.cycle_time				%d\n", GET_KNOB(ICACHE_CYCLES));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:ICache:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				32\n");
	fprintf(fp, "-module.assoc					1\n");	// FIXME
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction decoder\n");
	fprintf(fp, "-module.ID					core%d:instruction_decoder\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					%d \n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#micro-op sequencer\n");
	fprintf(fp, "-module.ID					core%d:uop_sequencer\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#operand decoder\n");
	fprintf(fp, "-module.ID					core%d:operand_decoder\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					6 # %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.scaling					3 # decode width\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction decoder to uopQ\n");
	fprintf(fp, "-module.ID                 core%d:ID2uQ\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH) * 32);	// FIXME # of th: 16/32
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#uop queue\n");
	fprintf(fp, "-module.ID					core%d:uopQ\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel		ram\n");
	fprintf(fp, "-module.line_width				15\n" );
	fprintf(fp, "-module.sets					%d\n", 3 * GET_KNOB(WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: uopQ to register renaming\n");
	fprintf(fp, "-module.ID                 core%d:uQ2RR\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#dependency check logic\n");
	fprintf(fp, "-module.ID					core%d:operand_dependency\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				dependency_check_logic\n");
	fprintf(fp, "-module.compare_bits				%d\n", (int)ceil(log((double)GET_KNOB(ROB_SIZE))/log((double)2)));
	fprintf(fp, "-module.decode_width				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.scaling					%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#freelist\n");
	fprintf(fp, "-module.ID                 core%d:freelist\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               array\n");
	fprintf(fp, "-module.energy_submodel                ram\n");
	fprintf(fp, "-module.line_width             %d\n", (int)ceil(log((double)GET_KNOB(ROB_SIZE))/log((double)2))/8);
	fprintf(fp, "-module.sets                   %d\n", GET_KNOB(ROB_SIZE));
	fprintf(fp, "-module.rd_ports               %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports               %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode                sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");

	fprintf(fp, "-module.ID							core%d:IRF\n", core_id);
	fprintf(fp, "-module.technology                  SMALL_CORE\n");
	fprintf(fp, "-module.energy_library              McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.size						%d\n", GET_KNOB(INT_REGFILE_SIZE)*GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.line_width					%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.rw_ports					1\n");
	fprintf(fp, "-module.rd_ports					%d\n", 2*GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports					%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode					sequential\n");
	//fprintf(fp, "-module.scaling						%d\n", GET_KNOB(MAX_THREADS_PER_CORE));	// FIXME: jieun: for GPU version, is this right to use this scaling factor?
	fprintf(fp, "-module.area_scaling				1.1 # cdb_overhead\n");
	fprintf(fp, "-module.end\n");

	fprintf(fp, "-module.ID							core%d:FRF\n", core_id);
	fprintf(fp, "-module.technology                  SMALL_CORE\n");
	fprintf(fp, "-module.energy_library              McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram \n");
	fprintf(fp, "-module.size						%d\n", GET_KNOB(FP_REGFILE_SIZE)*GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.line_width					%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.rw_ports					1\n");
	fprintf(fp, "-module.rd_ports					%d\n", 2*GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.wr_ports					%d\n", GET_KNOB(LARGE_WIDTH));
	fprintf(fp, "-module.access_mode					sequential\n");
	fprintf(fp, "-module.scaling						%d\n", GET_KNOB(MAX_THREADS_PER_LARGE_CORE));
	fprintf(fp, "-module.area_scaling				1.1 # cdb_overhead\n");
	fprintf(fp, "-module.end\n");
	/*
	fprintf(fp, "#general purpose\n");
	fprintf(fp, "-module.ID					core%d:GPREG\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					8 \n");	
	fprintf(fp, "-module.banks					1 \n");	// FIXME: 16/32
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#segment registers\n");
	fprintf(fp, "-module.ID					core%d:SEGREG\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					6 \n");	
	fprintf(fp, "-module.banks					1 \n");	// FIXME: 16/32
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#control registers\n");
	fprintf(fp, "-module.ID					core%d:CREG\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA)/8);
	fprintf(fp, "-module.sets					8 \n");	// FIXME
	fprintf(fp, "-module.banks					1 \n");	// FIXME: 16/32
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#flags registers\n");
	fprintf(fp, "-module.ID					core%d:FLAGREG\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.sets					1 \n");
	fprintf(fp, "-module.banks					1 \n");	// FIXME: 16/32
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#floating-point registers\n");
	fprintf(fp, "-module.ID					core%d:FPREG\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.sets					8 \n");	
	fprintf(fp, "-module.banks					1 \n");	// FIXME: 16/32
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	*/
	fprintf(fp, "#latch: register renaming to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:RR2RS\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: architecture registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ARF2RS\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    6 \n", 2*GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: physical registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ROB2RS\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    %d\n", 2*GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reservation station\n");
	fprintf(fp, "-module.ID					core%d:RS\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				27 \n");
	fprintf(fp, "-module.tag_width				11 \n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ISCHED_SIZE)+ GET_KNOB(MSCHED_SIZE)+ GET_KNOB(FSCHED_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(ISCHED_RATE)+ GET_KNOB(MSCHED_RATE)+ GET_KNOB(FSCHED_RATE));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(ISCHED_RATE)+ GET_KNOB(MSCHED_RATE)+ GET_KNOB(FSCHED_RATE));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction issue selection logic\n");
	fprintf(fp, "-module.ID					core%d:issue_select\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input			%d\n", GET_KNOB(ISCHED_SIZE)+ GET_KNOB(MSCHED_SIZE)+ GET_KNOB(FSCHED_SIZE));
	fprintf(fp, "-module.selection_output			%d\n", GET_KNOB(ISCHED_RATE) + GET_KNOB(MSCHED_RATE)+ GET_KNOB(FSCHED_RATE));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: reservation station to payload RAM\n");
	fprintf(fp, "-module.ID                 core%d:RS2PR\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#payload RAM\n");
	fprintf(fp, "-module.ID					core%d:payload\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				26 \n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: payload RAM output latch (to FU or snatch back)\n");
	fprintf(fp, "-module.ID                 core%d:PR2FU\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d \n", GET_KNOB(WIDTH) );
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port0 FU\n");
	fprintf(fp, "-module.ID					core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				fpu\n");
	fprintf(fp, "-module.scaling						1.2\n");	// FIXME: GPU overhead
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port1 FU\n");
	fprintf(fp, "-module.ID					core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				alu\n");
	fprintf(fp, "-module.scaling						1.2\n");	// FIXME: GPU overhead
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port2 FU\n");
	fprintf(fp, "-module.ID					core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				mul\n");
	fprintf(fp, "-module.scaling						1.2\n");	// FIXME: GPU overhead
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: functional unit to ROB\n");
	fprintf(fp, "-module.ID                 core%d:FU2ROB\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reorder buffer\n");
	fprintf(fp, "-module.ID					core%d:ROB\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				117 \n");
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(ROB_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(WIDTH) * 3);
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction commit selection logic\n");
	fprintf(fp, "-module.ID					core%d:commit_select\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input				%d\n", GET_KNOB(ROB_SIZE));
	fprintf(fp, "-module.selection_output			%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: ROB to commit\n");
	fprintf(fp, "-module.ID                 core%d:ROB2CM\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#load queue\n");
	fprintf(fp, "-module.ID					core%d:loadQ\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA) /8 );
	fprintf(fp, "-module.assoc					1 \n");		// FIXME : 0?
	fprintf(fp, "-module.tag_width				%d\n", GET_KNOB(32_64_ISA)+5);
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(LOAD_BUF_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.search_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to ROB\n");
	fprintf(fp, "-module.ID                 core%d:LQ2ROB\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA)+6);
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to DCache\n");
	fprintf(fp, "-module.ID                 core%d:LQ2L1\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: DCache to load queue\n");
	fprintf(fp, "-module.ID                 core%d:L12LQ\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#store queue\n");
	fprintf(fp, "-module.ID					core%d:storeQ\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(32_64_ISA) /8);
	fprintf(fp, "-module.assoc					1\n");	// FIXME: 0?
	fprintf(fp, "-module.tag_width				%d\n", GET_KNOB(32_64_ISA) +5);
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(STORE_BUF_SIZE));
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.search_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to DCache\n");
	fprintf(fp, "-module.ID                 core%d:SQ2L1\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to load queue\n");
	fprintf(fp, "-module.ID                 core%d:SQ2LQ\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*GET_KNOB(32_64_ISA));
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data TLB\n");
	fprintf(fp, "-module.ID					core%d:DTLB\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				23 \n");
	fprintf(fp, "-module.assoc					4\n");
	fprintf(fp, "-module.sets					64\n");
	fprintf(fp, "-module.tag_width				28 \n");
	fprintf(fp, "-module.rw_ports				2\n");
	fprintf(fp, "-module.access_time				2\n");
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache\n");
	fprintf(fp, "-module.ID					core%d:DCache\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L1_SMALL_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(L1_SMALL_NUM_BANK));
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(L1_SMALL_NUM_SET));
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L1_SMALL_LATENCY));
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME
	fprintf(fp, "-module.sets					%d\n", GET_KNOB(WIDTH));
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache linefill buffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:linefill\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME : 0?
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 \n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache writeback buffer\n");
	fprintf(fp, "-module.ID					core%d:DCache:writeback\n", core_id);
	fprintf(fp, "-module.technology				SMALL_CORE\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L1_SMALL_LINE_SIZE));
	fprintf(fp, "-module.assoc					1\n");	// FIXME: 0?
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 \n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	if(!GET_KNOB(L2_SMALL_BYPASS))
	{
		fprintf(fp, "#L2 cache\n");
		fprintf(fp, "-module.ID					core%d:L2\n", core_id);
		fprintf(fp, "-module.technology				SMALL_CORE\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				array\n");
		fprintf(fp, "-module.energy_submodel				cache\n");
		fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L2_SMALL_LINE_SIZE));
		fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L2_SMALL_ASSOC));
		fprintf(fp, "-module.bank					%d\n", GET_KNOB(L2_SMALL_NUM_BANK));
		fprintf(fp, "-module.sets					%d\n", GET_KNOB(L2_SMALL_NUM_SET));
		fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
		fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L2_READ_PORTS));
		fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L2_WRITE_PORTS));
		fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L2_SMALL_LATENCY));
		fprintf(fp, "-module.cycle_time				2\n");
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}
	/*
	fprintf(fp, "#bypass interconnect\n");
	fprintf(fp, "-module.ID                 core%d:exec_bypass\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               interconnect\n");
	fprintf(fp, "-module.data                   86 # data(80bit) + tag(6bit)\n");
	fprintf(fp, "-module.connect                    core%d:RS\n", core_id);
	fprintf(fp, "-module.connect                    core%d:RS2PR\n", core_id);
	fprintf(fp, "-module.connect                    core%d:payload\n", core_id);
	fprintf(fp, "-module.connect                    core%d:PR2FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:loadQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:storeQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:ROB\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.scaling                    5\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#bypass interconnect\n");
	fprintf(fp, "-module.ID                 core%d:load_bypass\n", core_id);
	fprintf(fp, "-module.technology             SMALL_CORE\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               interconnect\n");
	fprintf(fp, "-module.data                   38 # data(32bit) + tag(6bit)\n");
	fprintf(fp, "-module.connect                    core%d:RS\n", core_id);
	fprintf(fp, "-module.connect                    core%d:loadQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:storeQ\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.connect                    core%d:ROB\n", core_id);
	fprintf(fp, "-module.scaling                    5\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
*/
	
	/*
	for(l3_i=0; l3_i < GET_KNOB(NUM_L3); l3_i++)
	{
		fprintf(fp, "#data cache %d\n", l3_i);
		fprintf(fp, "-module.ID					core%d:L3:%d\n", core_id, l3_i);
		fprintf(fp, "-module.technology				L3\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				array\n");
		fprintf(fp, "-module.energy_submodel				cache\n");
		fprintf(fp, "-module.line_width				%d\n", GET_KNOB(L3_LINE_SIZE));
		fprintf(fp, "-module.assoc					%d\n", GET_KNOB(L3_ASSOC));
		fprintf(fp, "-module.bank					%d\n", GET_KNOB(L3_NUM_BANK));
		fprintf(fp, "-module.sets					%d\n", GET_KNOB(L3_NUM_SET));
		fprintf(fp, "-module.tag_width				37\n");
		fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(DCACHE_READ_PORTS));
		fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(DCACHE_WRITE_PORTS));
		fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L3_LATENCY));
		fprintf(fp, "-module.cycle_time				2\n");
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}

	for(mc_i=0; mc_i < GET_KNOB(DRAM_NUM_MC); mc_i++)
	{

		fprintf(fp, "#memory controller #%d\n", mc_i);
		fprintf(fp, "-module.ID				core%d:MemCon%d\n", core_id, mc_i);
		fprintf(fp, "-module.technology				SMALL_CORE\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				memory_controller\n");
		fprintf(fp, "-module.energy_submodel			memory_controller\n");
		fprintf(fp, "-module.line					64\n");	// xml_interface.sys.mc.llc_line_length
		fprintf(fp, "-module.request_window			32\n");	// req_window_size_per_channel
		fprintf(fp, "-module.io_buffer				32\n");	// IO_buffer_size_per_channer
		fprintf(fp, "-module.memory_channels		1 \n");	// memory_channers_per_mc
		fprintf(fp, "-module.transfer_rate			3200\n");	// peak_transfer_rate
		fprintf(fp, "-module.ranks					2\n");	// number_ranks
		fprintf(fp, "-module.data_bus				128\n");	// databus_width
		fprintf(fp, "-module.address_bus			51\n");	// addressbus_width
		fprintf(fp, "-module.lvds					true\n");	// LVDS
		fprintf(fp, "-module.with_phy				false\n");	// withPHY
		fprintf(fp, "-module.model					high_performance\n");	// withPHY
		fprintf(fp, "-module.type					memory_controller\n");	// withPHY
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}

	fprintf(fp, "-module.ID							network\n");
	fprintf(fp, "-module.technology				 SMALL_CORE	\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				network\n");
	fprintf(fp, "-module.energy_submodel				bus\n");
	fprintf(fp, "-module.flit_bits					256\n");
	fprintf(fp, "-module.duty_cycle					1\n");
	fprintf(fp, "-module.link_throughput				1\n");
	fprintf(fp, "-module.link_latency				1\n");
	fprintf(fp, "-module.chip_area					415.758e-6 # 415mm^2\n");
	fprintf(fp, "-module.chip_coverage				1\n");
	fprintf(fp, "-module.route_over_percentage		0.5\n");
	fprintf(fp, "-module.end\n");
	*/



	fprintf(fp, "#const cache\n");
	fprintf(fp, "-module.ID					core%d:ConstCache\n", core_id );
	fprintf(fp, "-module.technology				L3\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(CONST_CACHE_LINE_SIZE));
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(CONST_CACHE_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(CONST_CACHE_BANKS));
	fprintf(fp, "-module.size					%d\n", GET_KNOB(CONST_CACHE_SIZE) * 1024);
	fprintf(fp, "-module.tag_width				37\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L3_LATENCY));
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");


	fprintf(fp, "#texture cache\n");
	fprintf(fp, "-module.ID					core%d:TextureCache\n", core_id );
	fprintf(fp, "-module.technology				L3\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", GET_KNOB(TEXTURE_CACHE_LINE_SIZE));
	fprintf(fp, "-module.assoc					%d\n", GET_KNOB(TEXTURE_CACHE_ASSOC));
	fprintf(fp, "-module.bank					%d\n", GET_KNOB(TEXTURE_CACHE_BANKS));
	fprintf(fp, "-module.size					%d\n", GET_KNOB(TEXTURE_CACHE_SIZE) * 1024);
	fprintf(fp, "-module.tag_width				37\n");
	fprintf(fp, "-module.rd_ports				%d\n", GET_KNOB(L1_READ_PORTS));
	fprintf(fp, "-module.wr_ports				%d\n", GET_KNOB(L1_WRITE_PORTS));
	fprintf(fp, "-module.access_time			%d\n", GET_KNOB(L3_LATENCY));
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
}

void ei_power_c::ei_config_gen_top()
{

	FILE* fp = fopen(CONFIG_FILE_NAME, "w");

	int i;

	int num_large_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
	int num_medium_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
	int num_small_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;
	int num_l3 = *m_simBase->m_knobs->KNOB_NUM_L3;


	// technology
	if(num_large_cores > 0)
	{
		ei_config_gen_large_tech(fp, i);
	}
	if(num_medium_cores > 0)
	{
		ei_config_gen_medium_tech(fp, i);
	}
	if(num_small_cores > 0)
	{
		ei_config_gen_small_tech(fp, i);
	}
	if(num_l3 > 0)
	{
		ei_config_gen_llc_tech(fp);
	}

	// module
	for(i=0; i<num_large_cores; i++)
	{
		ei_config_gen_large_mod(fp, i);
	}
	for(i=0; i<num_medium_cores; i++)
	{
		ei_config_gen_medium_mod(fp, i+num_large_cores);
	}
	for(i=0; i<num_small_cores; i++)
	{
		ei_config_gen_small_mod(fp, i+num_large_cores+num_medium_cores );
	}


	fclose(fp);
}

#define COMPUTE_POWER(module_name, num_read, num_write, indent ) do {\
	counters.reset(); \
	counters.read=num_read; \
	counters.write=num_write; \
	energy_introspector->compute_power(current_time, period, get_name(module_name, core_id), counters); \
	power = energy_introspector->pull_data<power_t>(current_time, "module", get_name(module_name, core_id), "power"); \
	power_block = power_block + power; \
	power_stage = power_stage + power; \
	power_core = power_core + power; \
	power_total = power_total + power; \
	} while(0) \

/*
	cout << indent << module_name << ":" << endl; \
	cout << indent << "  Dynamic = " << power.total-power.leakage << "W" << endl; \
	cout << indent << "  Leakage = " << power.leakage << "W" << endl; \
	cout << endl; \
	*/

#define COMPUTE_POWER_CACHE(module_name, num_read, num_write, num_read_tag, num_write_tag, indent ) do {\
	counters.reset(); \
	counters.read=num_read; \
	counters.write=num_write; \
	counters.read_tag=num_read_tag; \
	counters.write_tag=num_write_tag; \
	energy_introspector->compute_power(current_time, period, get_name(module_name, core_id), counters); \
	power = energy_introspector->pull_data<power_t>(current_time, "module", get_name(module_name, core_id), "power"); \
	power_block = power_block + power; \
	power_stage = power_stage + power; \
	power_core = power_core + power; \
	power_total = power_total + power; \
	} while(0) \

/*
	cout << indent << module_name << ":" << endl; \
	cout << indent << "  Dynamic = " << power.total-power.leakage << "W" << endl; \
	cout << indent << "  Leakage = " << power.leakage << "W" << endl; \
	cout << endl; \
*/

#define COMPUTE_POWER_BLOCK(block_name) do { \
	cout << ""<< block_name << " Block:" << endl; \
	cout << "       Dynamic = " << power_block.total - power_block.leakage << "W" << endl; \
	cout << "       Leakage = " << power_block.leakage << "W" << endl; \
	power_block.reset(); \
	} while(0) \

/*
	cout << endl; \
*/

#define COMPUTE_POWER_STAGE(stage_name) do { \
	cout << "------------ " << stage_name << " Stage:" << endl; \
	cout << "             Dynamic = " << power_stage.total - power_stage.leakage << "W" << endl; \
	cout << "             Leakage = " << power_stage.leakage << "W" << endl; \
	cout << endl; \
	power_stage.reset(); \
	} while(0) \

/*
*/

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

void ei_power_c::ei_main()
{
	energy_introspector_t *energy_introspector = new energy_introspector_t(CONFIG_FILE_NAME);

	power_t power, power_block,  power_stage, power_core, power_total;
	counters_t counters;


	int gen_reg;
	int seg_reg;
	int con_reg;

	int core_id, l3_id;
	double current_time, period;
	string core_type;

	counters_t counter_GPREG; 
	counters_t counter_SEGREG; 
	counters_t counter_CREG; 
	counters_t counter_FLAGREG; 
	counters_t counter_FPREG; 
	counters_t counter_ARF2RS; 
	
	int num_large_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
	int num_medium_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
	int num_small_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;

	string level(2, ' ');
	string level0(2, ' ');
	string level1(4, ' ');
	
	power.reset();
	power_block.reset();
	power_stage.reset();
	power_total.reset();

	for(core_id=0; core_id<*m_simBase->m_knobs->KNOB_NUM_SIM_CORES; core_id++)
	{
		if(core_id < num_large_cores)
		{
			core_type = "x86";
			current_time = period = (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/GET_KNOB(CPU_FREQUENCY)/1e9); 
		}
		else if(core_id >= num_large_cores + num_medium_cores)
		{
			core_type = "ptx";
			current_time = period = (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/GET_KNOB(GPU_FREQUENCY)/1e9); 
		}
		/*
		else
		{
			core_type = "amd";
			current_time = period = (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/GET_KNOB(GPU_FREQUENCY)/1e9); 
		}
		*/

		power_core.reset();

		//counter_GPREG.read = GET_CORE_STAT(core_id, INST_COUNT) - GET_CORE_STAT(core_id, POWER_CONTROL_REGISTER_W) - GET_CORE_STAT(core_id, POWER_SEGMENT_REGISTER_W);

		//COMPUTE_POWER(get_name("fetch", core_id), GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_W), level);
		COMPUTE_POWER_CACHE("ICache", GET_CORE_STAT(core_id, POWER_ICACHE_R), GET_CORE_STAT(core_id, POWER_ICACHE_W), GET_CORE_STAT(core_id, POWER_ICACHE_R_TAG), GET_CORE_STAT(core_id, POWER_ICACHE_W), level);
		COMPUTE_POWER_CACHE("ICache:missbuffer", GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_R), GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_W), GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_R_TAG), GET_CORE_STAT(core_id, POWER_ICACHE_MISS_BUF_W), level);
		COMPUTE_POWER_BLOCK("Instruction Cache");
		
		COMPUTE_POWER("bpred1", GET_CORE_STAT(core_id, POWER_BR_PRED_R), GET_CORE_STAT(core_id, POWER_BR_PRED_W), level);
		COMPUTE_POWER_BLOCK("Branch Predictior");
		
		//COMPUTE_POWER("decode", GET_CORE_STAT(core_id, POWER_INST_DECODER_R), GET_CORE_STAT(core_id, POWER_INST_DECODER_W), level);
		COMPUTE_POWER("instruction_decoder", GET_CORE_STAT(core_id, POWER_INST_DECODER_R), GET_CORE_STAT(core_id, POWER_INST_DECODER_W), level);
		//COMPUTE_POWER("uop_sequencer", GET_CORE_STAT(core_id, POWER_MICRO_OP_SEQ_R), GET_CORE_STAT(core_id, POWER_MICRO_OP_SEQ_W), level);
		COMPUTE_POWER("operand_decoder", GET_CORE_STAT(core_id, POWER_OPERAND_DECODER_R), GET_CORE_STAT(core_id, POWER_OPERAND_DECODER_W), level);
		COMPUTE_POWER("ID2uQ", GET_CORE_STAT(core_id, POWER_INST_DECODER_R), GET_CORE_STAT(core_id, POWER_INST_QUEUE_W), level);
		COMPUTE_POWER_BLOCK("Instruction Decoder");
		
		COMPUTE_POWER("program_counter", GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_W), level);
		COMPUTE_POWER("byteQ", GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_W), level);
		COMPUTE_POWER("BQ2IQ", GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_R), GET_CORE_STAT(core_id, POWER_FETCH_QUEUE_W), level);
		COMPUTE_POWER("instQ", GET_CORE_STAT(core_id, POWER_INST_QUEUE_R), GET_CORE_STAT(core_id, POWER_INST_QUEUE_W), level);
		COMPUTE_POWER("IQ2ID", GET_CORE_STAT(core_id, POWER_INST_QUEUE_R), GET_CORE_STAT(core_id, POWER_INST_QUEUE_W), level);
		COMPUTE_POWER("uopQ", GET_CORE_STAT(core_id, POWER_UOP_QUEUE_R), GET_CORE_STAT(core_id, POWER_UOP_QUEUE_W), level);
		COMPUTE_POWER("uQ2RR", GET_CORE_STAT(core_id, POWER_UOP_QUEUE_R), GET_CORE_STAT(core_id, POWER_UOP_QUEUE_W), level);
		COMPUTE_POWER_BLOCK("Instruction Buffer");	
		COMPUTE_POWER_STAGE("Instruction Fetch Unit");
		
		//COMPUTE_POWER("schedule", GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_R), GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_W), level);
		if( core_type != "ptx")
		{
			COMPUTE_POWER("RAT", GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_R), GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_W), level);
			COMPUTE_POWER("RR2RS", GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_R), GET_CORE_STAT(core_id, POWER_REG_RENAMING_TABLE_W), level);
		}
		COMPUTE_POWER("operand_dependency", GET_CORE_STAT(core_id, POWER_DEP_CHECK_LOGIC_R), GET_CORE_STAT(core_id, POWER_DEP_CHECK_LOGIC_W), level);
		COMPUTE_POWER("freelist", GET_CORE_STAT(core_id, POWER_FREELIST_R), GET_CORE_STAT(core_id, POWER_FREELIST_W), level);
		COMPUTE_POWER_BLOCK("Renaming Unit");
		
		COMPUTE_POWER_STAGE("Renaming Unit");

		COMPUTE_POWER_CACHE("DCache", GET_CORE_STAT(core_id, POWER_DCACHE_R), GET_CORE_STAT(core_id, POWER_DCACHE_W), GET_CORE_STAT(core_id, POWER_DCACHE_R_TAG), GET_CORE_STAT(core_id, POWER_DCACHE_W), level);
		COMPUTE_POWER_CACHE("DCache:missbuffer", GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_R), GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_W), GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_R_TAG), GET_CORE_STAT(core_id, POWER_DCACHE_MISS_BUF_W), level);
		COMPUTE_POWER_CACHE("DCache:linefill", GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R), GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_W), GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R_TAG), GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_W_TAG), level);
		COMPUTE_POWER_CACHE("DCache:writeback", GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_R), GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_W), GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_R_TAG), GET_CORE_STAT(core_id, POWER_DCACHE_WB_BUF_W_TAG), level);
		COMPUTE_POWER("L12LQ", GET_CORE_STAT(core_id, POWER_DCACHE_R), GET_CORE_STAT(core_id, POWER_DCACHE_W), level);
		COMPUTE_POWER_BLOCK("Data Cache");
		
		COMPUTE_POWER_CACHE("loadQ", GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R_TAG), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), level);
		COMPUTE_POWER("LQ2ROB", GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), level);
		COMPUTE_POWER("LQ2L1", GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), level);
		COMPUTE_POWER_BLOCK("Load Queue");

		COMPUTE_POWER_CACHE("storeQ", GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R), GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R_TAG), GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), level);
		COMPUTE_POWER("SQ2L1", GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R), GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), level);
		COMPUTE_POWER("SQ2LQ", GET_CORE_STAT(core_id, POWER_STORE_QUEUE_R), GET_CORE_STAT(core_id, POWER_STORE_QUEUE_W), level);
		COMPUTE_POWER_BLOCK("Store Queue");
		
		COMPUTE_POWER_STAGE("Lost Store Unit");
		
		COMPUTE_POWER_CACHE("DTLB", GET_CORE_STAT(core_id, POWER_DATA_TLB_R), GET_CORE_STAT(core_id, POWER_DATA_TLB_W), GET_CORE_STAT(core_id, POWER_DATA_TLB_R_TAG), GET_CORE_STAT(core_id, POWER_DATA_TLB_W), level);
		COMPUTE_POWER_BLOCK("Memory Management Unit");
		COMPUTE_POWER_STAGE("Memory Management Unit");

		/*
		COMPUTE_POWER("GPREG", 0, GET_CORE_STAT(core_id, INST_COUNT), level);
		COMPUTE_POWER("SEGREG", 0, GET_CORE_STAT(core_id, POWER_SEGMENT_REGISTER_W), level);
		COMPUTE_POWER("CREG", 0, GET_CORE_STAT(core_id, POWER_CONTROL_REGISTER_W), level);
		COMPUTE_POWER("FLAGREG", 0, GET_CORE_STAT(core_id, POWER_FREELIST_W), level);
		COMPUTE_POWER("FPREG", 0, GET_CORE_STAT(core_id, POWER_FREELIST_W), level);
		*/
		COMPUTE_POWER("IRF", GET_CORE_STAT(core_id, POWER_INT_REGFILE_R), GET_CORE_STAT(core_id, POWER_INT_REGFILE_W), level);
		COMPUTE_POWER("FRF", GET_CORE_STAT(core_id, POWER_FP_REGFILE_R), GET_CORE_STAT(core_id, POWER_FP_REGFILE_R), level);
		COMPUTE_POWER("ARF2RS", GET_CORE_STAT(core_id, POWER_INT_REGFILE_R) +GET_CORE_STAT(core_id, POWER_FP_REGFILE_R) ,GET_CORE_STAT(core_id, POWER_INT_REGFILE_W) +GET_CORE_STAT(core_id, POWER_FP_REGFILE_R), level);
		COMPUTE_POWER_BLOCK("Register Files");
		COMPUTE_POWER_STAGE("Register Files");
		
		COMPUTE_POWER("RS", GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R), GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_W), level);
		COMPUTE_POWER("RS2PR", GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_R), GET_CORE_STAT(core_id, POWER_RESERVATION_STATION_W), level);
		COMPUTE_POWER("issue_select", GET_CORE_STAT(core_id, POWER_INST_ISSUE_SEL_LOGIC_R), GET_CORE_STAT(core_id, POWER_INST_ISSUE_SEL_LOGIC_W), level);
		COMPUTE_POWER("ROB", GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), GET_CORE_STAT(core_id, POWER_REORDER_BUF_W), level);
		COMPUTE_POWER("ROB2RS", GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), GET_CORE_STAT(core_id, POWER_REORDER_BUF_W), level);
		COMPUTE_POWER("ROB2CM", GET_CORE_STAT(core_id, POWER_REORDER_BUF_R), GET_CORE_STAT(core_id, POWER_REORDER_BUF_W), level);
		COMPUTE_POWER("commit_select", GET_CORE_STAT(core_id, POWER_INST_COMMIT_SEL_LOGIC_R), GET_CORE_STAT(core_id, POWER_INST_COMMIT_SEL_LOGIC_W), level);
		COMPUTE_POWER_BLOCK("Instruction Scheduler");
		COMPUTE_POWER_STAGE("Instruction Scheduler");

		//COMPUTE_POWER("execute", GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_R), GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_W), level);
		COMPUTE_POWER("port1:FU", GET_CORE_STAT(core_id, POWER_EX_ALU_R), 0, level);	// ALU
		COMPUTE_POWER_BLOCK("ALU");

		COMPUTE_POWER("port0:FU", GET_CORE_STAT(core_id, POWER_EX_FPU_R), 0, level);	// FPU
		COMPUTE_POWER_BLOCK("FPU");
		COMPUTE_POWER("port2:FU", GET_CORE_STAT(core_id, POWER_EX_MUL_R), 0, level);	// MUL
		COMPUTE_POWER_BLOCK("MUL");

		COMPUTE_POWER("FU2ROB", GET_CORE_STAT(core_id, POWER_EX_ALU_R) + GET_CORE_STAT(core_id, POWER_EX_FPU_R) + GET_CORE_STAT(core_id, POWER_EX_MUL_R), 0, level);
		COMPUTE_POWER_BLOCK("Result Broadcast Bus");
		//COMPUTE_POWER("payload", GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_R), GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_W), level);
		//COMPUTE_POWER("PR2FU", GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_R), GET_CORE_STAT(core_id, POWER_PAYLOAD_RAM_W), level);

		COMPUTE_POWER_STAGE("Execution Unit");
		
		//COMPUTE_POWER("memory", GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_R), GET_CORE_STAT(core_id, POWER_LOAD_QUEUE_W), level);

		COMPUTE_POWER_CACHE("L2", GET_CORE_STAT(core_id, POWER_L2CACHE_R), GET_CORE_STAT(core_id, POWER_L2CACHE_W), GET_CORE_STAT(core_id, POWER_L2CACHE_R_TAG), GET_CORE_STAT(core_id, POWER_L2CACHE_W), level);
		COMPUTE_POWER_BLOCK("L2");
		COMPUTE_POWER_STAGE("L2");
		
		if(core_type == "ptx")
		{
			COMPUTE_POWER_CACHE("ConstCache", GET_CORE_STAT(core_id, POWER_CONST_CACHE_R), GET_CORE_STAT(core_id, POWER_CONST_CACHE_W), GET_CORE_STAT(core_id, POWER_CONST_CACHE_R_TAG), GET_CORE_STAT(core_id, POWER_CONST_CACHE_W), level);
			COMPUTE_POWER_CACHE("TextureCache", GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_R), GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_W), GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_R_TAG), GET_CORE_STAT(core_id, POWER_TEXTURE_CACHE_W), level);
		}
		cout << endl;
		cout << "  Core (before L3)" << core_id << " Total Power : " << power_core.total << "W" << endl; 
		cout << endl;

		for(l3_id=0; l3_id < GET_KNOB(NUM_L3); l3_id++)
		{
			string str_l3 = "L3";
			str_l3.append(":");
			std::stringstream id_l3;
			id_l3 << l3_id;
			str_l3.append(id_l3.str());

			COMPUTE_POWER_CACHE(str_l3.data(), GET_STAT(POWER_L3CACHE_R), GET_STAT(POWER_L3CACHE_W), GET_STAT(POWER_L3CACHE_R_TAG), GET_STAT(POWER_L3CACHE_W), level);
			COMPUTE_POWER_BLOCK("L3");
		}
		COMPUTE_POWER_STAGE("L3");
		//for(mc_i=0; mc_i < GET_KNOB(DRAM_NUM_MC; mc_i++)
		//{
		//COMPUTE_POWER("MemCon0", GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_R), GET_CORE_STAT(core_id, POWER_DCACHE_LINEFILL_BUF_W), level);
		//}
		//COMPUTE_POWER_STAGE("Memory");
		cout << endl;
		cout << "  Core" << core_id << " Total Power : " << power_core.total << "W" << endl; 
		cout << endl;

	}
	cout << " Total Power : " << power_total.total << "W" << endl; 
}


