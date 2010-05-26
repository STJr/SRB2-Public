#Add-on Makefile for wxDev-C++ project file
OBJCOPY="objcopy.exe"
DGB=$(BIN).debug

all-after:
	$(OBJCOPY) --only-keep-debug $(BIN) $(DGB)
	$(OBJCOPY) --strip-debug $(BIN)
	$(OBJCOPY) --add-gnu-debuglink=$(DGB) $(BIN)
