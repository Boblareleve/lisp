;; '(0 1 1 2 3 5 8 13 21 34)
t
(defun fib ((n)

    (local a 0)
    (local b 1)
    (local i 0)
    
    (while (!= i n)
        (local tmp a)
        (= a (+ b a))
        (= b tmp)
        (++ i) ; (+ i 1))
    )
    a
))

(&&
    (== (fib 9) 34)
    (== (fib 4) 3)
    (== (fib 20) 6765)
    (== (fib 30) 832040)
    (== (fib 40) 102334155)
)