t
(defun foo ((n)
    (return n)
    -1
))
(== (foo 2) 2)