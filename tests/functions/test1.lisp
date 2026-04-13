; to define a global 'function
(global 'defun '(('name 'fun) (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))



;; (defun foo (() ()))
(global 'foo '(() ()))

;; (let a 0)
(var da (foo)) ; (da_make 8))
;; (local 'da (foo)) ; (da_make 8))
