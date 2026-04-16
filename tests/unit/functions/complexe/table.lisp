TRUE

(local 'table '(
    ((a int int) (+ a 24))
    ((a int int) (- a 4))
    ((a int int) (* a 11))
))

(local 'res 0)
(for it table (= res (it res)) )

(== res (* (- (+ 0 24) 4) 11)) ; ((0 + 24) - 11) * 11


