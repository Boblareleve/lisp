"b foo:a"

(local a "a")


('(()
    ;; (print a)
    (format a)
    (local b "b")
    (format a)
    (local a "foo:a")
    (format b " " a)
))

;; ('(()
;;     (print a)
;;     (local b "b")
;;     (print a)
;;     (local a "foo:a")
;;     (print b " " a)
;; ))

