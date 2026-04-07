t


(defun da_make ((n int list) (eval (list n) 0)))

(defun da_push ((obj list element any) 
    (local arr list ([] 0 obj))

    (if (< (len ([] 0 obj)) ([] 1 obj))
    (eval

        (local new (list (* 2 (len arr))))
        (local i 0)
        (while (< i (len arr))
            ([]= new i ([] arr i))   
        )
        (= arr new)
        ([]= obj 0 new)
    )) 
    ([]= arr ([] 1 obj) element)
))

(defun da_len ((obj list)
    ([] obj 1)
))

(defun da_slice ((obj list)
    ([] ([] 0 obj) 0 (da_len obj))
))

(defun da_pop ((obj list)
    ([]= obj 1 (- (da_len obj) 1))
))


(local da (da_make 10))

(print da)

(da_push da 1)

(print "\n\n" da)

t