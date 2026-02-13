SUBDIRS = manual reflex_tutorial

all:
	@target=`echo $@`; \
	dot_seen="no"; \
	for d in $(SUBDIRS); do \
		echo "Making $$target in $$d"; \
		if test $$d = "."; then \
			dot_seen="yes"; \
		else \
			(cd $$d && $(MAKE) "$$target" || exit 1); \
		fi; \
	done; \
	if test "$$dot_seen" = "no"; then \
		:; \
	fi

clean:
	@target=`echo $@`; \
	dot_seen="no"; \
	for d in $(SUBDIRS); do \
		echo "Making $$target in $$d"; \
		if test $$d = "."; then \
			dot_seen="yes"; \
		else \
			(cd $$d && $(MAKE) "$$target" || exit 1); \
		fi; \
	done; \
	if test "$$dot_seen" = "no"; then \
		:; \
	fi

