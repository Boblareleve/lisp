;; '(0 1 1 2 3 5 8 13 21 34)
t
(defun fib ((n)
    ;; (print n "\n")
    
    (if (<= n 1) (return n))
    ;; (if (== n 0) (return 0))
    ;; (if (== n 1) (return 1))
    
    (+ (fib (- n 1)) (fib (- n 2)))
))


;; (print (fib 4)  "\n")
;; (print (fib 9)  "\n")
;; (print (fib 10) "\n")
;; (print (fib 20) "\n")
;; (print (fib 25) "\n")
;; (print (fib 30) "\n")
;; (print (fib 40) "\n")

(&&
    (== (fib 9) 34)
    (== (fib 4) 3)
    (== (fib 20) 6765)
    ;; (== (fib 25) 75025)
    ;; (== (fib 30) 832040)
    ;; (== (fib 40) 102334155)
)