#Add-on Makefile for wxDev-C++ project file
SRCDIR=src

all-before:
	${RM} $(SRCDIR)/comptime.h
	comptime.bat $(SRCDIR)

clean-custom:
	${RM} $(SRCDIR)/comptime.h
