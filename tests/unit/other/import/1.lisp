TRUE

(import "core.lisp")


(defun bla (() t))
(let x 1)
(var y 1)

(&& 
    (bla)
    (== x y 1)
)
