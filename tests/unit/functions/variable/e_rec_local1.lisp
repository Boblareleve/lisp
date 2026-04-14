ERROR ; foo is local can't be call inside a function

(local 'foo '(()
    (foo)
))

(foo)
