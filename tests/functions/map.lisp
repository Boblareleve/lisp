
;; (local res (list ))
;; (= ([] res 0) (list ))
;; (= ([] res 1) 0)
;; (print ([] '(0) 0))
;; res
t

(defun map ((li fun) ; list fun list)
    (= li (copy li))
    (local i 0)
    (while (< i (len li))
        (= ([] li i) (fun ([] li i)))
        (++ i)
    )
    li
))

(defun filter ((li list fun list)
    ()
))


(local li '(1 2 3))
(= ([] li 0) 4)
(== li '(4 2 3))

;; (for it li
;;     (= it 0)
;; )
(== (map li '((x) 0)) '(0 0 0))


;; (print (map li '((x) (++ x))))
;; (== li (map li '((x) (+ x 1))))


;; (local res '(1 2 3))
;; (while (< i (len res))
;;     (= ([] res i) 
;;         (fun ([] res i))
;;     )
;; )
;; (print res)



