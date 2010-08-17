#Add-on Makefile for wxDev-C++ project file
ifdef ComSpec
COMSPEC=$(ComSpec)
endif
ifdef COMSPEC
OBJCOPY=objcopy.exe
OBJDUMP=objdump.exe
else
OBJCOPY=objcopy
OBJDUMP=objdump
endif
DGB=$(BIN).debug

all-after:
	$(OBJDUMP) -S -l $(BIN) > $(DGB).txt
	$(OBJCOPY) $(BIN) $(DGB)
	$(OBJCOPY) --strip-debug $(BIN)
	$(OBJCOPY) --add-gnu-debuglink=$(DGB) $(BIN)
