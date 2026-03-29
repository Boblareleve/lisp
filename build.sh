

WFLAGS='-Wextra -Werror=incompatible-pointer-types -Wall -Wno-missing-braces -Wno-address'

OUT=lisp_d
SRC='main.c lisp.c dump.c parse.c memory.c'


case $1 in
    release)
        OUT=lisp_r
        gcc  $SRC -o $OUT -I$MY_LIB -lm $WFLAGS -DNDEBUG -O1 
    ;;
    debug)
        OUT=lisp_d
        gcc  $SRC  -o $OUT -I$MY_LIB -lm $WFLAGS -DDEBUG -ggdb  
    ;;
    sanitized)
        OUT=lisp_s
        gcc  $SRC  -o $OUT -I$MY_LIB -g3 $WFLAGS -DDEBUG -lm -fsanitize=address,undefined 
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
        ./$OUT tests/*/*.lisp         # 1> log/log.txt
    ;;
    test_basic)
        ./$OUT tests/basic/*.lisp     # 1> log/log.txt
    ;;
    test_functions)
        ./$OUT tests/functions/*.lisp # 1> log/log.txt
    ;;
    test_unit)
        ./$OUT tests/$3               # 1> log/log.txt
    ;;
    '')
        exit 0
    ;;
    *)
        echo UNKOWN test \'$2\' usage: $0 '{release,debug,sanitized}' '?{test_all, test_basic test_functions}'
        exit 1
    ;;
esac