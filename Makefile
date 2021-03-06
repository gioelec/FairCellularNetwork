all: checkmakefiles
	cd src && rm -f *.exe
	cd src && $(MAKE)

clean: checkmakefiles
	cd src && $(MAKE) clean
	cd src && rm -f *.exe

cleanall: checkmakefiles
	cd src && $(MAKE) MODE=release clean
	cd src && $(MAKE) MODE=debug clean
	cd src && rm -f Makefile

makefiles:
	cd src && opp_makemake -f --deep

checkmakefiles:
	@if [ ! -f src/Makefile ]; then \
	make makefiles;\
	echo "Execute make all again"; \
	fi
