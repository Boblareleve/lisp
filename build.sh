

WFLAGS='-Werror=incompatible-pointer-types -Wall -Wno-missing-braces -Wno-address'

OUT=

case $1 in
    release)
        OUT=lisp_r
        gcc -DNDEBUG main.c lisp.c -o $OUT -I$MY_LIB -lm $WFLAGS
    ;;
    debug)
        OUT=lisp_d
        gcc -DDEBUG main.c lisp.c  -o $OUT -I$MY_LIB -lm -ggdb $WFLAGS
    ;;
    sanitized)
        OUT=lisp_s
        gcc -DDEBUG main.c lisp.c  -o $OUT -I$MY_LIB -g3 -lm -fsanitize=address -fsanitize=undefined $WFLAGS
    ;;
    *)
        echo UNKOWN target \'$1\' usage: $0 '{release,debug,sanitized}'
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
    '')
        exit 0
    ;;
    *)
        echo UNKOWN test \'$2\' usage: $0 '{release,debug,sanitized}' '?{test_all, test_basic}'
        exit 1
    ;;
esac