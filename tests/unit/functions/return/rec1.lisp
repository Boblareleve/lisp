TRUE

(global 'i 10)
(global 'foo '(()
    (-- i)
    (? (== 0 i) (return t) (return (foo)))
    "how"
))

(foo)
