

(defun map ((l list fun list list)
    (local res (copy l))
    (local i 0)
    (while (< i (len l))
        ([] res i (fun ([] i l)))
        (++ i)
    )
    res
))