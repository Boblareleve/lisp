

WFLAGS='-Wextra -Werror=incompatible-pointer-types -Wall -Wno-missing-braces -Wno-address'

OUT=lisp_d
SRC='main.c lisp.c dump.c parse.c'


case $1 in
    release)
        OUT=lisp_r
        gcc -DNDEBUG $SRC -o $OUT -I$MY_LIB -lm $WFLAGS
    ;;
    debug)
        OUT=lisp_d
        gcc -DDEBUG $SRC  -o $OUT -I$MY_LIB -lm -ggdb $WFLAGS
    ;;
    sanitized)
        OUT=lisp_s
        gcc -DDEBUG $SRC  -o $OUT -I$MY_LIB -g3 -lm -fsanitize=address -fsanitize=undefined $WFLAGS
    ;;
    _)
    ;;
    *)
        echo UNKOWN target \'$1\' usage: $0 '{release,debug,sanitized}'
        exit 1
    ;;
esac

case $2 in
    test_all)
        ./$OUT tests/*/*.lisp 1> log/log.txt
    ;;
    test_basic)
        ./$OUT tests/basic/*.lisp 1> log/log.txt
    ;;
    test_functions)
        ./$OUT tests/functions/*.lisp 1> log/log.txt
    ;;
    test_unit)
        ./$OUT tests/$3 1> log/log.txt
    ;;
    '')
        exit 0
    ;;
    *)
        echo UNKOWN test \'$2\' usage: $0 '{release,debug,sanitized}' '?{test_all, test_basic test_functions}'
        exit 1
    ;;
esac