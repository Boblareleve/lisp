EQUAL
15

('((...)
    (local 'sum 0)
    (local 'i 0)
    (while (< i (len ...))
        (= sum (+ sum ([] ... i)))
        (++ i)
    )
    sum
) 1 2 3 4 5)
