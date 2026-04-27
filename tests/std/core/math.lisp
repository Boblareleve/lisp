TRUE

(import "core.lisp")



(let v+ 0)
(+= v+ 10)

(let v- 10)
(-= v- 1)

(let v* 3)
(*= v* 9)



(let v/ 10)
(/= v/ 3)



(&&
    (== v+ 10)
    (== v- 9)
    (== v* 27)
    (== v/ 3)
    t
    t
)

