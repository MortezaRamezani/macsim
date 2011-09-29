/*********************************************************************************
* File 			: ei_power.h
* Author 		: Jieun Lim
* Date			: 9/6/2011
* SVN			:
* Description 	: main file to use energy introspector 
*********************************************************************************/

#ifndef EI_TEST_H_INCLUDED
#define EI_TEST_H_INCLUDED

#include <string>
#include <unordered_map>

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

#include "energy_introspector/energy_introspector.h"

class ei_power_c
{
	public:
		ei_power_c(macsim_c* simBase);
		~ei_power_c();

		void ei_config_gen_large_tech(FILE* fp, int core_id);
		void ei_config_gen_large_mod(FILE* fp, int core_id);
		void ei_config_gen_medium_tech(FILE* fp, int core_id);
		void ei_config_gen_medium_mod(FILE* fp, int core_id);
		void ei_config_gen_small_tech(FILE* fp, int core_id);
		void ei_config_gen_small_mod(FILE* fp, int core_id);
		void ei_config_gen_top();
		void ei_main();
		
	public:
		int test_var;

	private:
		macsim_c* m_simBase;

};

#endif		// EI_TEST_H_INCLUDED

