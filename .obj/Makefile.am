# what flags you want to pass to the C compiler & linker

XFLAGS = -DLONG_COUNTERS
WARN_FLAGS = -Wunused-function -Wreturn-type -Wpointer-arith -Wno-write-strings #-Wall -Wno-sign-conversion -Wno-sign-compare
DEBUG_FLAGS = -DNO_DEBUG
COMMON_FLAGS = -DNO_MPI 

EI_DIR = energy_introspector
ORION_DIR = orion

DEP_LIBS = -L../src/$(ORION_DIR) -lorion
LIBS = -lz -lpthread $(DEP_LIBS)

AM_CXXFLAGS = -I ../src/$(EI_DIR)/ \
							-I../src/$(ORION_DIR) \
							-I../src/manifold/kernel/include/kernel \
							-I ../src/manifold/kernel/include/ \
							-I ../src/manifold/models/iris/interfaces \
							-I ../src -I /usr/include
AM_LDFLAGS = -lz -lpthread -lgsl -lgslc++

all_sources = \
	../src/all_knobs.cc \
	../src/all_stats.cc \
	../src/allocate.cc \
	../src/allocate_smc.cc \
	../src/bp.cc \
	../src/bp_targ.cc \
	../src/bp_gshare.cc \
	../src/bug_detector.cc \
	../src/cache.cc \
	../src/core.cc \
	../src/dram.cc \
	../src/exec.cc \
	../src/factory_class.cc \
	../src/fetch_factory.cc \
	../src/frontend.cc \
	../src/knob.cc \
	../src/macsim.cc \
	../src/main.cc \
	../src/map.cc \
	../src/memory.cc \
	../src/memreq_info.cc \
	../src/noc.cc \
	../src/port.cc \
	../src/pref.cc \
	../src/pref_common.cc \
	../src/pref_factory.cc \
	../src/pref_stride.cc \
	../src/process_manager.cc \
	../src/readonly_cache.cc \
	../src/retire.cc \
	../src/rob.cc \
	../src/rob_smc.cc \
	../src/schedule.cc \
	../src/schedule_io.cc \
	../src/schedule_ooo.cc \
	../src/schedule_smc.cc \
	../src/statistics.cc \
	../src/sw_managed_cache.cc \
	../src/trace_read.cc \
	../src/utils.cc \
	../src/uop.cc \
	../src/ei_power.cc 

iris_kernel_srcs = ../src/manifold/kernel/src/clock.cc \
                   ../src/manifold/kernel/src/component.cc \
                   ../src/manifold/kernel/src/link.cc \
                   ../src/manifold/kernel/src/manifold.cc \
                   ../src/manifold/kernel/src/messenger.cc

iris_srcs = ../src/manifold/models/iris/iris_srcs/data_types/networkPacket.cc \
            ../src/manifold/models/iris/iris_srcs/data_types/flit.cc \
            ../src/manifold/models/iris/iris_srcs/data_types/linkData.cc \
            ../src/manifold/models/iris/iris_srcs/topology/twoNode.cc \
            ../src/manifold/models/iris/iris_srcs/topology/ring.cc \
            ../src/manifold/models/iris/iris_srcs/topology/mesh.cc \
            ../src/manifold/models/iris/iris_srcs/topology/torus.cc \
            ../src/manifold/models/iris/iris_srcs/topology/util.cc \
            ../src/manifold/models/iris/iris_srcs/components/genericBuffer.cc \
            ../src/manifold/models/iris/iris_srcs/components/genericRC.cc \
            ../src/manifold/models/iris/iris_srcs/components/genericSwitchArbiter.cc \
            ../src/manifold/models/iris/iris_srcs/components/genericVcAllocator.cc \
            ../src/manifold/models/iris/iris_srcs/components/manifoldProcessor.cc \
            ../src/manifold/models/iris/iris_srcs/components/ninterface.cc \
            ../src/manifold/models/iris/iris_srcs/components/simpleRouter.cc

#            ../src/manifold/models/iris/iris_srcs/components/pktgen.cc 
#            ../src/manifold/models/iris/iris_srcs/components/simpleMC.cc 

ei_srcs = ../src/$(EI_DIR)/parameters.cc \
          ../src/$(EI_DIR)/parser.cc \
          ../src/$(EI_DIR)/string_ops.cc \
          ../src/$(EI_DIR)/energy_introspector.cc \
          ../src/$(EI_DIR)/ENERGYLIB_McPAT.cc \
          ../src/$(EI_DIR)/ENERGYLIB_IntSim.cc \
          ../src/$(EI_DIR)/THERMALLIB_HotSpot.cc \
		  ../src/$(EI_DIR)/RELIABILITYLIB_RAMP.cc \
		  ../src/$(EI_DIR)/SENSORLIB_RNG.cc

all_sources += $(iris_kernel_srcs) $(iris_srcs) $(ei_srcs)


# this lists the binaries to produce, the (non-PHONY, binary) targets in
# the previous manual Makefile
bin_PROGRAMS = macsim_opt
check_PROGRAMS = macsim_dbg macsim_gpf
#lib_LTLIBRARIES = libmacsimInternal.la

#libmacsimInternal_la_CXXFLAGS =


#macsim_opt_CPPFLAGS = -O3 -std=c++0x -funroll-loops $(DEBUG_FLAGS) $(WARN_FLAGS) $(XFLAGS) $(COMMON_FLAGS) -I .
macsim_opt_CPPFLAGS = -O3 -std=c++0x -funroll-loops -Werror $(DEBUG_FLAGS) $(WARN_FLAGS) $(XFLAGS) $(COMMON_FLAGS) -I .
macsim_opt_SOURCES = $(all_sources)
macsim_opt_LDFLAGS = -static

macsim_dbg_CPPFLAGS = -g3 -std=c++0x $(WARN_FLAGS) $(XFLAGS) $(COMMON_FLAGS) -I .
macsim_dbg_SOURCES = $(all_sources)
macsim_dbg_LDFLAGS = -g3 -static

macsim_gpf_CPPFLAGS = $(macsim_opt_CPPFLAGS) $(COMMON_FLAGS) -pg -I .
macsim_gpf_SOURCES = $(all_sources)
macsim_gpf_LDFLAGS = -pg -static

opt: macsim_opt

dbg: macsim_dbg

gpf: macsim_gpf

cleanopt:
	-test -z macsim_opt || rm -f macsim_opt
	-rm -f macsim_opt*.o

cleandbg:
	-test -z macsim_dbg || rm -f macsim_dbg
	-rm -f macsim_dbg*.o

cleangpf:
	-test -z macsim_gpf || rm -f macsim_gpf
	-rm -f macsim_gpf*.o

cleandep:
	-rm -f .deps/*


