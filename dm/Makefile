# Makefile for dm

ALL_PROGS = decimod

all: all_progs

all_progs: $(ALL_PROGS)
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir -f Makefile all ; \
	done

install: install_progs

install_progs: $(ALL_PROGS)
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir -f Makefile install ; \
	done

docs:
	doxygen doxygen.conf

cleandocs:
	rm -rf ../docs/dm *.tag

clean:
	for dir in $(ALL_PROGS); do \
		make -C $$dir -f Makefile clean ; \
	done

veryclean: cleandocs
	for dir in $(ALL_PROGS); do \
		make -C $$dir -f Makefile veryclean ; \
	done

test: # no-op

ids:
rm-ids:
depend:
	for dir in $(ALL_PROGS) ; do \
		make -C $$dir depend ; \
	done
