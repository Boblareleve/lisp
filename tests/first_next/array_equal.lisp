t

(defun array_equal ((a b)

    (while (&& a b) ;; while a and b are not empty
        (if (!= (first a) (first b)) (return ()))

        (set a (next a))
        (set b (next b))
    )
    (&& (! a) (! b)) ;; a and b empty
))

;; (&&
    (array_equal '(1 2)   '(1 2))
    ;; (!(array_equal '()      '(1)))
    ;;   (array_equal '()      '())
    ;; (!(array_equal '(1 2 3) '(1 2)))
;; )
