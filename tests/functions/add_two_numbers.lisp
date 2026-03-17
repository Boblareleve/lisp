
;; (defun get_sum ((a b)
;;     (+ (first a) (first b))
;; ))

(defun add ((a b)

    (local res 0)
    
    (while (&& a b)
        (local res (* res 10))
        ;; (set sum (get_sum a b))
        (local sum (+ (first a) (first b)))
        (local res (+ res sum))
        (local a (next a))
        (local b (next b))
    )
))

(&&
    (== (add '(1 2 3 4) '(4 3 2 1)) 5555)
    (== (add '(9 1) '(2 0)) 111)
)
