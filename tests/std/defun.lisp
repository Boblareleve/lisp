; to define a global 'function
(global 'defun '(('name 'fun) (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))


;; TODO: 
(global 'for '(('iter symbole li list ...)
    (local i 0)
    (while (< i (len li)) (eval ...))
))
