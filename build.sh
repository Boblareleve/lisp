


OUT=

case $1 in
    release)
        OUT=lisp_r
        gcc -DNDEBUG main.c lisp.c -o $OUT -I$MY_LIB -lm  -Wall -Wno-missing-braces -Wno-address 
    ;;
    debug)
        OUT=lisp_d
        gcc -DDEBUG main.c lisp.c  -o $OUT -I$MY_LIB -lm -ggdb  -Wall -Wno-missing-braces -Wno-address 
    ;;
    sanitized)
        OUT=lisp_s
        gcc -DDEBUG main.c lisp.c  -o $OUT -I$MY_LIB -g3 -lm -fsanitize=address -fsanitize=undefined -Wall -Wno-missing-braces -Wno-address 
    ;;
    *)
        echo UNKOWN target usage: $0 '{release,debug,sanitized}'
        exit 1
    ;;
esac

case $2 in
    test_all)
        ./$OUT tests/*/*.lisp
    ;;
    test_basic)
        ./$OUT tests/basic/*.lisp
    ;;
    '') exit 0 ;;
    *)
        echo UNKOWN target usage: $0 '{release,debug,sanitized}' '?{test_all, test_basic}'
        exit 1
    ;;
esac