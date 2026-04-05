
;; (local res (list ))
;; (= ([] res 0) (list ))
;; (= ([] res 1) 0)
;; (print ([] '(0) 0))
;; res
t

(defun map ((li list fun list)
    (local res (copy li))
    (for it res
        (= it (fun it))
    )
    (print li)
    (print res)
    res
))

(defun filter ((li list fun list)
    ()
))


(local li '(1 2 3))
(= ([] li 0) 4)

(for it li
    (= it 0)
)
(== li '(0 0 0))


;; (print (map li '((x) (++ x))))
;; (== li (map li '((x) (+ x 1))))


;; (local res '(1 2 3))
;; (while (< i (len res))
;;     (= ([] res i) 
;;         (fun ([] res i))
;;     )
;; )
;; (print res)



