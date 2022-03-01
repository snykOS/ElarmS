# Makefile for epic

ALL	= ewpio ElarmSWP2 E2

all: $(ALL)
	for dir in $(ALL) ; do \
		make -C $$dir all ; \
	done

ids: $(ALL)
	for dir in $(ALL) ; do \
		make -C $$dir ids ; \
	done

rm-ids: $(ALL)
	for dir in $(ALL) ; do \
		make -C $$dir rm-ids ; \
	done

install:
	for dir in $(ALL) ; do \
		make -C $$dir install ; \
	done

docs:
	doxygen doxygen.conf

cleandocs:
	rm -rf ../docs/epic

clean:
	for dir in $(ALL) ; do \
		make -C $$dir clean ; \
	done

veryclean: cleandocs
	for dir in $(ALL) ; do \
		make -C $$dir veryclean ; \
	done

depend:
	for dir in $(ALL) ; do \
		make -C $$dir depend ; \
	done

test: # no-op
