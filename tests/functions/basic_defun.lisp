t
(defun foo ((n)
    (local r (+ n 1))
    ;;  (print n)
    r
))
(&&
    (== (foo 2) 3)
    (== (foo 1) 2)
    (== (foo 0) 1)
)
