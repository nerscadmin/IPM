
ipmetcdir = $(prefix)/etc

WRAPPER_TEMPLATES = fake_*.c wrap_*.c

WRAPPER_HELPER =  ipm_key_*
             
EXTRA_DIST = $(WRAPPER_HELPER) $(WRAPPER_TEMPLATES)
             
install-exec-hook:
	mkdir -p $(prefix)/etc
	for i in $(WRAPPER_HELPER); do \
		cp $$i $(prefix)/etc; \
	done

uninstall-local:
	for i in $(WRAPPER_HELPER); do \
		rm -f $(prefix)/etc/$$i; \
	done



             
