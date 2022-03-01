# Toplevel Makefile for EEW development and CM for RT programs and libraries

# third party libs and apps
THIRD_PARTY_LIBS = third_party
THIRD_PARTY_APPS = \
	$(THIRD_PARTY_LIBS)/conlog \
	$(THIRD_PARTY_LIBS)/mcast2ew \
	$(THIRD_PARTY_LIBS)/qmcast2ew \
	$(THIRD_PARTY_LIBS)/spyring
THIRD_PARTY = $(THIRD_PARTY_LIBS) $(THIRD_PARTY_APPS)

# ShakeAlert Library directories.
MAKE_LIB_DIRS = \
	libs

# ShakeAlert Programs directories
MAKE_PROG_DIRS = \
	dm \
	epic

# Other programs
OTHER_DIRS =

DOCSDIR = docs

.PHONY: all show-targets $(SUBDIRS)

# default target is first rule
TARGET_LIST=show-targets
show-targets:
	@echo TARGET_LIST=$(TARGET_LIST)

# Define full list of directories for generating convenient rules for developers
SUBDIRS = $(MAKE_LIB_DIRS) $(MAKE_PROG_DIRS) $(OTHER_DIRS)

# Define the 'all' target 
all: $(SUBDIRS)


docsdir:
	if [ ! -d "$(DOCSDIR)" ]; then mkdir $(DOCSDIR); fi


# define macro to generate rules for target, list of sub targets and rule for each.
define gen_recursive_targets
.PHONY: $(1) $(2:%=$(1)-%)
$(1): $(2:%=$(1)-%)
$(2:%=$(1)-%):
	$(MAKE) -C $$(@:$$$(1)-%=%) $1
	@echo -e ""
TARGET_LIST+= $(1) $(2:%=$(1)-%)
endef

# use macro to define recursive targets
$(eval $(call gen_recursive_targets, all, $(SUBDIRS) $(THIRD_PARTY) ))
$(eval $(call gen_recursive_targets, ids, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, rm-ids, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, install, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, clean, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, depend, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, veryclean, $(SUBDIRS) $(THIRD_PARTY) ))
$(eval $(call gen_recursive_targets, test, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, docs, $(SUBDIRS)))

# Define additional rules for building each algorithm with dependencies
third_party: all-third_party
epic: all-third_party  all-libs all-epic
dmlib: all-third_party all-libs
dm: all-third_party all-libs all-dm
documentation:

# Add the additional targets to the automatically generated target list
TARGET_LIST+=dmlib dm epic
TARGET_LIST+=printvars

force:	

printvars:
	@echo THIRD_PARTY_LIBS=$(THIRD_PARTY_LIBS)
	@echo THIRD_PARTY_APPS=$(THIRD_PARTY_APPS)
	@echo THIRD_PARTY=$(THIRD_PARTY)
	@echo MAKE_LIB_DIRS=$(MAKE_LIB_DIRS)
	@echo MAKE_PROG_DIRS=$(MAKE_PROG_DIRS)
	@echo OTHER_DIRS=$(OTHER_DIRS)
	@echo SUBDIRS=$(SUBDIRS)
	@echo DOCSDIR=$(DOCSDIR)
	@echo
	@echo TARGET_LIST=$(TARGET_LIST)
