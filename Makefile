TMP = tmps
OBJ = $(TMP)/obj
DEP = $(TMP)/dep
SRC = src
BLD = BUILD
CC = gcc
EXE = chachaRS
GLV = /usr/bin

inc_paths = 

files = chacha.c interface.c mac.c
all_OBJS = $(addprefix $(OBJ)/, $(files:.c=.o))
all_DEPS = $(addprefix $(DEP)/, $(files:.c=.d))

compilerFlags = -c -O3 -m64
linkerFlags = -lsodium

all: $(all_DEPS) $(all_OBJS)
	$(CC) $(all_OBJS) $(linkerFlags) -o $(BLD)/$(EXE)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $^ -c \
	$(compilerFlags) \
	-o $@

$(DEP)/%.d: $(SRC)/%.c
	$(CC) $^ -M \
	$(inc_paths)\
	> $@

.PHONY:install
install:
	cp $(BLD)/$(EXE) $(GLV)

.PHONY:uninstall
uninstall:
	rm -f $(GLV)/$(EXE)

.PHONY:clean
clean: 
	rm -f $(OBJ)/*
	rm -f $(DEP)/*
	rm -f $(BLD)/$(EXE)


include $(DEPS)
