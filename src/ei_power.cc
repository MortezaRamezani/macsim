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

#include "energy_introspector/energy_introspector.h"

// ****** mcpat *********
#include "energy_introspector/ENERGYLIB_McPAT/io.h"
#include "energy_introspector/ENERGYLIB_McPAT/xmlParser.h"
#include "energy_introspector/ENERGYLIB_McPAT/XML_Parse.h"
#include "energy_introspector/ENERGYLIB_McPAT/processor.h"
#include "energy_introspector/ENERGYLIB_McPAT/globalvar.h"
#include "energy_introspector/ENERGYLIB_McPAT/version.h"

#define CONFIG_FILE_NAME "Config_Macsim.xml"

using namespace std;

ei_power_c::ei_power_c(macsim_c* simBase)
{
	m_simBase = simBase;

}

ei_power_c::~ei_power_c()
{
}
/*
void ei_power_c::ei_config_gen_large_tech(FILE* fp, int core_id)
{

	cout << "--- [CORE " << core_id << "] (large core)  generating EI configuaration file(ei.config) - technology " << endl;
	fprintf(fp, "\t# large core\n");
	fprintf(fp, "\t\t# technology\n");
    fprintf(fp, "-technology.ID							test_tech\n");	
    fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue());
    fprintf(fp, "-technology.feature_size				65e-9\n");
    fprintf(fp, "-technology.component_type				core		## done\n");
    fprintf(fp, "-technology.core_type					%s			\n", (m_simBase->m_knobs->KNOB_LARGE_CORE_SCHEDULE->getValue() == "ooo")? "ooo" : "inorder");
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

void ei_power_c::ei_config_gen_large_mod(FILE* fp, int core_id )
{
	int mc_i;
	int l3_i;

	cout << "--- [CORE " << core_id << "] (large core) generating ei config file - modules " << core_id <<  endl;
	fprintf(fp, "# pseudo package 	## to calculate temperature\n");
	fprintf(fp, "-package.ID							  	test_package%d	\n", core_id);
	fprintf(fp, "-package.technology					 	test_tech	\n");
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
	fprintf(fp, "-partition.module					core%d:I$\n", core_id);
	fprintf(fp, "-partition.module					core%d:I$:missbuffer\n", core_id);
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
	fprintf(fp, "-partition.module				core%d:D$\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:missbuffer\n", core_id);
	//fprintf(fp, "-partition.module				core%d:D$:prefetch\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:linefill\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:writeback\n", core_id);
	fprintf(fp, "-partition.module				core%d:L2\n", core_id);
	for(mc_i=0; mc_i < m_simBase->m_knobs->KNOB_DRAM_NUM_MC->getValue(); mc_i++)
	{
		fprintf(fp, "-partition.module				core%d:MemCon%d\n", core_id, mc_i);
	}
	for(l3_i=0; l3_i < m_simBase->m_knobs->KNOB_NUM_L3->getValue(); l3_i++)
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
	fprintf(fp, "-module.technology             test_tech   \n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                3\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- decode partition\n");
	fprintf(fp, "-module.ID                 core%d:decode\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- schedule partition\n");
	fprintf(fp, "-module.ID                 core%d:schedule\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                3\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- memory partition\n");
	fprintf(fp, "-module.ID                 core%d:memory\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#undifferentiated core -- execute partition\n");
	fprintf(fp, "-module.ID                 core%d:execute\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               undifferentiated_core\n");
	fprintf(fp, "-module.pipeline_stages                2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#program counter\n");
	fprintf(fp, "-module.ID                 core%d:program_counter\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#fetch queue\n");
	fprintf(fp, "-module.ID					core%d:byteQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue()/4);
	fprintf(fp, "-module.sets					%d\n", 3 * m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: fetch queue to instruction queue\n");
	fprintf(fp, "-module.ID                 core%d:BQ2IQ\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           160\n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction queue\n");
	fprintf(fp, "-module.ID					core%d:instQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				4		\n");
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue() *m_simBase->m_knobs->KNOB_LARGE_CORE_ALLOC_LATENCY->getValue());	// jieun  width * depth
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());	// jieun  width 
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());	// jieun  width 
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction queue to decoder\n");
	fprintf(fp, "-module.ID                 core%d:IQ2ID\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           152\n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#branch predictor\n");
	fprintf(fp, "-module.ID					core%d:bpred1\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				1 # 2-bit saturation counter\n");
	fprintf(fp, "-module.sets					%d\n", (int)pow(2, m_simBase->m_knobs->KNOB_BP_HIST_LENGTH->getValue()));
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				fast\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache\n");
	fprintf(fp, "-module.ID					core%d:I$\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_LINE_SIZE->getValue() );
	fprintf(fp, "-module.assoc					%d\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_ASSOC->getValue());
	fprintf(fp, "-module.bank					%d\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_BANKS->getValue());
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_NUM_SET->getValue());
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_ICACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_ICACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.access_time				2\n");	 // jieun
	fprintf(fp, "-module.cycle_time				%d\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_CYCLES->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:I$:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				32\n");
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction decoder\n");
	fprintf(fp, "-module.ID					core%d:instruction_decoder\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					%d \n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#micro-op sequencer\n");
	fprintf(fp, "-module.ID					core%d:uop_sequencer\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					16\n");
	fprintf(fp, "-module.scaling					1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#operand decoder\n");
	fprintf(fp, "-module.ID					core%d:operand_decoder\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				instruction_decoder\n");
	fprintf(fp, "-module.x86					true\n");
	fprintf(fp, "-module.opcode					6 # %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.scaling					3 # decode width\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: instruction decoder to uopQ\n");
	fprintf(fp, "-module.ID                 core%d:ID2uQ\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#uop queue\n");
	fprintf(fp, "-module.ID					core%d:uopQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel		ram\n");
	fprintf(fp, "-module.line_width				15\n" );
	fprintf(fp, "-module.sets					%d\n", 3 * m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: uopQ to register renaming\n");
	fprintf(fp, "-module.ID                 core%d:uQ2RR\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#register renaming table\n");
	fprintf(fp, "-module.ID					core%d:RAT\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				1 \n");
	fprintf(fp, "-module.assoc					0 \n");
	fprintf(fp, "-module.sets					16\n");
	fprintf(fp, "-module.rd_ports				%d \n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d \n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.search_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				fast\n");
	fprintf(fp, "-module.scaling					%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#dependency check logic\n");
	fprintf(fp, "-module.ID					core%d:operand_dependency\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				dependency_check_logic\n");
	fprintf(fp, "-module.compare_bits				%d\n", (int)ceil(log((double)m_simBase->m_knobs->KNOB_ROB_SIZE->getValue())/log((double)2)));
	fprintf(fp, "-module.decode_width				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.scaling					%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#freelist\n");
	fprintf(fp, "-module.ID                 core%d:freelist\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               array\n");
	fprintf(fp, "-module.energy_submodel                ram\n");
	fprintf(fp, "-module.line_width             %d\n", (int)ceil(log((double)m_simBase->m_knobs->KNOB_ROB_SIZE->getValue())/log((double)2))/8);
	fprintf(fp, "-module.sets                   %d\n", m_simBase->m_knobs->KNOB_ROB_SIZE->getValue());
	fprintf(fp, "-module.rd_ports               %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports               %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode                sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#general purpose\n");
	fprintf(fp, "-module.ID					core%d:GPREG\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue()/8);
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#segment registers\n");
	fprintf(fp, "-module.ID					core%d:SEGREG\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue()/8);
	fprintf(fp, "-module.sets					6 \n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#control registers\n");
	fprintf(fp, "-module.ID					core%d:CREG\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue()/8);
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#flags registers\n");
	fprintf(fp, "-module.ID					core%d:FLAGREG\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.sets					1 \n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#floating-point registers\n");
	fprintf(fp, "-module.ID					core%d:FPREG\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.sets					8 \n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.energy_scaling				1.1 # McPAT overhead number\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: register renaming to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:RR2RS\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           136 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: architecture registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ARF2RS\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    6 \n", 2*m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: physical registers to reservation station\n");
	fprintf(fp, "-module.ID                 core%d:ROB2RS\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           80 \n");
	fprintf(fp, "-module.scaling                    %d\n", 2*m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reservation station\n");
	fprintf(fp, "-module.ID					core%d:RS\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				27 \n");
	fprintf(fp, "-module.tag_width				11 \n");
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_ISCHED_LARGE_SIZE->getValue()+ m_simBase->m_knobs->KNOB_MSCHED_LARGE_SIZE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_SIZE->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_ISCHED_LARGE_RATE->getValue()+ m_simBase->m_knobs->KNOB_MSCHED_LARGE_RATE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_RATE->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_ISCHED_LARGE_RATE->getValue()+ m_simBase->m_knobs->KNOB_MSCHED_LARGE_RATE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_RATE->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction issue selection logic\n");
	fprintf(fp, "-module.ID					core%d:issue_select\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input			%d\n", m_simBase->m_knobs->KNOB_ISCHED_LARGE_SIZE->getValue()+ m_simBase->m_knobs->KNOB_MSCHED_LARGE_SIZE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_SIZE->getValue());
	fprintf(fp, "-module.selection_output			%d\n", m_simBase->m_knobs->KNOB_ISCHED_LARGE_RATE->getValue() + m_simBase->m_knobs->KNOB_MSCHED_LARGE_RATE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_RATE->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: reservation station to payload RAM\n");
	fprintf(fp, "-module.ID                 core%d:RS2PR\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#payload RAM\n");
	fprintf(fp, "-module.ID					core%d:payload\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				26 \n");
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: payload RAM output latch (to FU or snatch back)\n");
	fprintf(fp, "-module.ID                 core%d:PR2FU\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           204 \n");
	fprintf(fp, "-module.scaling                    %d \n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue() );
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port0 FU\n");
	fprintf(fp, "-module.ID					core%d:port0:FU\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				fpu\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port1 FU\n");
	fprintf(fp, "-module.ID					core%d:port1:FU\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				alu\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#execution port2 FU\n");
	fprintf(fp, "-module.ID					core%d:port2:FU\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				functional_unit\n");
	fprintf(fp, "-module.energy_submodel				mul\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: functional unit to ROB\n");
	fprintf(fp, "-module.ID                 core%d:FU2ROB\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#reorder buffer\n");
	fprintf(fp, "-module.ID					core%d:ROB\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				117 \n");
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_ROB_LARGE_SIZE->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue() * 3);
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.search_ports				2\n");
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#instruction commit selection logic\n");
	fprintf(fp, "-module.ID					core%d:commit_select\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				selection_logic\n");
	fprintf(fp, "-module.selection_input				%d\n", m_simBase->m_knobs->KNOB_ROB_LARGE_SIZE->getValue());
	fprintf(fp, "-module.selection_output			%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: ROB to commit\n");
	fprintf(fp, "-module.ID                 core%d:ROB2CM\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           86 \n");
	fprintf(fp, "-module.scaling                    %d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#load queue\n");
	fprintf(fp, "-module.ID					core%d:loadQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue() /8 );
	fprintf(fp, "-module.assoc					0 # fully associative\n");
	fprintf(fp, "-module.tag_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue()+5);
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_LOAD_BUF_SIZE->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.search_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to ROB\n");
	fprintf(fp, "-module.ID                 core%d:LQ2ROB\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue()+6);
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: load queue to D$\n");
	fprintf(fp, "-module.ID                 core%d:LQ2L1\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: D$ to load queue\n");
	fprintf(fp, "-module.ID                 core%d:L12LQ\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#store queue\n");
	fprintf(fp, "-module.ID					core%d:storeQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue() /8);
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.tag_width				%d\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue() +5);
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_STORE_BUF_SIZE->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.search_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.access_mode				sequential\n");
	fprintf(fp, "-module.area_scaling				1.1\n");
	fprintf(fp, "-module.energy_scaling				1.1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to D$\n");
	fprintf(fp, "-module.ID                 core%d:SQ2L1\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#latch: store queue to load queue\n");
	fprintf(fp, "-module.ID                 core%d:SQ2LQ\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
	fprintf(fp, "-module.energy_library             McPAT\n");
	fprintf(fp, "-module.energy_model               pipeline\n");
	fprintf(fp, "-module.pipeline_stages                1\n");
	fprintf(fp, "-module.per_stage_vector           %d\n", 2*m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data TLB\n");
	fprintf(fp, "-module.ID					core%d:DTLB\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
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
	fprintf(fp, "-module.ID					core%d:D$\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_LINE_SIZE->getValue());
	fprintf(fp, "-module.assoc					%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_ASSOC->getValue());
	fprintf(fp, "-module.bank					%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_NUM_BANK->getValue());
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_NUM_SET->getValue());
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.access_time			%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY->getValue());
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:D$:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_LINE_SIZE->getValue());
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache linefill buffer\n");
	fprintf(fp, "-module.ID					core%d:D$:linefill\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_LINE_SIZE->getValue());
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 \n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache writeback buffer\n");
	fprintf(fp, "-module.ID					core%d:D$:writeback\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L1_LARGE_LINE_SIZE->getValue());
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 \n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#L2 cache\n");
	fprintf(fp, "-module.ID					core%d:L2\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L2_LARGE_LINE_SIZE->getValue());
	fprintf(fp, "-module.assoc					%d\n", m_simBase->m_knobs->KNOB_L2_LARGE_ASSOC->getValue());
	fprintf(fp, "-module.bank					%d\n", m_simBase->m_knobs->KNOB_L2_LARGE_NUM_BANK->getValue());
	fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_L2_LARGE_NUM_SET->getValue());
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.access_time			%d\n", m_simBase->m_knobs->KNOB_L2_LARGE_LATENCY->getValue());
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#bypass interconnect\n");
	fprintf(fp, "-module.ID                 core%d:exec_bypass\n", core_id);
	fprintf(fp, "-module.technology             test_tech\n");
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
	fprintf(fp, "-module.technology             test_tech\n");
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


	for(mc_i=0; mc_i < m_simBase->m_knobs->KNOB_DRAM_NUM_MC->getValue(); mc_i++)
	{

		fprintf(fp, "#memory controller #%d\n", mc_i);
		fprintf(fp, "-module.ID				core%d:MemCon%d\n", core_id, mc_i);
		fprintf(fp, "-module.technology				test_tech\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				memory_controller\n");
		//fprintf(fp, "-module.energy_submodel			memory_controller\n");
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

	for(l3_i=0; l3_i < m_simBase->m_knobs->KNOB_NUM_L3->getValue(); l3_i++)
	{
		fprintf(fp, "#data cache %d\n", l3_i);
		fprintf(fp, "-module.ID					core%d:L3:%d\n", core_id, l3_i);
		fprintf(fp, "-module.technology				test_tech\n");
		fprintf(fp, "-module.energy_library				McPAT\n");
		fprintf(fp, "-module.energy_model				array\n");
		fprintf(fp, "-module.energy_submodel				cache\n");
		fprintf(fp, "-module.line_width				%d\n", m_simBase->m_knobs->KNOB_L3_LINE_SIZE->getValue());
		fprintf(fp, "-module.assoc					%d\n", m_simBase->m_knobs->KNOB_L3_ASSOC->getValue());
		fprintf(fp, "-module.bank					%d\n", m_simBase->m_knobs->KNOB_L3_NUM_BANK->getValue());
		fprintf(fp, "-module.sets					%d\n", m_simBase->m_knobs->KNOB_L3_NUM_SET->getValue());
		fprintf(fp, "-module.tag_width				37\n");
		fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
		fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
		fprintf(fp, "-module.access_time			%d\n", m_simBase->m_knobs->KNOB_L3_LATENCY->getValue());
		fprintf(fp, "-module.cycle_time				2\n");
		fprintf(fp, "-module.end\n");
		fprintf(fp, "\n");
	}
	//fclose(fp);
}

void ei_power_c::ei_config_gen_medium_tech(FILE* fp, int core_id)
{

	fprintf(fp, "# medium core\n");
	fprintf(fp, "# technology\n");
	fprintf(fp, "-technology.ID							test_tech\n");	
	fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue());
	fprintf(fp, "-technology.feature_size				32e-9		## 32nm\n");
	fprintf(fp, "-technology.component_type				core		## done\n");
	fprintf(fp, "-technology.core_type					%s			\n", (m_simBase->m_knobs->KNOB_LARGE_CORE_SCHEDULE->getValue() == "ooo")? "ooo" : "inorder");
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

	fprintf(fp, "# small core\n");
	fprintf(fp, "# technology\n");
	fprintf(fp, "-technology.ID							test_tech\n");	
	fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue());
	fprintf(fp, "-technology.feature_size				32e-9		## 32nm\n");
	fprintf(fp, "-technology.component_type				core		## done\n");
	fprintf(fp, "-technology.core_type					%s			\n", (m_simBase->m_knobs->KNOB_LARGE_CORE_SCHEDULE->getValue() == "ooo")? "ooo" : "inorder");
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

void ei_power_c::ei_config_gen_small_mod(FILE* fp, int core_id)
{
	fprintf(fp, "# small core modules\n");
}


void ei_power_c::ei_config_gen_top()
{

	FILE* fp = fopen("ei.config", "w");

	int i;

	int num_large_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES;
	int num_medium_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES;
	int num_small_cores = *m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES;


	if(num_large_cores > 0)
	{
		ei_config_gen_large_tech(fp, i);
		for(i=0; i<num_large_cores; i++)
		{
			ei_config_gen_large_mod(fp, i);
		}
	}

	if(num_medium_cores > 0)
	{
		ei_config_gen_medium_tech(fp, i);
		for(i=0; i<num_medium_cores; i++)
		{
			ei_config_gen_medium_mod(fp, i+num_large_cores);
		}
	}

	if(num_small_cores > 0)
	{
		ei_config_gen_small_tech(fp, i);
		for(i=0; i<num_small_cores; i++)
		{
			ei_config_gen_small_mod(fp, i+num_large_cores+num_medium_cores );
		}
	}
	fclose(fp);
}
*/

// one more arg: core_type: large, medium, small
void ei_power_c::mcpat_config_gen_tech(FILE *fp, int core_id)
{
	fprintf(fp, "<component id=\"root\" name=\"root\">\n");
	fprintf(fp, "\t<component id=\"system\" name=\"system\">\n");
	fprintf(fp, "\t\t<param name=\"number_of_cores\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_NUM_SIM_CORES->getValue());
	fprintf(fp, "\t\t<param name=\"number_of_L1Directories\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"number_of_L2Directories\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"number_of_L2s\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_NUM_SIM_CORES->getValue()); 
	fprintf(fp, "\t\t<param name=\"Private_L2\" value=\"1\"/>\n"); 
	fprintf(fp, "\t\t<param name=\"number_of_L3s\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_NUM_L3->getValue()); 
	fprintf(fp, "\t\t<param name=\"number_of_NoCs\" value=\"1\"/>\n");

	int large = m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES->getValue();
	int medium = m_simBase->m_knobs->KNOB_NUM_SIM_MEDIUM_CORES->getValue();
	int small = m_simBase->m_knobs->KNOB_NUM_SIM_SMALL_CORES->getValue();
	int is_homo = !((large&&medium) || (medium&&small) || (small&&large));

	fprintf(fp, "\t\t<param name=\"homogeneous_cores\" value=\"%d\"/>\n", is_homo);	
	fprintf(fp, "\t\t<param name=\"homogeneous_L2s\" value=\"1\"/>\n");
	fprintf(fp, "\t\t<param name=\"homogeneous_L1Directorys\" value=\"1\"/>\n");
	fprintf(fp, "\t\t<param name=\"homogeneous_L2Directorys\" value=\"1\"/>\n");
	fprintf(fp, "\t\t<param name=\"homogeneous_L3s\" value=\"1\"/>\n");
	fprintf(fp, "\t\t<param name=\"homogeneous_ccs\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"homogeneous_NoCs\" value=\"1\"/>\n");
	fprintf(fp, "\t\t<param name=\"core_tech_node\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_FEATURE_SIZE->getValue());
	fprintf(fp, "\t\t<param name=\"target_core_clockrate\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue() *1000));
	fprintf(fp, "\t\t<param name=\"temperature\" value=\"380\"/>\n"); 
	int l1_bypass = m_simBase->m_knobs->KNOB_L1_LARGE_BYPASS->getValue();	// fixme: should include gpu feature
	int l2_bypass = m_simBase->m_knobs->KNOB_L2_LARGE_BYPASS->getValue();	// fixme: should include gpu feature
	int is_l3 = m_simBase->m_knobs->KNOB_NUM_L3 == 0? 1 : 0;
	int num_cache_levels = 3 - l1_bypass - l2_bypass - is_l3; 

	fprintf(fp, "\t\t<param name=\"number_cache_levels\" value=\"%d\"/>\n", num_cache_levels);	
	fprintf(fp, "\t\t<param name=\"interconnect_projection_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"device_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"longer_channel_device\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<param name=\"machine_bits\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "\t\t<param name=\"virtual_address_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "\t\t<param name=\"physical_address_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "\t\t<param name=\"virtual_memory_page_size\" value=\"4096\"/>\n");
	fprintf(fp, "\t\t<stat name=\"total_cycles\" value=\"%lld\"/>\n", (*m_simBase->m_ProcessorStats)[CYC_COUNT_TOT].getCount());
	fprintf(fp, "\t\t<stat name=\"idle_cycles\" value=\"0\"/>\n");
	fprintf(fp, "\t\t<stat name=\"busy_cycles\"  value=\"%lld\"/>\n", (*m_simBase->m_ProcessorStats)[CYC_COUNT_TOT].getCount());
}

// fixme: variable including "LARGE" should be changed according to the core type
void ei_power_c::mcpat_config_gen_large(FILE *fp, int core_id)
{
	fprintf(fp, "\t\t<component id=\"system.core%d\" name=\"core%d\">\n", core_id, core_id);
	fprintf(fp, "\t\t\t<param name=\"clock_rate\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue() *1000));
	fprintf(fp, "\t\t\t<param name=\"instruction_length\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_32_64_ISA->getValue());
	fprintf(fp, "\t\t\t<param name=\"opcode_width\" value=\"16\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"x86\" value=\"1\"/>\n");	// fixme: add gpu
	fprintf(fp, "\t\t\t<param name=\"micro_opcode_width\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"machine_type\" value=\"%d\"/>\n", (m_simBase->m_knobs->KNOB_LARGE_CORE_SCHEDULE->getValue() == "ooo")? 0 : 1 );	
	fprintf(fp, "\t\t\t<param name=\"number_hardware_threads\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_MAX_THREADS_PER_LARGE_CORE->getValue());	// fixme: gpu
	fprintf(fp, "\t\t\t<param name=\"fetch_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "\t\t\t<param name=\"number_instruction_fetch_ports\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_ICACHE_READ_PORTS->getValue());
	fprintf(fp, "\t\t\t<param name=\"decode_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "\t\t\t<param name=\"issue_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "\t\t\t<param name=\"peak_issue_width\" value=\"%d\"/>\n", max((int)m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue(), (int)m_simBase->m_knobs->KNOB_ICACHE_READ_PORTS->getValue()));	// fixme 
	fprintf(fp, "\t\t\t<param name=\"commit_width\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());	// fixme: gpu:# threads 
	fprintf(fp, "\t\t\t<param name=\"fp_issue_width\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"prediction_width\" value=\"1\"/>\n"); 
	fprintf(fp, "\t\t\t<param name=\"pipelines_per_core\" value=\"1,1\"/>\n");
	int pipeline_depth = m_simBase->m_knobs->KNOB_LARGE_CORE_FETCH_LATENCY->getValue()+ m_simBase->m_knobs->KNOB_LARGE_CORE_ALLOC_LATENCY->getValue() + 2;
	fprintf(fp, "\t\t\t<param name=\"pipeline_depth\" value=\"%d,%d\"/>\n", pipeline_depth, pipeline_depth);
	fprintf(fp, "\t\t\t<param name=\"ALU_per_core\" value=\"5\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"MUL_per_core\" value=\"1\"/>\n");    
	fprintf(fp, "\t\t\t<param name=\"FPU_per_core\" value=\"1\"/>\n");    
	fprintf(fp, "\t\t\t<param name=\"instruction_buffer_size\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue() * m_simBase->m_knobs->KNOB_LARGE_CORE_ALLOC_LATENCY->getValue());
	fprintf(fp, "\t\t\t<param name=\"decoded_stream_buffer_size\" value=\"%d\"/>\n", 3 * m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "\t\t\t<param name=\"instruction_window_scheme\" value=\"1\"/>\n");
	int instruction_window_size = m_simBase->m_knobs->KNOB_ISCHED_LARGE_SIZE->getValue()+m_simBase->m_knobs->KNOB_MSCHED_LARGE_SIZE->getValue()+ m_simBase->m_knobs->KNOB_FSCHED_LARGE_SIZE->getValue();
	fprintf(fp, "\t\t\t<param name=\"instruction_window_size\" value=\"%d\"/>\n", instruction_window_size);
	fprintf(fp, "\t\t\t<param name=\"fp_instruction_window_size\" value=\"%d\"/>\n", instruction_window_size);
	fprintf(fp, "\t\t\t<param name=\"ROB_size\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_ROB_LARGE_SIZE->getValue());
	fprintf(fp, "\t\t\t<param name=\"archi_Regs_IRF_size\" value=\"8\"/>\n");    
	fprintf(fp, "\t\t\t<param name=\"archi_Regs_FRF_size\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"phy_Regs_IRF_size\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_INT_REGFILE_SIZE->getValue());
	fprintf(fp, "\t\t\t<param name=\"phy_Regs_FRF_size\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_FP_REGFILE_SIZE->getValue());
	fprintf(fp, "\t\t\t<param name=\"rename_scheme\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"register_windows_size\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"LSU_order\" value=\"inorder\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"store_buffer_size\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_STORE_BUF_SIZE->getValue());
	fprintf(fp, "\t\t\t<param name=\"load_buffer_size\" value=\"32\"/>\n", m_simBase->m_knobs->KNOB_LOAD_BUF_SIZE->getValue()); 
	fprintf(fp, "\t\t\t<param name=\"memory_ports\" value=\"1\"/>\n");  
	fprintf(fp, "\t\t\t<param name=\"RAS_size\" value=\"16\"/>\n");    
	fprintf(fp, "\t\t\t<stat name=\"total_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount());
	int num_int_inst = m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount() - m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();	
	fprintf(fp, "\t\t\t<stat name=\"int_instructions\" value=\"%d\"/>\n", num_int_inst);	// fixme
	fprintf(fp, "\t\t\t<stat name=\"fp_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());	// fixme: #inst? or #uop?
	fprintf(fp, "\t\t\t<stat name=\"branch_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_BR_PRED_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"branch_mispredictions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[BP_ON_PATH_MISPREDICT-PER_CORE_STATS_ENUM_FIRST].getCount() + m_simBase->m_ProcessorStats->core(core_id)[BP_OFF_PATH_MISPREDICT-PER_CORE_STATS_ENUM_FIRST].getCount());	// fixme
	fprintf(fp, "\t\t\t<stat name=\"load_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_LOAD_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"store_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_STORE_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"committed_instructions\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"committed_int_instructions\" value=\"%d\"/>\n", num_int_inst);	// fixme: now only allow the completed? execution
	fprintf(fp, "\t\t\t<stat name=\"committed_fp_instructions\" value=\"0\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());	// fixme: = committed_int_instructions
	fprintf(fp, "\t\t\t<stat name=\"pipeline_duty_cycle\" value=\"1\"/>\n");	// fixme
	fprintf(fp, "\t\t\t<stat name=\"total_cycles\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"idle_cycles\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"busy_cycles\"  value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"ROB_reads\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"ROB_writes\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"rename_reads\" value=\"0\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"rename_writes\" value=\"0\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_W-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"fp_rename_reads\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"fp_rename_writes\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"inst_window_reads\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"inst_window_writes\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"inst_window_wakeup_accesses\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"fp_inst_window_reads\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"fp_inst_window_writes\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"fp_inst_window_wakeup_accesses\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"int_regfile_reads\" value=\"%d\"/>\n", num_int_inst*2);
	fprintf(fp, "\t\t\t<stat name=\"float_regfile_reads\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount()*2);
	fprintf(fp, "\t\t\t<stat name=\"int_regfile_writes\" value=\"%d\"/>\n", num_int_inst);
	fprintf(fp, "\t\t\t<stat name=\"float_regfile_writes\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"function_calls\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"context_switches\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"ialu_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"fpu_accesses\" value=\"0\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"mul_accesses\" value=\"0\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());

	// fixme
	fprintf(fp, "\t\t\t<stat name=\"cdb_alu_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"cdb_mul_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"cdb_fpu_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t<stat name=\"IFU_duty_cycle\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"LSU_duty_cycle\" value=\"0.5\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"MemManU_I_duty_cycle\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"MemManU_D_duty_cycle\" value=\"0.5\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"ALU_duty_cycle\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"MUL_duty_cycle\" value=\"0.3\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"FPU_duty_cycle\" value=\"0.3\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"ALU_cdb_duty_cycle\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"MUL_cdb_duty_cycle\" value=\"0.3\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"FPU_cdb_duty_cycle\" value=\"0.3\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_of_BPT\" value=\"2\"/>\n");
	// fixme


	fprintf(fp, "\t\t\t<component id=\"system.core%d.predictor\" name=\"PBT\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"local_predictor_size\" value=\"10,3\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"local_predictor_entries\" value=\"%d\"/>\n", (int)pow(2, m_simBase->m_knobs->KNOB_BP_HIST_LENGTH->getValue()));
	fprintf(fp, "\t\t\t\t<param name=\"global_predictor_entries\" value=\"%d\"/>\n", (int)pow(2, m_simBase->m_knobs->KNOB_BP_HIST_LENGTH->getValue()));
	fprintf(fp, "\t\t\t\t<param name=\"global_predictor_bits\" value=\"2\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"chooser_predictor_entries\" value=\"%d\"/>\n", (int)pow(2, m_simBase->m_knobs->KNOB_BP_HIST_LENGTH->getValue()));
	fprintf(fp, "\t\t\t\t<param name=\"chooser_predictor_bits\" value=\"2\"/>\n");
	fprintf(fp, "\t\t\t</component>\n");
	
	// no itlb
	
	fprintf(fp, "\t\t\t<component id=\"system.core%d.itlb\" name=\"itlb\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"number_entries\" value=\"128\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"total_accesses\" value=\"m5sim_core_itlb_total_accesses\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"total_misses\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t</component>\n");
	
	fprintf(fp, "\t\t\t<component id=\"system.core%d.icache\" name=\"icache\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"icache_config\" value=\"%d,%d,%d,%d,%d,%d,%d,%d\"/>\n", 2*m_simBase->m_knobs->KNOB_ICACHE_LARGE_NUM_SET->getValue()*1024, m_simBase->m_knobs->KNOB_32_64_ISA->getValue(), m_simBase->m_knobs->KNOB_ICACHE_LARGE_ASSOC->getValue(), m_simBase->m_knobs->KNOB_ICACHE_LARGE_BANKS->getValue(), 2, m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY->getValue(), 32, 0 );	// jieun: 2* should be removed 
	//fprintf(fp, "\t\t\t\t<param name=\"icache_config\" value=\"%d,%d,%d,%d,%d,%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_ICACHE_LARGE_NUM_SET->getValue()*1024, m_simBase->m_knobs->KNOB_32_64_ISA->getValue(), m_simBase->m_knobs->KNOB_ICACHE_LARGE_ASSOC->getValue(), m_simBase->m_knobs->KNOB_ICACHE_LARGE_BANKS->getValue(), 2, m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY->getValue(), 32, 0 );
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"%d,%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE->getValue(), 0, 0, 0);
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"%d\"/>\n", 0); // fixme
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t</component>\n");
	fprintf(fp, "\t\t\t<component id=\"system.core%d.dtlb\" name=\"dtlb\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"number_entries\" value=\"128\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"total_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_DATA_TLB_R-PER_CORE_STATS_ENUM_FIRST].getCount() );	// fixme
	fprintf(fp, "\t\t\t\t<stat name=\"total_misses\" value=\"%d\"/>\n", 0);
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t</component>\n");
	fprintf(fp, "\t\t\t<component id=\"system.core%d.dcache\" name=\"dcache\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"dcache_config\" value=\"%d,%d,%d,%d,%d,%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_L1_LARGE_NUM_SET->getValue()*1024, 32, m_simBase->m_knobs->KNOB_L1_LARGE_ASSOC->getValue(), m_simBase->m_knobs->KNOB_L1_LARGE_NUM_BANK->getValue(), 2, m_simBase->m_knobs->KNOB_L1_LARGE_LATENCY->getValue(), 16, 1 );
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"%d, %d, %d, %d\"/>\n", m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE->getValue(), 8, 0, 8);
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"%d\"/>\n", 0);	// fixme
	fprintf(fp, "\t\t\t\t<stat name=\"write_misses\" value=\"%d\"/>\n", 0);	// fixme
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t</component>\n");
	fprintf(fp, "\t\t\t<param name=\"number_of_BTB\" value=\"2\"/>\n");
	fprintf(fp, "\t\t\t<component id=\"system.core%d.BTB\" name=\"BTB\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"BTB_config\" value=\"8192,4,2,1, 1,3\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"%d\"/>\n", 0);	// fixme
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"%d\"/>\n", 0);	// fixme
	fprintf(fp, "\t\t\t</component>\n");
	fprintf(fp, "\t</component>\n");
	// ================ End of Core


	// no L1/L2 Directories
	
	fprintf(fp, "\t\t<component id=\"system.L1Directory0\" name=\"L1Directory0\">\n");
	fprintf(fp, "\t\t\t\t<param name=\"Directory_type\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"Dir_config\" value=\"4096,2,0,1,100,100, 8\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"8, 8, 8, 8\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"clockrate\" value=\"3400\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"ports\" value=\"1,1,1\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"device_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"800000\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"27276\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"1632\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"write_misses\" value=\"183\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"20\"/>\n");
	fprintf(fp, "\t\t</component>\n");
	fprintf(fp, "\t\t<component id=\"system.L2Directory0\" name=\"L2Directory0\">\n");
	fprintf(fp, "\t\t\t\t<param name=\"Directory_type\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"Dir_config\" value=\"1048576,16,16,1,2, 100\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"8, 8, 8, 8\"/> \n");
	fprintf(fp, "\t\t\t\t<param name=\"clockrate\" value=\"3500\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"ports\" value=\"1,1,1\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"device_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"58824\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"27276\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"1632\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"write_misses\" value=\"183\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"100\"/>\n");    
	fprintf(fp, "\t\t</component>\n");
	

	fprintf(fp, "\t\t<component id=\"system.L2%d\" name=\"L20\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"L2_config\" value=\"%d,%d,%d,%d,%d,%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_L2_LARGE_NUM_SET->getValue()*1024, 32, m_simBase->m_knobs->KNOB_L2_LARGE_ASSOC->getValue(), m_simBase->m_knobs->KNOB_L2_LARGE_NUM_BANK->getValue(), 2, m_simBase->m_knobs->KNOB_L2_LARGE_LATENCY->getValue(), 16, 1 );
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"%d, %d, %d, %d\"/>\n", m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE->getValue(), 16, 16, 16 );
	fprintf(fp, "\t\t\t\t<param name=\"clockrate\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue() *1000));
	fprintf(fp, "\t\t\t\t<param name=\"ports\" value=\"%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_L2_READ_PORTS->getValue(), m_simBase->m_knobs->KNOB_L2_WRITE_PORTS->getValue(), 0 );
	fprintf(fp, "\t\t\t\t<param name=\"device_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_L2CACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_L2CACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"%d\"/>\n", 0);
	fprintf(fp, "\t\t\t\t<stat name=\"write_misses\" value=\"%d\"/>\n", 0);
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>  \n");
	fprintf(fp, "\t\t\t\t<stat name=\"duty_cycle\" value=\"1.0\"/>  \n");
	fprintf(fp, "\t\t</component>\n");
	fprintf(fp, "\t\t<component id=\"system.L3%d\" name=\"L30\">\n", core_id);
	fprintf(fp, "\t\t\t\t<param name=\"L3_config\" value=\"%d,%d,%d,%d,%d,%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_L3_NUM_SET->getValue()*1024, 32, m_simBase->m_knobs->KNOB_L3_ASSOC->getValue(), m_simBase->m_knobs->KNOB_L3_NUM_BANK->getValue(), 2, m_simBase->m_knobs->KNOB_L3_LATENCY->getValue(), 16, 1 );
	fprintf(fp, "\t\t\t\t<param name=\"clockrate\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue() *1000));
	fprintf(fp, "\t\t\t\t<param name=\"ports\" value=\"%d,%d,%d\"/>\n", m_simBase->m_knobs->KNOB_L3_READ_PORTS->getValue(), m_simBase->m_knobs->KNOB_L3_WRITE_PORTS->getValue(), 0 );
	fprintf(fp, "\t\t\t\t<param name=\"device_type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t\t<param name=\"buffer_sizes\" value=\"%d, %d, %d, %d\"/>\n", m_simBase->m_knobs->KNOB_MEM_MSHR_SIZE->getValue(), 16, 16, 16 );
	fprintf(fp, "\t\t\t\t<stat name=\"read_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_L3CACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"write_accesses\" value=\"%lld\"/>\n", m_simBase->m_ProcessorStats->core(core_id)[POWER_L3CACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount());
	fprintf(fp, "\t\t\t\t<stat name=\"read_misses\" value=\"%d\"/>\n", 0);
	fprintf(fp, "\t\t\t\t<stat name=\"write_misses\" value=\"%d\"/>\n", 0);
	fprintf(fp, "\t\t\t\t<stat name=\"conflicts\" value=\"0\"/>  \n");
	fprintf(fp, "\t\t</component>\n");

	fprintf(fp, "\t\t<component id=\"system.NoC0\" name=\"noc0\">\n");
	fprintf(fp, "\t\t\t<param name=\"clockrate\" value=\"3400\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"horizontal_nodes\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"vertical_nodes\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"has_global_link\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"link_throughput\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"link_latency\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"input_ports\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"output_ports\" value=\"1\"/>\n");
	//fprintf(fp, "\t\t\t<param name=\"virtual_channel_per_port\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"flit_bits\" value=\"256\"/>\n");
	//fprintf(fp, "\t\t\t<param name=\"input_buffer_entries_per_vc\" value=\"0\"/>\n");
	//fprintf(fp, "\t\t\t<param name=\"ports_of_input_buffer\" value=\"1,1,0\"/>\n");
	//printf(fp, "\t\t\t<param name=\"dual-pump\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"chip_coverage\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"link_routing_over_percentage\" value=\"0.5\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"total_accesses\" value=\"1052\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"duty_cycle\" value=\"1\"/>\n");
	fprintf(fp, "\t\t</component>\n");

	fprintf(fp, "\t\t<component id=\"system.mem\" name=\"mem\">\n");
	fprintf(fp, "\t\t\t<param name=\"mem_tech_node\" value=\"32\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"device_clock\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_DRAM_FREQUENCY->getValue() *1000));
	fprintf(fp, "\t\t\t<param name=\"peak_transfer_rate\" value=\"6400\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"internal_prefetch_of_DRAM_chip\" value=\"4\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"capacity_per_channel\" value=\"4096\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_ranks\" value=\"2\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"num_banks_of_DRAM_chip\" value=\"%d\"/>\n", m_simBase->m_knobs->KNOB_DRAM_NUM_BANKS->getValue());
	fprintf(fp, "\t\t\t<param name=\"Block_width_of_DRAM_chip\" value=\"64\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"output_width_of_DRAM_chip\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"page_size_of_DRAM_chip\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"burstlength_of_DRAM_chip\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_accesses\" value=\"1000\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_reads\" value=\"500\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_writes\" value=\"500\"/>\n");
	fprintf(fp, "\t\t</component>\n");
	fprintf(fp, "\t\t<component id=\"system.mc\" name=\"mc\">\n");
	fprintf(fp, "\t\t\t<param name=\"type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"mc_clock\" value=\"%d\"/>\n", (int)(m_simBase->m_knobs->KNOB_DRAM_FREQUENCY->getValue() * 1000));
	fprintf(fp, "\t\t\t<param name=\"peak_transfer_rate\" value=\"3200\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"block_size\" value=\"64\"/>\n");
	//fprintf(fp, "\t\t\t<param name=\"llc_line_length\" value=\"64\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_mcs\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"memory_channels_per_mc\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_ranks\" value=\"2\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"withPHY\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"req_window_size_per_channel\" value=\"32\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"IO_buffer_size_per_channel\" value=\"32\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"databus_width\" value=\"128\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"addressbus_width\" value=\"51\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_accesses\" value=\"1000\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_reads\" value=\"500\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"memory_writes\" value=\"500\"/>\n");
	fprintf(fp, "\t\t</component>\n");

	fprintf(fp, "\t\t<component id=\"system.niu\" name=\"niu\">\n");
	fprintf(fp, "\t\t\t<param name=\"type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"clockrate\" value=\"350\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_units\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"duty_cycle\" value=\"1.0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"total_load_perc\" value=\"0.7\"/>\n");
	fprintf(fp, "\t\t</component>\n");
	fprintf(fp, "\t\t<component id=\"system.pcie\" name=\"pcie\">\n");
	fprintf(fp, "\t\t\t<param name=\"type\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"withPHY\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"clockrate\" value=\"350\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"number_units\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"num_channels\" value=\"8\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"duty_cycle\" value=\"1.0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"total_load_perc\" value=\"0.7\"/>\n");
	fprintf(fp, "\t\t</component>\n");
	fprintf(fp, "\t\t<component id=\"system.flashc\" name=\"flashc\">\n");
	fprintf(fp, "\t\t\t<param name=\"number_flashcs\" value=\"0\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"type\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"withPHY\" value=\"1\"/>\n");
	fprintf(fp, "\t\t\t<param name=\"peak_transfer_rate\" value=\"200\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"duty_cycle\" value=\"1.0\"/>\n");
	fprintf(fp, "\t\t\t<stat name=\"total_load_perc\" value=\"0.7\"/>\n");
	fprintf(fp, "\t\t</component>\n");


	fprintf(fp, "\t</component>\n");
	fprintf(fp, "</component>\n");
}

void ei_power_c::mcpat_config_gen_top()
{

	FILE* fp = fopen(CONFIG_FILE_NAME, "w");
	mcpat_config_gen_tech(fp, 0);
	mcpat_config_gen_large(fp, 0);
	fclose(fp);
}	

// EI main
/*
   void ei_power_c::ei_main()
   {
   int core_id;
   double cur_time;
   double period;
   double total_power_rt;
   double total_power_peak;

   energy_introspector_t *energy_introspector = new energy_introspector_t("ei.config");

// test counters 
counters_t                  cnt_byteQ;
counters_t                  cnt_instQ;
counters_t                  cnt_bpred1;
counters_t                  cnt_IC;
counters_t                  cnt_IC_missbuffer;
counters_t                  cnt_instruction_decoder;
counters_t                  cnt_uop_sequencer;
counters_t                  cnt_operand_decoder;
	counters_t                  cnt_uopQ;
	counters_t                  cnt_RAT;
	counters_t                  cnt_operand_dependency;
	counters_t                  cnt_freelist;
	counters_t                  cnt_GPREG;
	counters_t                  cnt_SEGREG;
	counters_t                  cnt_CREG;
	counters_t                  cnt_FLAGREG;
	counters_t                  cnt_FPREG;
	counters_t                  cnt_ARF2RS;
	counters_t                  cnt_RS;
	counters_t                  cnt_issue_select;
	counters_t                  cnt_payload;
	counters_t                  cnt_FU0;
	counters_t                  cnt_FU1;
	counters_t                  cnt_FU2;
	counters_t                  cnt_FU2ROB;
	counters_t                  cnt_ROB;
	counters_t                  cnt_commit_select;
	counters_t                  cnt_loadQ;
	counters_t                  cnt_storeQ;
	counters_t                  cnt_DTLB;
	counters_t                  cnt_DC;
	counters_t                  cnt_DC_missbuffer;
	counters_t                  cnt_DC_linefill;
	counters_t                  cnt_DC_writeback;
	counters_t                  cnt_L2;
	counters_t                  cnt_L3;
	counters_t                  cnt_MC;

	int gen_reg;
	int seg_reg;
	int con_reg;

	total_power_rt = 0;
	total_power_peak = 0;
	
	for(core_id=0; core_id<*m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES; core_id++)
	{
		cur_time = (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue()/1000000000); 
		period= (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue()/1000000000); 
		cout << "period:" << period << endl;

		gen_reg = m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount() - m_simBase->m_ProcessorStats->core(core_id)[POWER_CONTROL_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount() - m_simBase->m_ProcessorStats->core(core_id)[POWER_SEGMENT_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();

		cnt_byteQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_FETCH_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_bpred1.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_BR_PRED_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC_missbuffer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_MISS_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instruction_decoder.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_DECODER_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_uop_sequencer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_MICRO_OP_SEQ_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		//cnt_operand_decoder.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_OPERAND_DECODER_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_decoder.read = cnt_instruction_decoder.read;
		cnt_uopQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_UOP_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_RAT.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_dependency.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DEP_CHECK_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_freelist.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_GPREG.read = gen_reg; 
		cnt_SEGREG.read = 0;
		cnt_FLAGREG.read = 0;
		cnt_CREG.read = 0;
		cnt_ARF2RS.read= cnt_GPREG.read+ cnt_SEGREG.read+ cnt_CREG.read+ cnt_FLAGREG.read;
		cnt_RS.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_RESERVATION_STATION_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_issue_select.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_ISSUE_SEL_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_payload.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_PAYLOAD_RAM_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU0.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU1.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2ROB.read = cnt_FU0.read + cnt_FU1.read + cnt_FU2.read;
		cnt_ROB.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_commit_select.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_COMMIT_SEL_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_loadQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_LOAD_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_storeQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_STORE_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DTLB.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DATA_TLB_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.read_tag = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_R_TAG-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_missbuffer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_MISS_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_linefill.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_LINEFILL_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_writeback.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_WB_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L2.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_L2CACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L2.read_tag = m_simBase->m_ProcessorStats->core(core_id)[POWER_L2CACHE_R_TAG-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L3.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_L3CACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L3.read_tag = m_simBase->m_ProcessorStats->core(core_id)[POWER_L3CACHE_R_TAG-PER_CORE_STATS_ENUM_FIRST].getCount();
	
		// ---------------- write -----------------
		cnt_byteQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FETCH_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		//cnt_bpred1.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_BR_PRED_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_bpred1.write = cnt_bpred1.read;
		cnt_IC.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC_missbuffer.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_MISS_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		//cnt_instruction_decoder.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_DECODER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_decoder.write = cnt_instruction_decoder.write;
		
		cnt_uopQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_UOP_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_RAT.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_dependency.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DEP_CHECK_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_freelist.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_GPREG.write = m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_SEGREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_SEGMENT_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_CREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_CONTROL_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FLAGREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FPREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_ARF2RS.write = cnt_GPREG.write + cnt_SEGREG.write + cnt_CREG.write + cnt_FLAGREG.write;
		cnt_RS.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_RESERVATION_STATION_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_issue_select.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_ISSUE_SEL_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_payload.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_PAYLOAD_RAM_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU0.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU1.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2ROB.write= cnt_FU0.write + cnt_FU1.write + cnt_FU2.write;
		cnt_ROB.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_commit_select.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_COMMIT_SEL_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_loadQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_LOAD_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_storeQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_STORE_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DTLB.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DATA_TLB_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.write_tag = cnt_DC.write;
		cnt_DC_missbuffer.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_MISS_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_linefill.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_LINEFILL_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_writeback.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_WB_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L2.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_L2CACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L2.write_tag = cnt_L2.write;
		cnt_L3.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_L3CACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_L3.write_tag = cnt_L3.write;

	
	energy_introspector->compute_power(cur_time, period, "core0:fetch", cnt_byteQ);
	energy_introspector->compute_power(cur_time, period, "core0:program_counter", cnt_byteQ);	// fixme: counter
	energy_introspector->compute_power(cur_time, period, "core0:byteQ", cnt_byteQ);
	energy_introspector->compute_power(cur_time, period, "core0:BQ2IQ", cnt_byteQ);
	energy_introspector->compute_power(cur_time, period, "core0:instQ", cnt_instQ);
	energy_introspector->compute_power(cur_time, period, "core0:IQ2ID", cnt_instQ);
	energy_introspector->compute_power(cur_time, period, "core0:bpred1", cnt_bpred1);
	energy_introspector->compute_power(cur_time, period, "core0:I$", cnt_IC);
	energy_introspector->compute_power(cur_time, period, "core0:I$:missbuffer", cnt_IC_missbuffer);
	energy_introspector->compute_power(cur_time, period, "core0:decode", cnt_instruction_decoder);	// fixme: counter
	energy_introspector->compute_power(cur_time, period, "core0:instruction_decoder", cnt_instruction_decoder);
	energy_introspector->compute_power(cur_time, period, "core0:uop_sequencer", cnt_uop_sequencer);
	energy_introspector->compute_power(cur_time, period, "core0:operand_decoder", cnt_operand_decoder);
	energy_introspector->compute_power(cur_time, period, "core0:ID2uQ", cnt_instruction_decoder);
	energy_introspector->compute_power(cur_time, period, "core0:uopQ", cnt_uopQ);
	energy_introspector->compute_power(cur_time, period, "core0:uQ2RR", cnt_uopQ);
	energy_introspector->compute_power(cur_time, period, "core0:schedule", cnt_RAT);	// fixme: counter
	energy_introspector->compute_power(cur_time, period, "core0:RAT", cnt_RAT);
	energy_introspector->compute_power(cur_time, period, "core0:RR2RS", cnt_RAT);
	energy_introspector->compute_power(cur_time, period, "core0:operand_dependency", cnt_operand_dependency);
	energy_introspector->compute_power(cur_time, period, "core0:freelist", cnt_freelist);
	energy_introspector->compute_power(cur_time, period, "core0:GPREG", cnt_GPREG);
	energy_introspector->compute_power(cur_time, period, "core0:SEGREG", cnt_SEGREG);
	energy_introspector->compute_power(cur_time, period, "core0:CREG", cnt_CREG);
	energy_introspector->compute_power(cur_time, period, "core0:FLAGREG", cnt_FLAGREG);
	energy_introspector->compute_power(cur_time, period, "core0:FPREG", cnt_FPREG);
	energy_introspector->compute_power(cur_time, period, "core0:ARF2RS", cnt_ARF2RS);
	energy_introspector->compute_power(cur_time, period, "core0:RS", cnt_RS);
	energy_introspector->compute_power(cur_time, period, "core0:RS2PR", cnt_RS);
	energy_introspector->compute_power(cur_time, period, "core0:issue_select", cnt_issue_select);
	energy_introspector->compute_power(cur_time, period, "core0:execute", cnt_payload);		// fixme: counter
	energy_introspector->compute_power(cur_time, period, "core0:payload", cnt_payload);
	energy_introspector->compute_power(cur_time, period, "core0:PR2FU", cnt_payload);
	energy_introspector->compute_power(cur_time, period, "core0:port0:FU", cnt_FU0);
	energy_introspector->compute_power(cur_time, period, "core0:port1:FU", cnt_FU1);
	energy_introspector->compute_power(cur_time, period, "core0:port2:FU", cnt_FU2);
	energy_introspector->compute_power(cur_time, period, "core0:FU2ROB", cnt_FU2ROB);
	energy_introspector->compute_power(cur_time, period, "core0:ROB", cnt_ROB);
	energy_introspector->compute_power(cur_time, period, "core0:ROB2RS", cnt_ROB);
	energy_introspector->compute_power(cur_time, period, "core0:ROB2CM", cnt_ROB);
	energy_introspector->compute_power(cur_time, period, "core0:commit_select", cnt_commit_select);
	energy_introspector->compute_power(cur_time, period, "core0:memory", cnt_loadQ);	// fixme: counter
	energy_introspector->compute_power(cur_time, period, "core0:loadQ", cnt_loadQ);
	energy_introspector->compute_power(cur_time, period, "core0:LQ2ROB", cnt_loadQ);
	energy_introspector->compute_power(cur_time, period, "core0:LQ2L1", cnt_loadQ);
	energy_introspector->compute_power(cur_time, period, "core0:storeQ", cnt_storeQ);
	energy_introspector->compute_power(cur_time, period, "core0:SQ2L1", cnt_storeQ);
	energy_introspector->compute_power(cur_time, period, "core0:SQ2LQ", cnt_storeQ);
	energy_introspector->compute_power(cur_time, period, "core0:DTLB", cnt_DTLB);
	energy_introspector->compute_power(cur_time, period, "core0:D$", cnt_DC);
	energy_introspector->compute_power(cur_time, period, "core0:D$:missbuffer", cnt_DC_missbuffer);
	energy_introspector->compute_power(cur_time, period, "core0:D$:linefill", cnt_DC_linefill);
	energy_introspector->compute_power(cur_time, period, "core0:D$:writeback", cnt_DC_writeback);
	energy_introspector->compute_power(cur_time, period, "core0:L12LQ", cnt_DC);
	//for(mc_i=0; mc_i < m_simBase->m_knobs->KNOB_DRAM_NUM_MC; mc_i++)
	//{
	energy_introspector->compute_power(cur_time, period, "core0:MemCon0", cnt_DC_linefill);
	//}
	energy_introspector->compute_power(cur_time, period, "core0:L2", cnt_L3);
	energy_introspector->compute_power(cur_time, period, "core0:L3:0", cnt_L3);
	energy_introspector->compute_power(cur_time, period, "core0:L3:1", cnt_L3);
	energy_introspector->compute_power(cur_time, period, "core0:L3:2", cnt_L3);
	energy_introspector->compute_power(cur_time, period, "core0:L3:3", cnt_L3);

	cout << "==============================" << endl;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:fetch", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:program_counter", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:byteQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:BQ2IQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:IQ2ID", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:bpred1", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$:missbuffer", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:decode", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instruction_decoder", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uop_sequencer", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_decoder", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ID2uQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uopQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uQ2RR", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:schedule", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RAT", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RR2RS", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_dependency", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:freelist", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:GPREG", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SEGREG", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:CREG", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FLAGREG", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FPREG", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ARF2RS", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RS", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RS2PR", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:issue_select", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:execute", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:payload", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:PR2FU", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port0:FU", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port1:FU", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port2:FU", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FU2ROB", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB2RS", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB2CM", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:commit_select", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:memory", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:loadQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:LQ2ROB", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:LQ2L1", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:storeQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SQ2L1", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SQ2LQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:DTLB", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:missbuffer", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:linefill", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:writeback", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L12LQ", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:MemCon0", "power").total;
	// fixme	should use for loop
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L2", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:0", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:1", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:2", "power").total;
	total_power_rt += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:3", "power").total;

	}
	cout << "total_power_rt = " << total_power_rt << "W" << endl;

	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:fetch", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:program_counter", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:byteQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:BQ2IQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:IQ2ID", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:bpred1", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$:missbuffer", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:decode", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instruction_decoder", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uop_sequencer", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_decoder", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ID2uQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uopQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uQ2RR", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:schedule", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RAT", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RR2RS", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_dependency", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:freelist", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:GPREG", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SEGREG", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:CREG", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FLAGREG", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FPREG", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ARF2RS", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RS", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RS2PR", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:issue_select", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:execute", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:payload", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:PR2FU", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port0:FU", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port1:FU", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port2:FU", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FU2ROB", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB2RS", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB2CM", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:commit_select", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:memory", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:loadQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:LQ2ROB", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:LQ2L1", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:storeQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SQ2L1", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SQ2LQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:DTLB", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:missbuffer", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:linefill", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:writeback", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L12LQ", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:MemCon0", "power").peak_total;
	// fixme	should use for loop
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L2", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:0", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:1", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:2", "power").peak_total;
	total_power_peak += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:L3:3", "power").peak_total;

	
	cout << "total_power_peak = " << total_power_peak << "W" << endl;
}
*/

// McPAT main
/*
void ei_power_c::ei_main()
{
	char *fb;
	//int plevel = 2;
	int plevel = m_simBase->m_knobs->KNOB_POWER_PRINT_LEVEL->getValue();
	opt_for_clk = true;

	fb = CONFIG_FILE_NAME;
	//fb = "Xeon.xml";

	//parse XML-based interface
	ParseXML *p1= new ParseXML();
	p1->parse(fb);
	Processor proc(p1);
	proc.displayEnergy(2, plevel);
	delete p1; 
	//return 0;
}
*/

void ei_power_c::ei_main()
{
	energy_introspector_t *energy_introspector = new energy_introspector_t("new_ei.config");
}



