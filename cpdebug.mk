#Add-on Makefile for wxDev-C++ project file
ifdef ComSpec
COMSPEC=$(ComSpec)
endif
ifdef COMSPEC
OBJCOPY="objcopy.exe"
else
OBJCOPY="objcopy"
endif
DGB=$(BIN).debug

all-after:
	$(OBJCOPY) $(BIN) $(DGB)
	$(OBJCOPY) --strip-debug $(BIN)
	$(OBJCOPY) --add-gnu-debuglink=$(DGB) $(BIN)
