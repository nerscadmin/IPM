
SUBDIRS = src bin etc utils

tests : all
	cd test && $(MAKE) || exit 1	

.PHONY: tests

tests-clean:
	cd test && $(MAKE) clean

#clean-local:
#	cd test && $(MAKE) clean

DIST_SUBDIRS = src bin etc test utils
# EXTRA_DIST = utils/*.cc utils/configure* utils/*.h utils/Makefile.*
EXTRA_DIST = utils/cubew/lib/*.c utils/cubew/lib/*.h utils/cubew/lib/Makefile utils/mf/common.defs utils/Makefile.defs



