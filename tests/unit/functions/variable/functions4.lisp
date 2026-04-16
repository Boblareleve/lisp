EQUAL ; recursivity without local variable
0

(global 'c 10)

(global 'foo '(()
    (-- c)
    (? (> c 0) (foo) ())
    c
))

(foo)