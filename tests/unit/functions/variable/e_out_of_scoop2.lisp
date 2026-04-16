ERROR ; foo is local can't be call inside a function

(local 'y 0)

('(()
    (local 'x 0)
))

x
