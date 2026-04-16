MAKEFLAGS += -j16

.PHONY: all clean re san debug release tests



CFLAGS = -I$(MY_LIB)						\
		 -MMD -MP 							\
		 -Wextra -Wall 						\
		 -Werror=incompatible-pointer-types	\
		 -Wno-missing-braces 				\
		 -Wno-type-limits 					\
		 -Wno-address
LFLAGS = -lm -lffi

OBJ_DIR = obj
SRC_DIR = src
DEP_DIR = obj

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

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(EXE)

re: clean all



$(OBJ_DIR):
	mkdir $(OBJ_DIR)


san: lisp_s

lisp_s: CFLAGS += -g3 -fsanitize=address,undefined
lisp_s: $(OBJS_SAN)
	gcc -o lisp_s $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.san.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)


debug: lisp_d

lisp_d: CFLAGS += -ggdb
lisp_d: $(OBJS_DEBUG)
	gcc -o lisp_d $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)


release: lisp_r

lisp_r: CFLAGS += -O3
lisp_r: $(OBJS_RELEASE)
	gcc -o lisp_r $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.release.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)



lisp:
lisp: $(OBJS)
	gcc -o lisp $^ $(LFLAGS) $(CFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS) 



# DIR ?= *
EXE ?= lisp_s
FILES ?= ../$(shell find ./tests/unit -type f)
tests: $(EXE)
	@./$(EXE) ./tests/$(FILES)
