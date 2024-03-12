RM        := /bin/rm -rf
SIM       := ./sim
CC        := g++
CFLAGS    := -lz -O2 -lm  -W -Wall -Wno-deprecated  -Wno-unused-parameter -Wno-unused-variable -Wno-unused-but-set-variable
DFLAGS    := -pg -g
PFLAGS    := -pg




all:  folders traces
	${CC} ${CFLAGS} dram.c  mgries.c cra_ctr.c ctrcache.c scoreboard.c mcore.c os.c  mcache.c sim.c memsys.c  hash_lib.c malloc_lib.c gct.c rrs.c -o ${SIM} -lz

folders: 
	mkdir -p ../RESULTS

traces: 
	if test -d ../TRACES; then echo "Traces exist" ; else echo "Downloading traces..." ; wget -O traces.tar.gz "https://www.dropbox.com/s/9khghoyl27jxj26/traces.tar.gz?dl=1" ; mv traces.tar.gz ../. ;  cd ../ ; tar -zxvf traces.tar.gz ; rm -rf traces.tar.gz; fi

clean: 
	$(RM) ${SIM} *.o
