TRUE

(global 'i 2)
(global 'foo '(()
    (-- i)
    (? (<= i 0) 
        (return t)
        ()
    )
    (foo) (foo)
))

(foo)
