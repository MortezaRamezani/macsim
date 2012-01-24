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


/*********************************************************************************
* File 			: ei_power.h
* Author 		: Jieun Lim
* Date			: 9/6/2011
* SVN			:
* Description 	: main file to use energy introspector 
*********************************************************************************/

#ifndef EI_POWER_H_INCLUDED
#define EI_POWER_H_INCLUDED

#include <string>
#include <unordered_map>

#include "macsim.h"
#include "global_defs.h"
#include "global_types.h"

#include "energy_introspector/energy_introspector.h"


#define MCPAT_GEN_CONFIG_DECL(type, fp, core_id) void mcpat_config_gen_tech_##type(FILE *fp, int core_id)

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
		void ei_config_gen_llc_tech(FILE* fp);
		void ei_config_gen_top();

		string get_name(string module_name, int core_id);

		void ei_main();

		
	public:
		int test_var;

	private:
		macsim_c* m_simBase;

};

#endif		// EI_TEST_H_INCLUDED

