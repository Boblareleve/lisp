;; '(0 1 1 2 3 5 8 13 21 34)
t
(defun fib ((n)

    (set a 0)
    (set b 1)
    (set i 0)
    (while (!= i n)
        (set tmp a)
        (set a (+ b a))
        (set b tmp)
        (set i (+ i 1))
    )
    a
))
(print (fib 9)) (print " == ") (print 34)
(print "   ")
(print (fib 4)) (print " == ") (print 3)
(print "   ")
t