EQUAL
6


(global 'for '('('iter li 'body)
    (local 'i 0)
    (local 'iter ([] li i))
    (while (< i (len li))
        (eval body)
    )
))

(local 'sum 0)
(for it '(1 2 3)
    ()
    (= 'sum (+ sum it))
)


