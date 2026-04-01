;; '(0 1 1 2 3 5 8 13 21 34)
t
(defun fib ((n int int)
    ;; (print n "\n")
    
    (if (<= n 1) (return n))
    ;; (if (== n 0) (return 0))
    ;; (if (== n 1) (return 1))
    
    (+ (fib (- n 1)) (fib (- n 2)))
))


;; (local i 0)
;; (while (<= i 40)
;;     (print i  " " (fib i)  "\n")
;;     (++ i)
;; )

(&&
    (== (fib 9) 34)
    (== (fib 4) 3)
    (== (fib 20) 6765)
    ;; (== (fib 25) 75025)
    ;; (== (fib 30) 832040)
    ;; (== (fib 40) 102334155)
)
