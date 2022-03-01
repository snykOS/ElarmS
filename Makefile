# Makefile for libs

SUBDIRS=dmlib utils datapkt

.PHONY: all show-targets $(SUBDIRS)
show-targets:
	@echo TARGETS:  $(TARGET_LIST)
TARGET_LIST=show-targets

all: $(SUBDIRS)

ids: $(SUBDIRS)
rm-ids: $(SUBDIRS)
test: $(SUBDIRS)
docs: $(SUBDIRS)

# define macro to generate rules for target, list of sub targets and rule for each.
define gen_recursive_targets
.PHONY: $(1) $(2:%=$(1)-%)
$(1): $(2:%=$(1)-%)
$(2:%=$(1)-%):
	$(MAKE) -C $$(@:$$$(1)-%=%) $1
	@echo -e ""
TARGET_LIST+= $(1) $(2:%=$(1)-%)
endef

#TARGETS=all clean depend veryclean test check
# invoke macro for each target -- not currently working
#$(forearch target, $(TARGETS), $(eval $(call gen_recursive_targets, $(target), $(SUBDIRS))))

# use macro to define recursive targets
$(eval $(call gen_recursive_targets, ids, 		$(SUBDIRS)))
$(eval $(call gen_recursive_targets, rm-ids, 	$(SUBDIRS)))
$(eval $(call gen_recursive_targets, all, 		$(SUBDIRS)))
$(eval $(call gen_recursive_targets, clean, 	$(SUBDIRS)))
$(eval $(call gen_recursive_targets, depend, 	$(SUBDIRS)))
$(eval $(call gen_recursive_targets, veryclean, $(SUBDIRS)))
$(eval $(call gen_recursive_targets, ut, 		$(SUBDIRS)))
$(eval $(call gen_recursive_targets, test, 		$(SUBDIRS)))
$(eval $(call gen_recursive_targets, docs, 		$(SUBDIRS)))

