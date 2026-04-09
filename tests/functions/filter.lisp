t

(defun filter ((li fun) ; list fun list)
    
    (local res (array (len li)))
    (local remp 0)
    
    (local i 0)
    (while (< i (len li))
        (if (fun ([] li i))
            (= (&[] res remp) ([] li i))
            (++ remp)
        )
        (++ i)
    )
    ([] res 0 (+ remp 1))
))

(local li '(1 2 3))

;; (print (filter li '((x) (== x 3))))
(== (filter li '((x) (== x 3))) '(3))


