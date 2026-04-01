t
(defun foo ((n)
    ;;  (print n)
    (+ n 1)
))
(&&
    (== (foo 2) 3)
    (== (foo 1) 2)
    (== (foo 0) 1)
)
