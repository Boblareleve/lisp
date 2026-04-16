t
(global 'foo '(()
    (return 2)
    1
))

(== (foo) 2)
