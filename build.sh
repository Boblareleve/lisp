gcc -DDEBUG main.c lisp.c -o lisp -I$MY_LIB -lm -ggdb  -Wall -Wno-missing-braces -Wno-address 
# ./main tests/test*.lisp

