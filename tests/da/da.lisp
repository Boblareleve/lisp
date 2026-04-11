t
;; use types

(global '[]= '((dest_array index value)
    (= (&[] dest_array index) value)
))

(global 'da_make '((n int list) (eval (array n) 0)))

(global 'da_push '((obj list element any) 
    (local 'arr list ([] 0 obj))

    (if (< (len ([] 0 obj)) ([] 1 obj))
    (eval

        (local 'new (list (* 2 (len arr))))
        (local 'i 0)
        (while (< i (len arr))
            ([]= new i ([] arr i))   
        )
        (= arr new)
        ([]= obj 0 new)
    )) 
    ([]= arr ([] 1 obj) element)
))

(global 'da_len '((obj list)
    ([] obj 1)
))

(global 'da_slice '((obj list)
    ([] ([] 0 obj) 0 (da_len obj)) ; take a slice [0:len]
))

(global 'da_pop '((obj list)
    ([]= obj 1 (- (da_len obj) 1))
))


;; (local 'a 0)
(local 'da ('((a) (1, 2)) 10))

(print da)

(da_push da 1)

(print "\n\n" da)

t