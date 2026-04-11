t

(local 'a 0)
(++ a)

(== 
    (++ (reference 0))
    a
    1
)