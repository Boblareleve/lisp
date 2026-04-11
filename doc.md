

A list start with '(' and end with ')'. Each element of a list is separated by white space or end of atom. 

Exemples :
 + () ; empty list
 + (1)
 + (1 a =)

A list can contain one of the following atoms:
 + list: () (...)
 + interger: 1, 100...
 + real: 1.0 1.1 0.1 "[0-9]+\.[0-9]*"
 + string: "" "14DSF" ")()" "\n" ; use C style escaping
 + symbole: ANY thing else = + dlqsmfjd 0s ; ALL the element in this lign are individual symbole "symbole:" also

On evaluation of atoms they can either return themself or do something
 + interger|real|string: return themself
 + list: look at the first element:
     - On primitive (ex: local, global...) it take the unique action describe later
     - else it expect the first element to be a function once evaluated see later
 + symbole: search for a variable in local scope then in global scoop (OPTI concern) and return it's value

a primitive is one of the following symbole with action associated
 + 'local':
    format: (local NAME VALUE)
    desc: store a newly created variable named NAME in the stack, and set it's value to VALUE evaluated  
 + 'global':
    format: (global NAME VALUE)
    desc: store a newly created variable named NAME in a global scoop, and set it's value to VALUE evaluated  
    return: ()
 + '=':
    format: (= NAME VALUE)
    desc: change the value of a variable named NAME to VALUE evaluated, start by searching the stack then global scoop if the value is a reference the original value is changed
 + '[]':
    format: ([] LIST INDEX)
    desc: index into LIST at index INDEX, INDEX is an integer and LIST a list
    return: LIST[INDEX]
 + '&[]':
    format: (&[] LIST INDEX)
    desc: index into LIST at index INDEX, INDEX is an integer and LIST a list
    return: &LIST[INDEX]
 + 'copy': 
    format: (copy LIST)
    return: a deep copy of LIST
 + '?':
    format: (? CONDITION ON_NOT_NIL ON_NIL)
    desc: eval condition
    return: ON_NOT_NIL evaluated if CONDITION evaluated is not equal to (), else ON_NIL evaluated
 <!-- + 'if': -->
 + 'print':
    format: (print ...)
    desc: write to stdout any list pass in parameter in a json like format
 + 'while':
    format: (while CONDITION ...)
    desc: while CONDITION evaluated is not equal to () evaluate sequenally ...
    return: value of the last argument last time evaluated  
 + 'return':
    format: (return [VALUE])
    desc: jump the execution to the function caller setting VALUE evaluated as the function call result
    return: NO_RETURN 
 + '+':
    format: (+ ...)
    desc: sum every argument in ... once evaluated
    return: the sum
 + '++': 
    format: (++ VALUE)
    desc: add one to the value of the variable or reference VALUE
    return: value of variable or reference incremented
 + 'decrement':
    format: (-- VALUE)
    desc: substract one to the value of the variable or reference VALUE
    return: value of variable or reference decremented
 + '-':
    format: (- ...)
    desc: substract every argument in ... from the second one once evaluated to the first
    return: the difference
 + '*': ... 
 + '/': ...
 + '//': ...
 + '==': ...
 + '<=': ...
 + '>=': ...
 + '>': ...
 + '<': ...
 + '!=': ...
 + '!': ...
 + '&&': ...
 + '||': ...
 + 'first':
    format: (first LIST)
    return: first element of LIST
 + 'next':
    format: (first LIST)
    return: element from second index to last one in a list
 + 'for': 
 + 'format': 
 + 'quote': 
 + 'typeof': 
 + 'eval': 
 + 'type': 
 + 'len': 
 + 'reference': 
 + 'dereference': 
 + 'list': 
 + 'array':
