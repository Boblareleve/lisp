t


; to define a global 'function
(global 'defun '(('name 'fun) (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))


;; use types


(defun []= ((dest_array index value)
    (= (&[] dest_array index) value)
))

(defun da_make ((list) (eval () 0)))

;; (defun da_push ((obj list element any) 
;;     (local 'arr list '([] obj 0))

;;     (if (< (len ([] obj 0)) ([] obj 1))
;;         (let new (array (* 2 (len arr))))
;;         (let i 0)
;;         (while (< i (len arr))
;;             ([]= new i ([] arr i))
;;         )
;;         (= arr new)
;;         ([]= obj 0 new)
;;     )
;;     ([]= arr ([] obj 1) element)
;;     (++ (&[] obj 1))
;; ))

(defun da_len ((obj list)
    ([] obj 1)
))

(defun da_slice ((obj list)
    ([] ([] 0 obj) 0 (da_len obj)) ; take a slice [0:len]
))

(defun da_pop ((obj list)
    ([]= obj 1 (- (da_len obj) 1))
))





(let da (da_make))


(print da)

;; (let arr list ([] da 0))



(if (<= (len ([] da 0)) ([] da 1))
    (let old ([] da 0))
    ([]= da 0 (array (+ (* (len old) 2) 1)))
)

([]= ([] da 0) ([] da 1) 67)
(++ (&[] da 1))

;; (da_push da 1)

(print da)

;; (let res t)


;; (defun &&= (('name value) (= name (&& name value))))

;; (&&
    (== (da_len da) 1)

;; )