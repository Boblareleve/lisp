6
(local 'sum 0)
;; (print "(")
(for it (copy '('(1) '(2) '(3)))
    (= sum (+ sum (first it)))
    ;; (print it)
    ;; (print " ")
)
sum
;; (print ")")