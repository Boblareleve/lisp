t

; to define a global 'function
(global 'defun '(('name 'fun)  (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))





(defun foo ((a) (+ a 1)))
(let b 0)
(var g b)

(local 'cc 1)

(let c b)

(==
    (foo g)
    (- (foo cc) cc)
    (- (foo c)  c)
    (foo b)
    1
)
