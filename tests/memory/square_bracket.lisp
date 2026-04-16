t

(local 'array '(1 2 3 4 5))
;; t
(&&
    ;; (||
    ;;     (print  ([] array 4))
    ;;     (print  ([] array 0))
    ;;     (print  ([] array 3 5))
    ;;     (print  ([] array 0 5))
    ;;     t
    ;; )
    (== 5 ([] array 4))
    (== 1 ([] array 0))
    (== '(4 5) ([] array 3 5))
    (== array ([] array 0 5))
)
t