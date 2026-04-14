EQUAL

(eval
    string
    int
    float
    type
    list
    symbole 
    any
    int
    int
)
(local 'x any 0)
(local 'y 1)
(local 'yy int 1)
(eval
    (typeof "bla")
    (typeof 3245)
    (typeof 49.2)
    (typeof any)
    (typeof '())
    (typeof 'a)
    (typeof x)
    (typeof y)
    (typeof yy)
)
