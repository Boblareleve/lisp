; to define a global 'function
(global 'defun '(('name 'fun) (global name fun) ))
; to define a local 'variable
(global 'let   '(('name value) (local name value) (upgrade name) ))
; to define a global 'variable
(global 'var   '(('name value) (global name value) ))

;; ([]= symbole|reference list int)
(global '[]= '('(li list index int value any) (= (&[] li index) value) li))

;; (_= sym value) <=> (= 'sym (_ sym value))
(global '+= '('('name symbole value any) (= name (+ (multi 2 name) value))))
(global '*= '('('name symbole value any) (= name (* (multi 2 name) value))))
(global '-= '('('name symbole value any) (= name (- (multi 2 name) value))))
(global '/= '('('name symbole value any) (= name (/ (multi 2 name) value))))


(global 'for '('('iter symbole li list ...)
    (local '__i 0)
    (local iter ())
    (while (< __i (len li))
        (= iter ([] li __i))
        (eval ...)
    )
))


t
