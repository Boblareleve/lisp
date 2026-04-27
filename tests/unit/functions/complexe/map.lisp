TRUE

(global 'map '((li list fun list)

    (= 'li (copy li))
    
    (local 'i 0)
    (while (< i (len li))
        (= (&[] li i) (fun ([] li i)))
        (++ i)
    )
    li
))



(local 'li '(1 2 3))

(== (map li '((x) (+ x 1))) '(2 3 4))


