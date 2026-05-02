EQUAL

(list '(
    string
    int
    float
    number
    number
    type
    list
    symbole 
    any
    int
    int
))

(local 'x any 0)
(local 'y 1)
(local 'yy int 1)
(list '(
    (typeof "bla")
    (typeof 3245)
    (typeof 49.2)
    (typeof 1)
    (typeof 1.0)
    (typeof any)
    (typeof '())
    (typeof 'a)
    (typeof x)
    (typeof y)
    (typeof yy)
))
