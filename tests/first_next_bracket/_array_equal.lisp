t

(global 'array_equal '((a b)

    (while (&& a b) ;; while a and b are not empty
        ;; (if (!= (first a) (first b)) (return ()))
        (= a (next a))
    )
    1 ;(&& (! a) (! b)) ;; a and b empty
))


;; (!(array_equal '(1 2 3) '(1 2)))
t
