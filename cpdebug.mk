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
OBJDUMP_OPTS?=--wide --source --line-numbers
GZIP?=gzip
GZIP_OPTS?=-9 -f
GZIP_OPT2=$(GZIP_OPTS) --rsyncable

all-after:
	$(OBJDUMP) $(OBJDUMP_OPTS) $(BIN) > $(DGB).txt
	-$(GZIP) $(GZIP_OPT2) $(BIN)/$(DBGNAME).txt || -$(GZIP) $(GZIP_OPTS) $(BIN)/$(DBGNAME).txt
	$(OBJCOPY) $(BIN) $(DGB)
	$(OBJCOPY) --strip-debug $(BIN)
	$(OBJCOPY) --add-gnu-debuglink=$(DGB) $(BIN)
