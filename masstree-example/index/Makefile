# Requires anujkaliaiitd/masstree-beta (fork of mikeandmore/masstree-beta) at
# MASSTREE_HOME

MASSTREE_HOME=${HOME}/masstree-beta
DEBUG=
#DEBUG=-DNDEBUG

all:
	g++ -std=c++14 -g -O3 ${DEBUG} -o example example.cc mt_index_api.cc \
		-I ${MASSTREE_HOME} -include ${MASSTREE_HOME}/config.h -L ${MASSTREE_HOME} \
		-lmasstree -lpthread
clean:
	rm example
