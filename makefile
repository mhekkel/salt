# only for creating a version file

#WCREV				= $(shell svn info | grep '^Revision' | sed -e s'/Revision: //')
WCREV				= $(shell svnversion)
WCNOW				= $(shell svn info | sed -n -e 's/.*(.*, \(.*\))/\1/p')
WCNOW_DAY			= $(shell echo $(WCNOW) | awk '{print $$1}')
WCNOW_MONTH			= $(shell echo $(WCNOW) | awk '{print $$2}')
WCNOW_YEAR			= $(shell echo $(WCNOW) | awk '{print $$3}')

Sources/MSaltVersion.h: Sources/MSaltVersion.h.tmpl makefile
	sed -e 's/\$$WCREV\$$/$(WCREV)/' \
	    -e 's/\$$WCNOW=%Y\$$/$(WCNOW_YEAR)/' \
	    -e 's/\$$WCNOW=%m\$$/$(WCNOW_MONTH)/' \
	    -e 's/\$$WCNOW=%d\$$/$(WCNOW_DAY)/' \
		< Sources/MSaltVersion.h.tmpl > $@
