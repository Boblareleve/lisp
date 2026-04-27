TRUE

(import "core.lisp")




(let sum 0)
(for it '(1 2 3)
    
    
    (for jt '(1 2 3)
        
        (= 'sum (* sum jt))
    )

    (= 'sum (+ sum it))

)

(== sum 51)


