 + lign character error info (using .str.arr ptr )
 + code stepping (hard)
 + optimize recursion -> flatten nativ stack (no recusion) 
 + gc: test in an running program
 + integer/real
 + opperands: 
    - $
    - match (rust) (== "deux" (match '(1 2)
                     ('(1)   "un") 
                     ('(1 2) "deux")
                     ("default")
                  ))
    - eval / parse
 + C-API
 + handle the collision between types/local/global symboles
 + type
    - typeof list -> TYPE
    - how to handle list/tuple 