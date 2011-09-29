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
	fprintf(fp, "# large core\n");
	fprintf(fp, "# technology\n");
    fprintf(fp, "-technology.ID							test_tech\n");	
    fprintf(fp, "-technology.clock_frequency				%.2fe9 \n", m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue());
    fprintf(fp, "-technology.feature_size				32e-9\n");
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
	fprintf(fp, "-package.grid_rows						32\n");
	fprintf(fp, "-package.grid_columns					32\n");
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
	fprintf(fp, "-partition.width					1e-3\n");
	fprintf(fp, "-partition.length					1e-3\n");
	fprintf(fp, "-partition.layer					0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module					core%d:byteQ\n", core_id);
	fprintf(fp, "-partition.module					core%d:instQ\n", core_id);
	fprintf(fp, "-partition.module					core%d:bpred1\n", core_id);
	fprintf(fp, "-partition.module					core%d:I$\n", core_id);
	fprintf(fp, "-partition.module					core%d:I$:missbuffer\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID							core%d:decode\n", core_id);
	fprintf(fp, "-partition.x_left				0.0\n");
	fprintf(fp, "-partition.y_bottom				1e-3\n");
	fprintf(fp, "-partition.width				1e-3\n");
	fprintf(fp, "-partition.length				1e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module				core%d:instruction_decoder\n", core_id);
	fprintf(fp, "-partition.module				core%d:uop_sequencer\n", core_id);
	fprintf(fp, "-partition.module				core%d:operand_decoder\n", core_id);
	//fprintf(fp, "-partition.module				core%d:ID2uQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:uopQ\n", core_id);
	//fprintf(fp, "-partition.module				core%d:uQ2RR\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:schedule\n", core_id);
	fprintf(fp, "-partition.x_left				1e-3\n");
	fprintf(fp, "-partition.y_bottom				0.0\n");
	fprintf(fp, "-partition.width				1e-3\n");
	fprintf(fp, "-partition.length				1e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module				core%d:RAT\n", core_id);
	fprintf(fp, "-partition.module				core%d:operand_dependency\n", core_id);
	fprintf(fp, "-partition.module				core%d:freelist\n", core_id);
	fprintf(fp, "-partition.module				core%d:GPREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:SEGREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:CREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:FLAGREG\n", core_id);
	fprintf(fp, "-partition.module				core%d:FPREG\n", core_id);
	//fprintf(fp, "-partition.module				core%d:RR2RS\n", core_id);
	//fprintf(fp, "-partition.module				core%d:ARF2RS\n", core_id);
	//fprintf(fp, "-partition.module				core%d:ROB2RS\n", core_id);
	fprintf(fp, "-partition.module				core%d:RS\n", core_id);
	//fprintf(fp, "-partition.module				core%d:RS2PR\n", core_id);
	fprintf(fp, "-partition.module				core%d:issue_select\n", core_id);
	fprintf(fp, "-partition.module				core%d:ROB\n", core_id);
	fprintf(fp, "-partition.module				core%d:commit_select\n", core_id);
	//fprintf(fp, "-partition.module				core%d:ROB2CM\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:memory\n", core_id);
	fprintf(fp, "-partition.x_left				1e-3\n");
	fprintf(fp, "-partition.y_bottom				1e-3\n");
	fprintf(fp, "-partition.width				1e-3\n");
	fprintf(fp, "-partition.length				1e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module				core%d:loadQ\n", core_id);
	//fprintf(fp, "-partition.module				core%d:LQ2ROB\n", core_id);
	//fprintf(fp, "-partition.module				core%d:LQ2L1\n", core_id);
	//fprintf(fp, "-partition.module				core%d:L12LQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:storeQ\n", core_id);
	//fprintf(fp, "-partition.module				core%d:SQ2L1\n", core_id);
	//fprintf(fp, "-partition.module				core%d:SQ2LQ\n", core_id);
	fprintf(fp, "-partition.module				core%d:DTLB\n", core_id);
	//fprintf(fp, "-partition.module				core%d:DTLB2\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:missbuffer\n", core_id);
	//fprintf(fp, "-partition.module				core%d:D$:prefetch\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:linefill\n", core_id);
	fprintf(fp, "-partition.module				core%d:D$:writeback\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "-partition.ID					core%d:execute\n", core_id);
	fprintf(fp, "-partition.x_left				2e-3\n");
	fprintf(fp, "-partition.y_bottom				0.0\n");
	fprintf(fp, "-partition.width				2e-3\n");
	fprintf(fp, "-partition.length				2e-3\n");
	fprintf(fp, "-partition.layer				0\n");
	fprintf(fp, "-partition.temperature				315\n");
	fprintf(fp, "-partition.module				core%d:payload\n", core_id);
	//fprintf(fp, "-partition.module				core%d:PR2FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port0:FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port1:FU\n", core_id);
	fprintf(fp, "-partition.module				core%d:port2:FU\n", core_id);
	//fprintf(fp, "-partition.module				core%d:FU2ROB\n", core_id);
	fprintf(fp, "-partition.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#fetch queue\n");
	fprintf(fp, "-module.ID					core%d:byteQ\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				ram\n");
	fprintf(fp, "-module.line_width				4		\n");
	fprintf(fp, "-module.sets					%d\n", 3 * m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_LARGE_WIDTH->getValue());
	fprintf(fp, "-module.wr_ports				1\n");
	fprintf(fp, "-module.access_mode				sequential\n");
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
	fprintf(fp, "-module.line_width				32\n");
	fprintf(fp, "-module.assoc					4\n");
	fprintf(fp, "-module.sets					128\n");
	fprintf(fp, "-module.tag_width				37 # 32-bit virtual address + overhead\n");
	fprintf(fp, "-module.rd_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_READ_PORTS->getValue());
	fprintf(fp, "-module.wr_ports				%d\n", m_simBase->m_knobs->KNOB_DCACHE_WRITE_PORTS->getValue());
	fprintf(fp, "-module.access_time				2\n");
	fprintf(fp, "-module.cycle_time				2\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");
	fprintf(fp, "#data cache missbuffer\n");
	fprintf(fp, "-module.ID					core%d:D$:missbuffer\n", core_id);
	fprintf(fp, "-module.technology				test_tech\n");
	fprintf(fp, "-module.energy_library				McPAT\n");
	fprintf(fp, "-module.energy_model				array\n");
	fprintf(fp, "-module.energy_submodel				cache\n");
	fprintf(fp, "-module.line_width				32\n");
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
	fprintf(fp, "-module.line_width				32\n");
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
	fprintf(fp, "-module.line_width				32\n");
	fprintf(fp, "-module.assoc					0\n");
	fprintf(fp, "-module.sets					8\n");
	fprintf(fp, "-module.tag_width				37 \n");
	fprintf(fp, "-module.rw_ports				1\n");
	fprintf(fp, "-module.search_ports				1\n");
	fprintf(fp, "-module.end\n");
	fprintf(fp, "\n");


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


void ei_power_c::ei_main()
{
	int core_id;
	double cur_time;
	double period;
	double total_power;

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
	counters_t                  cnt_RS;
	counters_t                  cnt_issue_select;
	counters_t                  cnt_payload;
	counters_t                  cnt_FU0;
	counters_t                  cnt_FU1;
	counters_t                  cnt_FU2;
	counters_t                  cnt_ROB;
	counters_t                  cnt_commit_select;
	counters_t                  cnt_loadQ;
	counters_t                  cnt_storeQ;
	counters_t                  cnt_DTLB;
	counters_t                  cnt_DC;
	counters_t                  cnt_DC_missbuffer;
	counters_t                  cnt_DC_linefill;
	counters_t                  cnt_DC_writeback;

	int gen_reg;
	int seg_reg;
	int con_reg;

	int inst_interrupt;
	int inst_io;
	int inst_iostring;
	int inst_misc;
	int inst_ret;
	int inst_rotate;
	int inst_semaphore;
	int inst_syscall;
	int inst_sysret;
	int inst_system;
	int inst_vtx;
	int inst_xsave;

	total_power = 0;

	
	for(core_id=0; core_id<*m_simBase->m_knobs->KNOB_NUM_SIM_LARGE_CORES; core_id++)
	{
	
		cur_time = (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue()/1000000000); 
		period= (double)(m_simBase->m_ProcessorStats->core(core_id)[CYC_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount()/m_simBase->m_knobs->KNOB_CPU_FREQUENCY->getValue()/1000000000); 
		gen_reg = m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount() - m_simBase->m_ProcessorStats->core(core_id)[POWER_CONTROL_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount() - m_simBase->m_ProcessorStats->core(core_id)[POWER_SEGMENT_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();

		cnt_byteQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_FETCH_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_bpred1.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_BR_PRED_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC_missbuffer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_MISS_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instruction_decoder.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_DECODER_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_uop_sequencer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_MICRO_OP_SEQ_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_decoder.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_OPERAND_DECODER_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_uopQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_UOP_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_RAT.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_dependency.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DEP_CHECK_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_freelist.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_GPREG.read = gen_reg; 
		cnt_SEGREG.read = 0;
		cnt_FLAGREG.read = 0;
		cnt_CREG.read = 0;
		cnt_RS.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_RESERVATION_STATION_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_issue_select.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_ISSUE_SEL_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_payload.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_PAYLOAD_RAM_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU0.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU1.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_ROB.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_commit_select.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_COMMIT_SEL_LOGIC_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_loadQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_LOAD_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_storeQ.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_STORE_QUEUE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DTLB.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DATA_TLB_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_missbuffer.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_MISS_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_linefill.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_LINEFILL_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_writeback.read = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_WB_BUF_R-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_byteQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FETCH_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_bpred1.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_BR_PRED_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_IC_missbuffer.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_ICACHE_MISS_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_instruction_decoder.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_DECODER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_uop_sequencer.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_MICRO_OP_SEQ_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_decoder.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_OPERAND_DECODER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_uopQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_UOP_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_RAT.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_REG_RENAMING_TABLE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_operand_dependency.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DEP_CHECK_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_freelist.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_GPREG.write = m_simBase->m_ProcessorStats->core(core_id)[INST_COUNT-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_SEGREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_SEGMENT_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_CREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_CONTROL_REGISTER_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FLAGREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FPREG.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_FREELIST_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_RS.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_RESERVATION_STATION_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_issue_select.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_ISSUE_SEL_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_payload.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_PAYLOAD_RAM_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU0.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P0_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU1.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P1_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_FU2.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_EX_P2_FU_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_ROB.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_REORDER_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_commit_select.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_INST_COMMIT_SEL_LOGIC_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_loadQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_LOAD_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_storeQ.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_STORE_QUEUE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DTLB.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DATA_TLB_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_missbuffer.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_MISS_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_linefill.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_LINEFILL_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
		cnt_DC_writeback.write = m_simBase->m_ProcessorStats->core(core_id)[POWER_DCACHE_WB_BUF_W-PER_CORE_STATS_ENUM_FIRST].getCount();
	}
	
	energy_introspector->compute_power(cur_time, period, "core0:byteQ", cnt_byteQ);
	energy_introspector->compute_power(cur_time, period, "core0:instQ", cnt_instQ);
	energy_introspector->compute_power(cur_time, period, "core0:bpred1", cnt_bpred1);
	energy_introspector->compute_power(cur_time, period, "core0:I$", cnt_IC);
	energy_introspector->compute_power(cur_time, period, "core0:I$:missbuffer", cnt_IC_missbuffer);
	energy_introspector->compute_power(cur_time, period, "core0:instruction_decoder", cnt_instruction_decoder);
	energy_introspector->compute_power(cur_time, period, "core0:uop_sequencer", cnt_uop_sequencer);
	energy_introspector->compute_power(cur_time, period, "core0:operand_decoder", cnt_operand_decoder);
	energy_introspector->compute_power(cur_time, period, "core0:uopQ", cnt_uopQ);
	energy_introspector->compute_power(cur_time, period, "core0:RAT", cnt_RAT);
	energy_introspector->compute_power(cur_time, period, "core0:operand_dependency", cnt_operand_dependency);
	energy_introspector->compute_power(cur_time, period, "core0:freelist", cnt_freelist);
	energy_introspector->compute_power(cur_time, period, "core0:GPREG", cnt_GPREG);
	energy_introspector->compute_power(cur_time, period, "core0:SEGREG", cnt_SEGREG);
	energy_introspector->compute_power(cur_time, period, "core0:CREG", cnt_CREG);
	energy_introspector->compute_power(cur_time, period, "core0:FLAGREG", cnt_FLAGREG);
	energy_introspector->compute_power(cur_time, period, "core0:FPREG", cnt_FPREG);
	energy_introspector->compute_power(cur_time, period, "core0:RS", cnt_RS);
	energy_introspector->compute_power(cur_time, period, "core0:issue_select", cnt_issue_select);
	energy_introspector->compute_power(cur_time, period, "core0:payload", cnt_payload);
	energy_introspector->compute_power(cur_time, period, "core0:port0:FU", cnt_FU0);
	energy_introspector->compute_power(cur_time, period, "core0:port1:FU", cnt_FU1);
	energy_introspector->compute_power(cur_time, period, "core0:port2:FU", cnt_FU2);
	energy_introspector->compute_power(cur_time, period, "core0:ROB", cnt_ROB);
	energy_introspector->compute_power(cur_time, period, "core0:commit_select", cnt_commit_select);
	energy_introspector->compute_power(cur_time, period, "core0:loadQ", cnt_loadQ);
	energy_introspector->compute_power(cur_time, period, "core0:storeQ", cnt_storeQ);
	energy_introspector->compute_power(cur_time, period, "core0:DTLB", cnt_DTLB);
	energy_introspector->compute_power(cur_time, period, "core0:D$", cnt_DC);
	energy_introspector->compute_power(cur_time, period, "core0:D$:missbuffer", cnt_DC_missbuffer);
	energy_introspector->compute_power(cur_time, period, "core0:D$:linefill", cnt_DC_linefill);
	energy_introspector->compute_power(cur_time, period, "core0:D$:writeback", cnt_DC_writeback);

	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:byteQ", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instQ", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:bpred1", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:I$:missbuffer", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:instruction_decoder", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uop_sequencer", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_decoder", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:uopQ", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RAT", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:operand_dependency", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:freelist", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:GPREG", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:SEGREG", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:CREG", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FLAGREG", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:FPREG", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:RS", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:issue_select", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:payload", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port0:FU", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port1:FU", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:port2:FU", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:ROB", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:commit_select", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:loadQ", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:storeQ", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:DTLB", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:missbuffer", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:linefill", "power").total;
	total_power += energy_introspector->pull_data<power_t>(cur_time, "module", "core0:D$:writeback", "power").total;


	cout << "total_power = " << total_power << "W" << endl;
}
