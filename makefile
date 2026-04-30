MAKEFLAGS += -j16

.PHONY: all clean re san debug strict release tests profile


CC = gcc

CFLAGS = -I$(MY_LIB)							\
		 -MMD -MP 								\
		 -Wextra -Wall 							\
		 -Werror=incompatible-pointer-types		\
		 -Wno-missing-braces 				    \
		 -Wno-type-limits 					    \
		 -Wno-unused-function   			    \
		 -Wno-address							\
		 $(FLAGS)

LFLAGS = -lm -lffi

BIN_DIR   = bin
OBJ_DIR   = tmp
SRC_DIR   = src
DEP_DIR   = tmp
DEBUG_DIR = sarif
# for profiling optimisation
# GCDA_DIR  = tmp 

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS 		 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_SAN     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.san.o, $(SRCS))
OBJS_DEBUG 	 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.debug.o, $(SRCS))
OBJS_STRICT  = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.strict.o, $(SRCS))
OBJS_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.release.o, $(SRCS))

DEPS		 = $(patsubst $(SRC_DIR)/%.c, $(DEP_DIR)/%.d, $(SRCS))
DEPS_SAN     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.san.d, $(SRCS))
DEPS_DEBUG 	 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.debug.d, $(SRCS))
DEPS_STRICT  = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.strict.d, $(SRCS))
DEPS_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.release.d, $(SRCS))


-include $(DEPS) $(DEPS_SAN) $(DEPS_DEBUG) $(DEPS_RELEASE) $(DEPS_STRICT)

all: lisp san debug release strict

EXES = lisp_d lisp_s lisp_r lisp_strict

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(EXES)
	rm -rf $(DEBUG_DIR)

re: clean
	$(MAKE) all

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(DEBUG_DIR):
	mkdir $(DEBUG_DIR)

$(BIN_DIR):
	mkdir $(BIN_DIR)

re_san: clean
	$(MAKE) san

san: lisp_s

lisp_s: CFLAGS += -g3 -fsanitize=address,undefined -DDEBUG -DFSAN
lisp_s: $(OBJS_SAN)
	$(CC) -o $(BIN_DIR)/lisp_s $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.san.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)



re_debug: clean
	$(MAKE) debug

debug: lisp_d

# -DRAISE_ON_FAILED_TRY
lisp_d: CFLAGS += -ggdb -DDEBUG 
lisp_d: $(OBJS_DEBUG)
	$(CC) -o $(BIN_DIR)/lisp_d $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)


re_release: clean
	$(MAKE) release

release: lisp_r

lisp_r: CFLAGS += -O1 -DNDEBUG
lisp_r: $(OBJS_RELEASE)
	$(CC) -o $(BIN_DIR)/lisp_r $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.release.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)


re_strict: clean
	$(MAKE) strict

strict: lisp_strict


lisp_strict: CFLAGS += -Wstrict-aliasing=3 -fanalyzer -DDEBUG
# lisp_strict: CFLAGS += -fdiagnostics-format=sarif-file
lisp_strict: $(OBJS_STRICT) | $(DEBUG_DIR)
	$(CC) -o $(BIN_DIR)/lisp_strict $^ $(LFLAGS) $(CFLAGS)
# 	2>> $(DEBUG_DIR)/strict_aliasing.sarif

$(OBJ_DIR)/%.strict.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEBUG_DIR)
	$(CC) -c -o $@ $< $(CFLAGS) 
# 	mv $*.strict.c.sarif $(DEBUG_DIR)/$*.c.sarif
# 		2>> $(DEBUG_DIR)/strict_aliasing.sarif


# profile: lisp_p

# lisp_p: CFLAGS += -O2 -DNDEBUG
# lisp_p: $(GCDA_DIR)/profile.gcda
# 	$(CC) -fprofile-use -fprofile-correction -o $(BIN_DIR)/lisp_p $^ $(LFLAGS) $(CFLAGS)

# $(GCDA_DIR)/profile.gcda: $(SRCS) lisp_i
# 	./lisp_i -s 100 $(shell find ./tests/unit -type f)

# lisp_i: $(SRCS)
# 	$(CC) -fprofile-generate -o $(BIN_DIR)/lisp_i $(SRCS) $(CFLAGS) $(LFLAGS)


lisp:
lisp: $(OBJS)
	$(CC) -o $(BIN_DIR)/lisp $^ $(LFLAGS) $(CFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS) 



# DIR ?= *
EXE ?= lisp_s
FILES ?= $(shell find ./tests -type f -name "*.lisp")
TESTS = $(shell find $(FILES) -type f -name "*.lisp" | sort)
SAMPLES ?= 1
tests: $(EXE)
	@echo samples: $(SAMPLES)
	@$(BIN_DIR)/$(EXE) -s $(SAMPLES) $(TESTS)

tests_full: $(EXES)
	@echo test all build samples: $(SAMPLES)
	for exe in $(EXES); do \
		echo $$exe: && $(BIN_DIR)/$$exe -s $(SAMPLES) $(TESTS) || exit 1; \
    done
	
