 + lign character error info (using .str.arr ptr )
 + code stepping (hard)
 + optimize recursion
 + gc: ref counting

 + opperands: 
    - $
    - match (rust) (== "deux" (match '(1 2)
                     ('(1)   "un") 
                     ('(1 2) "deux")
                     ("default")
                  ))
    - eval / parse
    
 + test [] (index)
 + see 'static Strb' if they are ok
