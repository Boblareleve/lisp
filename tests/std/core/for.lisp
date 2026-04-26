TRUE

(import "core.lisp")


(local 'sum 0)
(for it '(1 2 3)
    
    (= sum (+ sum it))
    (= sum (+ sum it))
)

(&&
    (== sum 12)
    (== ()
        (for jt '() t t t)
    )
    t
)



