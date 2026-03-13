;; '(0 1 1 2 3 5 8 13 21 34)
t
(defun fib ((n a b i)

    (set a 0)
    (set b 1)
    (set i 0)
    (while (!= i n)
        (set tmp a)
        (set a (+ b a))
        (set b tmp)
        (set i (+ i 1))
    )
    a
))
(== (fib 9 0 0 0) 34)