EQUAL
10

(local 'sum 0)
(local 'i 4)

(while (!= i 0) 
    (= 'sum (+ sum i))
    (-- i)
)

sum
