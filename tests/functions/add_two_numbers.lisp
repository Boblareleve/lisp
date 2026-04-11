t
(global 'get_sum '((a b)
    (+ (first a) (first b))
))

(global 'add '((a list b list)

    (local 'res 0)
    
    (while (&& a b)
        (= res (* res 10))
        (local 'sum (get_sum a b))
        ;; (local 'sum (+ (first a) (first b)))
        (= res (+ res sum))
        (= a (next a))
        (= b (next b))
    )
    res
))

(&&
    (== (add '(1 2 3 4) '(4 3 2 1)) 5555)
    (== (add '(9 1) '(2 0)) 111)
)
