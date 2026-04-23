MAKEFLAGS += -j16

.PHONY: all clean re san debug release tests profile



CFLAGS = -I$(MY_LIB)						\
		 -MMD -MP 							\
		 -Wextra -Wall 						\
		 -Werror=incompatible-pointer-types	\
		 -Wno-missing-braces 				\
		 -Wno-type-limits 					\
		 -Wno-address
LFLAGS = -lm -lffi

OBJ_DIR  = tmp
SRC_DIR  = src
DEP_DIR  = tmp
# GCDA_DIR = tmp

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS 		 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_SAN     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.san.o, $(SRCS))
OBJS_DEBUG 	 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.debug.o, $(SRCS))
OBJS_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.release.o, $(SRCS))

DEPS		 = $(patsubst $(SRC_DIR)/%.c, $(DEP_DIR)/%.d, $(SRCS))
DEPS_SAN     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.san.d, $(SRCS))
DEPS_DEBUG 	 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.debug.d, $(SRCS))
DEPS_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.release.d, $(SRCS))


-include $(DEPS) $(DEPS_SAN) $(DEPS_DEBUG) $(DEPS_RELEASE)

all: lisp san debug release

EXES = lisp_d lisp_s lisp_r 

clean:
	rm -rf $(OBJ_DIR)
	rm -f $(EXES)

re: clean all



$(OBJ_DIR):
	mkdir $(OBJ_DIR)


san: lisp_s

lisp_s: CFLAGS += -g3 -fsanitize=address,undefined -DDEBUG
lisp_s: $(OBJS_SAN)
	gcc -o lisp_s $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.san.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)


debug: lisp_d

lisp_d: CFLAGS += -ggdb -DDEBUG
lisp_d: $(OBJS_DEBUG)
	gcc -o lisp_d $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)


release: lisp_r

lisp_r: CFLAGS += -O2 -DNDEBUG
lisp_r: $(OBJS_RELEASE)
	gcc -o lisp_r $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.release.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)



# profile: lisp_p

# lisp_p: CFLAGS += -O2 -DNDEBUG
# lisp_p: $(GCDA_DIR)/profile.gcda
# 	gcc -fprofile-use -fprofile-correction -o lisp_p $^ $(LFLAGS) $(CFLAGS)

# $(GCDA_DIR)/profile.gcda: $(SRCS) lisp_i
# 	./lisp_i -s 100 $(shell find ./tests/unit -type f)

# lisp_i: $(SRCS)
# 	gcc -fprofile-generate -o lisp_i $(SRCS) $(CFLAGS) $(LFLAGS)


lisp:
lisp: $(OBJS)
	gcc -o lisp $^ $(LFLAGS) $(CFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS) 



# DIR ?= *
EXE ?= lisp_s
FILES ?= ../$(shell find ./tests/unit -type f)
SAMPLES ?= 1
tests: $(EXE)
	@echo samples: $(SAMPLES)
	@./$(EXE) -s $(SAMPLES) ./tests/$(FILES)

tests_full: $(EXES)
	@echo test all build samples: $(SAMPLES)
	for exe in $(EXES); do \
		echo $$exe: && ./$$exe -s $(SAMPLES) ./tests/$(FILES) || exit 1; \
    done
	
