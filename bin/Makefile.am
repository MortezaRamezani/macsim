AUTOMAKE_OPTIONS = foreign
SUBDIRS = ../ 


all:

opt:
	@echo Build optimized executable
	make opt -C ../.obj
	-test -z ../.obj/macsim_opt || (test -z ./macsim || rm -f ./macsim) && ln -s ../.obj/macsim_opt macsim

dbg:
	@echo Build debugging executable
	make -C ../scripts
	make dbg -C ../.obj
	-test -z ../.obj/macsim_dbg || (test -z ./macsim || rm -f ./macsim) && ln -s ../.obj/macsim_dbg macsim

gpf:
	@echo Build gprof-capable executable
	make -C ../scripts
	make gpf -C ../.obj
	-test -z ../.obj/macsim_gpf || (test -z ./macsim || rm -f ./macsim) && ln -s ../.obj/macsim_gpf macsim

cleanopt:
	make clean -C ../scripts
	make cleanopt -C ../.obj
	-test -z macsim || rm -f macsim

cleandbg:
	make clean -C ../scripts
	make cleandbg -C ../.obj
	-test -z macsim || rm -f macsim

cleangpf:
	make clean -C ../scripts
	make cleangpf -C ../.obj
	-test -z macsim || rm -f macsim

cleandep:
	make cleandep -C ../.obj
