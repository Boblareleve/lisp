6
(local 'sum 0)
;; (print "(")
(for it (copy '('(1) '(2) '(3)))
    (= sum (+ sum ([] it 0)))
    ;; (print it)
    ;; (print " ")
)
sum
;; (print ")")