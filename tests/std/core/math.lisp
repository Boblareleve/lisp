TRUE

(import "core.lisp")


(print 'there)
(print +=)
(print 'and_?)


(let v+ 0)
(+= v+ 10)

(print 'then)

(let v- 10)
(-= v- 1)

(let v* 3)
(*= v* 9)


(let v% 10)
(%= v% 3)

(let v/ 10)
(/= v/ 3)



(&&
    (== v+ 10)
    (== v- 9)
    (== v* 27)
    (== v% 1)
    (== v/ 3)
    t
    t
)

