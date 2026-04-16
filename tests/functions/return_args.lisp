t
(global 'foo '((n)
    (if (== n 2) 
        (return n)
    )
    1
))
(&&
    (== (foo 2)  2)
    (== (foo 3)  1)
    (== (foo -1) 1)
)