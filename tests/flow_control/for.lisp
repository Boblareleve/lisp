6
(local 'sum 0)
;; (print "(")
(for it '(1 2 3)
    (= sum (+ sum it))
    ;; (print it)
    ;; (print " ")
)

sum
;; (print ")")