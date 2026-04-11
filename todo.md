 + lign character error info (using .str.arr ptr )
 + code stepping (hard)
 + optimize recursion -> flatten nativ stack (no recusion) 
 + gc: test in an running program
 + integer/real
 + opperands: 
    - ($ ($ 1 + 1) == 2)
    - match (rust) (== "deux" (match '(1 2)
                     ('(1)   "un") 
                     ('(1 2) "deux")
                     ("default")
                  ))
    - read entier file
    - eval / parse
 + C-API
 + type
    - typeof list -> TYPE
    - how to handle list/tuple 

 + test: global variable colision

 +? local -> all accessible but ordonate and not unique 
 + reference:
 + hash table object
 + variatic function arguments the special variable '...' 
   get encapsulated into a ... roulement de tambour ... a list ; exemple  '((...) (len ...))

 + make primitive to be powerful and let user make wrapper function to have convinent way of using the funtionnality ?
