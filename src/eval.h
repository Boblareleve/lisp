#ifndef EVAL_LIST_H
#define EVAL_LIST_H

#include "lisp.h"

size_t local_Variable(Variable var);
bool global_Variable(Variable var);
bool mutate_Variable(Variable var);
Variable *get_Variable(List name);
Variable *get_local_Variable(List name);


#endif /* EVAL_LIST_H */
