TRUE

(global 'fib '((n int int)

    (? (<= n 1) (return n) ())
    (+ (fib (- n 1)) (fib (- n 2)))
))


(&&
    (== (fib 9) 34)
    (== (fib 4) 3)
    (== (fib 20) 6765)
    
    ;; (== (fib 25) 75025)
    ;; (== (fib 30) 832040)
    ;; (== (fib 40) 102334155)
)
