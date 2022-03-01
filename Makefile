# Makefile for third_party

ALL_LIBS = qlib2 librtseis libtnchnl libtndb libtnstd libtntime libtnwave OTL oracle libamq
ALL_PROGS = conlog mcast2ew qmcast2ew spyring

all: all_libs all_progs

all_libs: $(ALL_LIBS)
	for dir in $(ALL_LIBS) ; do \
		make -C $$dir all ; \
	done

all_progs: $(ALL_PROGS)
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir all ; \
	done

install: install_libs install_progs

install_libs: $(ALL_LIBS)
	for dir in $(ALL_LIBS) ; do \
		make -C $$dir install ; \
	done

install_progs: $(ALL_PROGS)
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir install ; \
	done

clean:
	for dir in $(ALL_LIBS) ; do \
		make -C $$dir clean ; \
	done
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir clean ; \
	done

veryclean:      clean

depend:	# no-op

docs: # no-op
ids: # no-op
rm-ids: # no-op
test: # no-op
