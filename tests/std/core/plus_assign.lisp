TRUE

(global '+= '('('name symbole value) (= name (+ (multi 2 name) value))))


(local 'v 0)
(+= v 10)

(== v 10)
