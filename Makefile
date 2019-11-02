CC := gcc
LD := gcc
TARGET := suckulent

SRC := ./src
INC := ./inc
OBJ := ./obj

OBJECTS := $(OBJ)/suckulent.o\
           $(OBJ)/util.o

DEPS := $(OBJECTS:.o=.d)

CFLAGS  := -I$(INC) -Wall
LDFLAGS := -lreadline

.PHONY: all debug release
all: debug

debug: CFLAGS += -g
debug: $(TARGET)

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

