; to define a global 'function
(global 'defun '(('name 'fun) (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))

;; []=


;;  doesn't work name is not eval in (=) and (+)  
(global '+= '('('name symbole value) (= name (+ name value))))
(global '*= '('('name symbole value) (= name (* name value))))
(global '-= '('('name symbole value) (= name (- name value))))
(global '/= '('('name symbole value) (= name (/ name value))))
(global '%= '('('name symbole value) (= name (% name value))))



;; TODO: 
(global 'for '('('iter symbole li list ...)
    (local '__i 0)
    (local iter ())
    (while (< __i (len li))
        (= 'iter ([] li __i))
        (eval ...)
    )
))




t
