;; '(0 1 1 2 3 5 8 13 21 34)
t
(global 'fib '((n)

    (local 'a 0)
    (local 'b 1)
    (local 'i 0)
    (while (!= i n)
        (= tmp a)
        (= a (+ b a))
        (= b tmp)
        (= i (+ i 1))
    )
    a
))

;; (print (fib 9)) (print " == ") (print 34)
;; (print "   ")
;; (print (fib 4)) (print " == ") (print 3)
;; (print "   ")
t