CC := gcc
LD := gcc
TARGET := suckulent

SRC := ./src
INC := ./inc
OBJ := ./obj

OBJECTS := $(OBJ)/suckulent.o\
           $(OBJ)/arfs.o\
           $(OBJ)/util.o

DEPS := $(OBJECTS:.o=.d)

CFLAGS  := -I$(INC)
CFLAGS  += -Wall -pedantic
CFLAGS  += -D_X_OPEN_SOURCE=500 -D_GNU_SOURCE
CFLAGS  += -I/usr/local/opt/libarchive/include
LDFLAGS := -lreadline -L/usr/local/Cellar/libarchive/3.4.0/lib -larchive

.PHONY: all debug release
all: debug

debug: CFLAGS += -g -O0
debug: $(TARGET)

release: CFLAGS += -O2
release: $(TARGET)

ifneq ($(filter clean,$(MAKECMDGOALS)),clean)
-include $(DEPS)
endif

$(TARGET): $(OBJECTS)
	@echo [LD] -o $@ $^
	@$(LD) $(LDFLAGS) -o $@ $^

$(DEPS) $(OBJECTS): | $(OBJ)/

$(OBJ)/%.o: $(SRC)/%.c
	@echo [CC] -o $@ $<
	@$(CC) $(CFLAGS) -c -o $@ $<

$(OBJ)/%.d: $(SRC)/%.c
	@echo [DEP] $<
	@set -e; rm -f $@; \
	$(CC) -MM -MF $@ $(CFLAGS) $<;\
	sed -i 's,\($*\)\.o[\s:]*,\1.o $@: ,g' $@;

%:
	mkdir -p $@

.PHONY: clean
clean:
	$(RM) -r $(OBJ) $(TARGET)

