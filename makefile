.PHONY: all clean re san debug release


CFLAGS=-I$(MY_LIB) -Wextra -Werror=incompatible-pointer-types -Wall -Wno-type-limits -Wno-missing-braces -Wno-address
LFLAGS=-lm -lffi

OBJ_DIR = obj
SRC_DIR = src

SRCS = $(wildcard $(SRC_DIR)/*.c)

OBJS 		 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))
OBJS_SAN     = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.san.o, $(SRCS))
OBJS_DEBUG 	 = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.debug.o, $(SRCS))
OBJS_RELEASE = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.release.o, $(SRCS))

all: lisp san debug release

clean:
	rm -rf $(OBJ_DIR)
	rm -rf $(EXE)

re: clean all



$(OBJ_DIR):
	mkdir $(OBJ_DIR)



san: CFLAGS += -g3 -fsanitize=address,undefined
san: $(OBJS_SAN)
	gcc -o lisp_l $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.san.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)



debug: CFLAGS += -ggdb
debug: $(OBJS_DEBUG)
	gcc -o lisp_d $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.debug.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)



release: CFLAGS += -O1
release: $(OBJS_RELEASE)
	gcc -o lisp_r $^ $(LFLAGS) $(CFLAGS)

$(OBJ_DIR)/%.release.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS)



lisp:
lisp: $(OBJS)
	gcc -o lisp $^ $(LFLAGS) $(CFLAGS) 

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	gcc -c -o $@ $< $(CFLAGS) 






