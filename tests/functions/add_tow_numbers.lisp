

(defun get_sum ((a b)
    (+ (first a) (first b))
))

(defun add ((a b)

    (set res 0)
    
    (while (&& a b)
        (set res (* res 10))
        (set sum (get_sum a b))
        (set res (+ res sum))
        (set a (next a))
        (set b (next b))
    )
))

(&&
    (== (add '(1 2 3 4) '(4 3 2 1)) 5555)
    (== (add '(9 1) '(2 0)) 111)
)