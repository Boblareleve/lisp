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
    - T1 | T2

 + test: global variable colision

 +? local -> all accessible but ordonate and not unique 
 + reference:
 + hash table object
 + variatic function arguments the special variable '...' 
   get encapsulated into a ... roulement de tambour ... a list ; exemple  '((...) (len ...))
   can have a name exemple: args... but need to end with '...' and is use whiout them args
   '... 
 + macro: function but they have a special stack frame and don't eval their arguments
 
 + make primitive to be powerful and let user make wrapper function to have convinent way of using the funtionnality ?
