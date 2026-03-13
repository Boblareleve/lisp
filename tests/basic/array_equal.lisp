t

(defun array_equal ((a b)
    (set fa (first a))
    (set fb (first b))
    (set na (next a))
    (set nb (next b))
    (if (!= fa fb) (return ()))

    (while (&& na nb) ;; while na and nb are not empty
        (if (!= fa fb) (return ()))

        (set fa (first a))
        (set fb (first b))
        (set na (next a))
        (set nb (next b))
    )
    t
))