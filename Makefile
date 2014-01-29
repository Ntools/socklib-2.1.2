all:
	make -f Makefile.linux
	make -f Makefile.win32

clean:
	make -f Makefile.linux clean

rpm:
	make -f Makefile.linux rpm
