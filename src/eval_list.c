#include "eval_list.h"

#define EVAL_LIST_ASSERT(expr) assert(expr);

static inline List add_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = a.integer + b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_integer,
                .real = (double)a.integer + b.real
            };
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_real,
                .real = a.real + (double)b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = a.real + b.real
            };
    }
    return NIL_LIST;
}
static inline List sub_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = a.integer - b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = (double)a.integer - b.real
            };
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_real,
                .real = a.real - (double)b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = a.real - b.real
            };
    }
    return NIL_LIST;
}
static inline List mult_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = a.integer * b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = (double)a.integer * b.real
            };
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_real,
                .real = a.real * (double)b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = a.real * b.real
            };
    }
    return NIL_LIST;
}
static inline List div_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = a.integer / b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = (double)a.integer / b.real
            };
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_real,
                .real = a.real / (double)b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_real,
                .real = a.real / b.real
            };
    }
    return NIL_LIST;
}
static inline List idiv_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = a.integer / b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_integer,
                .integer = a.integer / (int64_t)b.real
            };
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return (List){
                .tag = tag_integer,
                .integer = (int64_t)a.real / b.integer
            };
        else if (b.tag == tag_real)
            return (List){
                .tag = tag_integer,
                .integer = (int64_t)a.real / (int64_t)b.real
            };
    }
    return NIL_LIST;
}
static inline List le_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer <= b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.integer <= b.real ? TRUE_LIST : NIL_LIST;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real <= b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.real <= b.real ? TRUE_LIST : NIL_LIST;
    }
    return NIL_LIST;
}
static inline List ge_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer >= b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.integer >= b.real ? TRUE_LIST : NIL_LIST;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real >= b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.real >= b.real ? TRUE_LIST : NIL_LIST;
    }
    return NIL_LIST;
}
static inline List l_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer < b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.integer < b.real ? TRUE_LIST : NIL_LIST;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real < b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.real < b.real ? TRUE_LIST : NIL_LIST;       
    }
    return NIL_LIST;
}
static inline List g_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer > b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.integer > b.real ? TRUE_LIST : NIL_LIST;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real > b.integer ? TRUE_LIST : NIL_LIST;
        else if (b.tag == tag_real)
            return a.real > b.real ? TRUE_LIST : NIL_LIST;
    }
    return NIL_LIST;
}
static inline List typeof_List(List li)
{
    List res = {
        .tag = tag_type,
        .type_tag = li.tag,
        .size = (li.tag == tag_list) ? li.size : 0
    };
    return res;
}

// if (List_equal_lit(op, "local")) // create and initilize a local variable
bool primitive_local(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "local"));
    UNUSED(out);
    TRY(li.size == 3, error_log("expected 3 element for 'local' got %d", li.size));
    TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(li.list[1].tag)));
    
    Variable var = { .name = li.list[1], .type = ANY_TYPE };
    TRY(eval(li.list[2], &var.value));

    local_Variable(var);

    return true;
}

// if (List_equal_lit(op, "tlocal")) // create and initilize a local variable
bool primitive_tlocal(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "tlocal"));
    UNUSED(out);
    TRY(li.size == 4, error_log("expected 4 element for 'tlocal' (type local) got %d", li.size));

    const List name = li.list[1];
    TRY(name.tag == tag_symbole, error_log("expected a symbole in 'tlocal' to got %s", tag_to_string(li.list[1].tag)));
    
    List type = {0};
    TRY(eval(li.list[2], &type));
    TRY(type.tag == tag_type, error_log("expected a type in 'tlocal' got %s", tag_to_string(type.tag)));

    Variable var = { .name = name, .type = type };
    TRY(eval(li.list[3], &var.value));
    TRY(is_of_type(var.value, var.type), error_log("set value in 'tlocal' is not in the expected type"));

    local_Variable(var);

    return true;
}

// if (List_equal_lit(op, "global")) // create and initilize a global variable
bool primitive_global(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "global"));
    UNUSED(out);
    TRY(li.size == 3, error_log("expected 3 element for 'global' got %d", li.size));
    TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to 'global' to got %s", tag_to_string(li.list[1].tag)));

    Variable var = { 
        .name = li.list[1], 
        .type = ANY_TYPE
    };
    TRY(eval(li.list[2], &var.value));

    TRY(global_Variable(var), error_log("global variable %sv already exist", List_to_Strv(var.name)));
    
    return true;
}

// if (List_equal_lit(op, "defun")) // same as global but don't eval argument 
bool primitive_defun(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "defun"));
    UNUSED(out);
    TRY(li.size == 3, error_log("expected 3 element for 'defun' got %d elements", li.size));
    TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to 'defun' to got %s", tag_to_string(li.list[1].tag)));

    Variable var = {
        .name = li.list[1],
        .value = li.list[2],
        .type = ANY_TYPE
    };

    // set or replace function var.name
    TRY(global_Variable(var), error_log("global variable %sv already exist", List_to_Strv(var.name)));
    
    return true;
}

// if (List_equal_lit(op, "=")) // change value of a variable
bool primitive_assign(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "="));
    UNUSED(out);
    TRY(li.size == 3, error_log("expected 3 element for '=' got %d", li.size));
    if (li.list[1].tag == tag_symbole)
    {
        Variable var = { .name = li.list[1], .type = ANY_TYPE };
        TRY(eval(li.list[2], &var.value));
        if (var.value.tag == tag_reference)
        {
            assert(var.value.list);
            TRY(eval(li.list[2], var.value.list));
            return true;
        }
        TRY(mutate_Variable(var));
        return true;
    }

    List left = li.list[1];
    if (left.tag != tag_reference)
        TRY(eval(left, &left));

    if (left.tag == tag_reference)
    {
        assert(left.list);
        TRY(eval(li.list[2], left.list));
        return true;
    }

    error_log("expected a symbole or a reference to set to got %s", tag_to_string(li.list[1].tag));
    return false;
}

// if (List_equal_lit(op, "[]="))
/* bool primitive_bracket_assign(const List li, List *out)
{
    UNUSED(out);
    TRY(li.size == 4, error_log("expected 4 element list for []= got %d", li.size));
    
    List list = {0};
    TRY(eval(li.list[1], &list));
    TRY(list.tag == tag_list, error_log("expected a list to index %s", tag_to_string(list.tag)));
    
    List index_l = {0};
    TRY(eval(li.list[2], &index_l));
    TRY(index_l.tag == tag_integer, error_log("expected an index %s", tag_to_string(index_l.tag)));
    int i_index_l = index_l.integer;
    TRY(0 <= i_index_l && i_index_l < list.size, error_log("out of bounds %d is not range of list of size %d", i_index_l, list.size));
    
    List res = {0};
    TRY(eval(li.list[3], &res));
    list.list[i_index_l] = res;
    return true;

    // TODO: pack unpack (py)->  [a, b] = [1, 2][:]
    // ([]= '(a b c) 0 3 '(A B C))
}
 */


 // if (List_equal_lit(op, "[]"))
bool primitive_square_bracket(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "[]"));
    TRY(li.size == 3 || li.size == 4, error_log("expected 3 or 4 element for '[]' got %d", li.size));
    
    List list = {0};
    TRY(eval(li.list[1], &list));
    TRY(list.tag == tag_list, error_log("expected a list to index %s", tag_to_string(list.tag)));
    
    List index_l = {0};
    TRY(eval(li.list[2], &index_l));
    TRY(index_l.tag == tag_integer, error_log("expected an index %s", tag_to_string(index_l.tag)));
    int i_index_l = index_l.integer;
    TRY(0 <= i_index_l && i_index_l < list.size, error_log("out of bounds %d is not range of list of size %d", i_index_l, list.size));

    List index_h = {0};
    if (li.size == 4)
    {
        TRY(eval(li.list[3], &index_h));
        TRY(index_h.tag == tag_integer, error_log("expected an index %s", tag_to_string(index_h.tag)));
        int i_index_h = index_h.integer;
        TRY(i_index_l < i_index_h && i_index_h <= list.size, error_log("out of bounds %d is not range of list of size %d", i_index_h, list.size));
        *out = list;
        out->size = i_index_h - i_index_l; // [] '(1 2 3) 1 2 -> .size = 1  
        out->offset += i_index_l;          //                 -> offset+1
        out->list   += i_index_l;          //                 -> ptr + 1
        // printf("->> [%d:%d] %d %d\n", i_index_l, i_index_h, out->size, out->offset);
        return true;
    }
    
    *out = (List){
        .tag = tag_reference,
        .list = &list.list[i_index_l],
        .offset = i_index_l
    };
    return true;
}

// if (List_equal_lit(op, "copy"))
bool primitive_copy(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "copy"));
    TRY(li.size == 2, error_log("expected 2 argument for 'copy' got %d elements", li.size));

    List to_copy = {0};
    TRY(eval(li.list[1], &to_copy));
    *out = List_copy(to_copy);
    return true;
}

// if (List_equal_lit(op, "?"))
bool primitive_exclamation_mark(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "?"));
    TRY(li.size == 4, error_log("expected 4 element for '?' got %d", li.size));
    List cond = {0};
    TRY(eval(li.list[1], &cond));
    return eval(li.list[(!IS_NIL(cond)) ? 2 : 3], out);
}

// if (List_equal_lit(op, "if"))
bool primitive_if(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "if"));
    TRY(li.size == 3, error_log("expected 3 element for 'if' got %d", li.size));
    List cond = {0};
    TRY(eval(li.list[1], &cond));
    if (!IS_NIL(cond))
        return eval(li.list[2], out);
    return true;
}

// if (List_equal_lit(op, "print"))
bool primitive_print(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "print"));
    UNUSED(out);
    TRY(li.size >= 2, error_log("expected at least 2 elements for 'print' got %d", li.size));

    for (int i = 1; i < li.size; i++)
    {
        List li_to_print = {0};
        TRY(eval(li.list[i], &li_to_print), error_log("failed to eval to 'print'"));
        TRY(List_print(li_to_print), error_log("failed to print"));
    }
    return true;
}

// if (List_equal_lit(op, "while"))
bool primitive_while(const List li, List *out)
{ // return last value of the body and of the last iteration or () if no body
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "while"));
    TRY(li.size >= 2, error_log("expected at least 2 elements for 'while' got %d", li.size));
    for (;;)
    {
        // the condition can have side effects
        List cond = {0};
        TRY(eval(li.list[1], &cond));
        
        if (IS_NIL(cond))
            break;
        for (int i = 2; i < li.size; i++)
            TRY(eval(li.list[i], out));
    }
    return true;
}

// if (List_equal_lit(op, "return"))
bool primitive_return(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "return"));
    TRY(li.size == 1 || li.size == 2, error_log("expected 2 or 3 elements for 'return' got %d", li.size));
    if (li.size == 2)
        TRY(eval(li.list[1], out));
    
    g_ctx->in_return = true;
    return false;
}

// if (List_equal_lit(op, "+"))
bool primitive_plus(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "+"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '+' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        
        TRY(eval(li.list[i], &operand));
        TRY((res = add_List(res, operand)).tag != tag_list);
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "++"))
bool primitive_increment(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "++"));
    TRY(li.size == 2, error_log("expected 2 elements for '++' got %d", li.size));
    TRY(li.list[1].tag == tag_symbole, error_log("can only increment variable got %s", tag_to_string(li.list[1].tag)));
    
    Variable *to_inc = get_Variable(li.list[1]);
    TRY(to_inc, error_log("variable \"%.*s\" to increment not found", li.list[1].size, li.list[1].str));
    TRY(to_inc->value.tag == tag_integer, error_log("try to increment %s", tag_to_string(to_inc->value.tag)));
    to_inc->value.integer += 1;
    *out = to_inc->value;
    return true;
}

// if (List_equal_lit(op, "--"))
bool primitive_decrement(const List li, List *out)
{
    // EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "--"));
    TRY(li.size == 2, error_log("expected 2 elements for '==' got %d", li.size));
    TRY(li.list[1].tag == tag_symbole, error_log("can only decrement variable got %s", tag_to_string(li.list[1].tag)));
    
    Variable *to_dec = get_Variable(li.list[1]);
    TRY(to_dec, error_log("variable \"%.*s\" to decrement not found", li.list[1].size, li.list[1].str));
    TRY(to_dec->value.tag == tag_integer, error_log("try to decrement %s", tag_to_string(to_dec->value.tag)));
    to_dec->value.integer -= 1;
    *out = to_dec->value;
    return true;
}

// if (List_equal_lit(op, "-"))
bool primitive_minus(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "-"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '-' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));
    TRY(res.tag == tag_integer);

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        
        TRY(eval(li.list[i], &operand));
        TRY((res = sub_List(res, operand)).tag != tag_list, error_log("expected a number to subtruct got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "*"))
bool primitive_product(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "*"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '*' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = mult_List(res, operand)).tag != tag_list, error_log("expected a number to multiply got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "/"))
bool primitive_div(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "/"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '/' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = div_List(res, operand)).tag != tag_list, error_log("expected a number to divide got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "//"))
bool primitive_integer_div(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "//"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '//' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = idiv_List(res, operand)).tag != tag_list, error_log("expected a number to divide integer got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "=="))
bool primitive_equal(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "=="));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '==' got %d", li.size));
    
    List acc = {0};
    TRY(eval(li.list[1], &acc));
    
    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        
        if (!List_equal(acc, operand))
            return true; // out is already set to nil 
        
    }
    *out = TRUE_LIST;
    return true;
}

// if (List_equal_lit(op, "<="))
bool primitive_less_or_equal_than(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "<="));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '<=' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = le_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, ">="))
bool primitive_more_or_equal_than(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], ">="));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '>=' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = ge_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, ">"))
bool primitive_more_than(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], ">"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '>' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = g_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "<"))
bool primitive_less_than(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "<"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '<' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        TRY((res = l_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
    }
    *out = res;
    return true;
}

// if (List_equal_lit(op, "!="))
bool primitive_not_equal(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "!="));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '!=' got %d", li.size));
    
    List acc = {0};
    TRY(eval(li.list[1], &acc));
    
    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        
        if (List_equal(acc, operand))
            return true; // out is already set to nil 
    }
    *out = TRUE_LIST;
    return true;
}

// if (List_equal_lit(op, "!"))
bool primitive_not(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "!"));
    TRY(li.size >= 2, error_log("expected at least 2 elements for '==' got %d", li.size));
    
    List res = {0};
    TRY(eval(li.list[1], &res));
    if (IS_NIL(res))
        *out = TRUE_LIST;
    return true;
}

// if (List_equal_lit(op, "&&"))
bool primitive_and(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "&&"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '&&' got %d", li.size));
    
    for (int i = 1; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        
        if (IS_NIL(operand))
            return true; // out is already set to nil (false)
    }
    *out = TRUE_LIST;
    return true;
}

// if (List_equal_lit(op, "||"))
bool primitive_or(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "||"));
    TRY(li.size >= 3, error_log("expected at least 3 elements for '||' got %d", li.size));
    
    for (int i = 2; i < li.size; i++)
    {
        List operand = {0};
        TRY(eval(li.list[i], &operand));
        
        if (!IS_NIL(operand))
        {
            *out = TRUE_LIST;
            return true;
        }
    }
    return true; // out is already set to nil (false)
}

// if (List_equal_lit(op, "first"))
bool primitive_first(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "first"));
    TRY(li.size == 2, error_log("expected 2 elements for 'first' got %d", li.size));
    TRY(eval(li.list[1], out),  *out = NIL_LIST);
    TRY(out->tag == tag_list,        *out = NIL_LIST);
    TRY(out->size > 0,               *out = NIL_LIST; error_log("can't take first element of an empty list"));
    
    *out = out->list[0];
    return true;
}

// if (List_equal_lit(op, "next"))
bool primitive_next(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "next"));
    TRY(li.size == 2, error_log("expected 2 elements for 'next' got %d", li.size));
    TRY(eval(li.list[1], out));
    TRY(out->tag == tag_list, *out = NIL_LIST);
    
    if (out->quote_count > 0)
    { // (next (quote (a b))) -> (a b)
        out->quote_count--;
    }
    else if (out->size > 1)
    {
        *out = (List){
            .tag = tag_list,
            .offset = out->offset + 1,
            .size = out->size - 1,
            .list = &out->list[1]
        };
    }
    else
        *out = NIL_LIST;
    return true;
}

// if (List_equal_lit(op, "for"))
bool primitive_for(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "for"));
    TRY(li.size >= 4, error_log("expected 4 elements for 'for' got %d", li.size));
    TRY(li.list[1].tag == tag_symbole);
    
    List iterable = {0};
    TRY(eval(li.list[2], &iterable));
    TRY(iterable.tag == tag_list);
    // TYPED optionnal
    size_t it_idx = local_Variable((Variable){ 
        .name = li.list[1], 
        .value = {
            .tag = tag_reference,
            .list = List_alloc(sizeof(List))
        },
        .type = ANY_TYPE
    });

    size_t frame_idx = g_ctx->args_stack.size - 1;
    for (int i = 0; i < iterable.size; i++)
    {
        assert(g_ctx->args_stack.arr[frame_idx].arr[it_idx].value.list);
        *g_ctx->args_stack.arr[frame_idx].arr[it_idx].value.list = iterable.list[i];
        
        for (int j = 3; j < li.size; j++)
            TRY(eval(li.list[j], out));
    }
    
    return true;
}

// if (List_equal_lit(op, "format"))
bool primitive_format(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "format"));
    TRY(li.size >= 2, error_log("expected at least 2 elements for 'format' got %d", li.size));

    static Strb acc = {0};
    acc.size = 0;

    for (int i = 1; i < li.size; i++)
    {
        List tmp = {0};
        TRY(eval(li.list[i], &tmp));
        TRY(dump(&acc, tmp));
    }
    
    *out = (List){
        .tag = tag_string,
        .size = acc.size,
        .str = List_duplicate(acc.arr, acc.size)
    };
    return true;
}

// if (List_equal_lit(op, "quote"))
bool primitive_quote(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "quote"));
    TRY(li.size == 2, error_log("expected 2 elements for 'quote' got %d", li.size));

    *out = li.list[1];
    return true;
}

// if (List_equal_lit(op, "typeof"))
bool primitive_typeof(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "typeof"));
    TRY(li.size == 2, error_log("expected 2 elements for 'typeof' got %d", li.size));
    TRY(eval(li.list[1], out));
    *out = typeof_List(*out);
    return true;
}

// if (List_equal_lit(op, "eval"))
bool primitive_eval(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "eval"));
    TRY(li.size >= 2, error_log("expected at least 2 elements for 'eval' got %d", li.size));
    *out = (List){
        .tag = tag_list,
        .size = li.size-1,
        .list = List_alloc(sizeof(List) * (li.size - 1))
    };
    for (int i = 1; i < li.size; i++)
        TRY(eval(li.list[i], &out->list[i-1]));
    
    return true;
}

// if (List_equal_lit(op, "type"))
bool primitive_type(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "type"));
    TODO("type");
    TRY(li.size == 2, error_log("expected 2 elements for 'type' got %d", li.size));
    *out = (List){
        .tag = tag_type,
        .type_tag = li.list[1].tag,
    };
    if (out->tag == tag_list)
    {
        out->size = li.list[1].size,
        out->list = List_alloc(sizeof(List) * (li.size - 1));
    }
    for (int i = 1; i < li.size; i++)
        TRY(eval(li.list[i], &out->list[i-1]));
    
    return true;
}

// if (List_equal_lit(op, "len"))
bool primitive_len(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "len"));
    TRY(li.size == 2, error_log("expected 2 elements for 'len' got %d", li.size));
    List list = {0};
    TRY(eval(li.list[1], &list));
    TRY(list.tag == tag_list 
        || list.tag == tag_string
        || list.tag == tag_symbole);
    *out = (List){
        .tag = tag_integer,
        .integer = list.size
    };
    return true;
}

// if (List_equal_lit(op, "$"))
bool primitive_dollar(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "$"));
    TODO("$");
    TRY(li.size >= 3, error_log("expected at least 2 elements for '$' got %d", li.size));
    List res = {0};
    TRY(eval(li.list[1], &res));

    for (int i = 2; i < li.size; i++)
    {
    }
    *out = res;
    return true;
}

// create a copy but with evaluated elements
bool primitive_list(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "list"));

    TRY(li.size >= 2, error_log("expected at least 2 elements for 'list' got %d", li.size));
    List res = {
        .tag = tag_list,
        .size = li.size - 1,
        .list = List_alloc(sizeof(List) * (li.size-1))
    };
    for (int i = 1; i < li.size; i++)
        TRY(eval(li.list[1], &res.list[i-1]));
    
    *out = res;
    return true;
}

// create an array of size n
bool primitive_array(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "array"));

    TRY(li.size == 2, error_log("expected 2 elements for 'array' got %d", li.size));

    List count = {0};
    TRY(eval(li.list[1], &count));
    TRY(count.tag == tag_integer);
    // printf("--array> %ld\n", count.integer);
    
    *out = (List){
        .tag = tag_list,
        .size = count.integer,
        .list = List_alloc(sizeof(List) * count.integer)
    };
    return true;
}

bool primitive_reference(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "reference"));

    TRY(li.size == 2, error_log("expected 2 elements for 'reference' got %d", li.size));
    // List ref = {0};
    // TRY(eval(li.list[1], ));
    *out = (List){
        .tag = tag_reference,
        .list = List_duplicate(&li.list[1], sizeof(li.list[1]))
    };
    
    return true;
}

bool primitive_dereference(const List li, List *out)
{
    EVAL_LIST_ASSERT(List_equal_lit(li.list[0], "dereference"));


    TRY(li.size == 2, error_log("expected 2 elements for 'dereference' got %d", li.size));
    List to_deref = {0};
    TRY(eval(li.list[1], &to_deref));
    TRY(to_deref.tag == tag_reference, error_log("exprected a tag_reference but got %s", tag_to_string(to_deref.tag)));
    *out = *to_deref.list;

    return true;
}


typedef struct Primitive
{
    List name;
    primitive_t fun;
} Primitive;


uint64_t set_Primitive_hash(const Primitive prim, uint64_t seed)
{
    const List str = prim.name;
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8; // <- useless
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}
int set_Primitive_equal(const Primitive a, const Primitive b)
{
    return List_str_equal(a.name, b.name);
}


SET_TYPEDEF_HASH_SET(Primitive);
#define SET_PRIM_IS_NULL(p) ((p).fun == NULL)
#define SET_PRIM_SET_NULL(p) ((p).fun = NULL)
SET_IMPLEMENT_HASH_SET(Primitive, SET_PRIM_IS_NULL, SET_PRIM_SET_NULL, 2, 0.7, 64);


static const Primitive keys[] = {
    { .name = _cstr_to_List("local"),       .fun = primitive_local              },
    { .name = _cstr_to_List("tlocal"),      .fun = primitive_tlocal             },
    { .name = _cstr_to_List("global"),      .fun = primitive_global             },
    { .name = _cstr_to_List("defun"),       .fun = primitive_defun              },
    { .name = _cstr_to_List("="),           .fun = primitive_assign             },
    { .name = _cstr_to_List("[]"),          .fun = primitive_square_bracket     },
    { .name = _cstr_to_List("copy"),        .fun = primitive_copy               },
    { .name = _cstr_to_List("?"),           .fun = primitive_exclamation_mark   },
    { .name = _cstr_to_List("if"),          .fun = primitive_if                 },
    { .name = _cstr_to_List("print"),       .fun = primitive_print              },
    { .name = _cstr_to_List("while"),       .fun = primitive_while              },
    { .name = _cstr_to_List("return"),      .fun = primitive_return             },
    { .name = _cstr_to_List("+"),           .fun = primitive_plus               },
    { .name = _cstr_to_List("++"),          .fun = primitive_increment          },
    { .name = _cstr_to_List("--"),          .fun = primitive_decrement          },
    { .name = _cstr_to_List("-"),           .fun = primitive_minus              },
    { .name = _cstr_to_List("*"),           .fun = primitive_product            },
    { .name = _cstr_to_List("/"),           .fun = primitive_div                },
    { .name = _cstr_to_List("//"),          .fun = primitive_integer_div        },
    { .name = _cstr_to_List("=="),          .fun = primitive_equal              },
    { .name = _cstr_to_List("<="),          .fun = primitive_less_or_equal_than },
    { .name = _cstr_to_List(">="),          .fun = primitive_more_or_equal_than },
    { .name = _cstr_to_List(">"),           .fun = primitive_more_than          },
    { .name = _cstr_to_List("<"),           .fun = primitive_less_than          },
    { .name = _cstr_to_List("!="),          .fun = primitive_not_equal          },
    { .name = _cstr_to_List("!"),           .fun = primitive_not                },
    { .name = _cstr_to_List("&&"),          .fun = primitive_and                },
    { .name = _cstr_to_List("||"),          .fun = primitive_or                 },
    { .name = _cstr_to_List("first"),       .fun = primitive_first              },
    { .name = _cstr_to_List("next"),        .fun = primitive_next               },
    { .name = _cstr_to_List("for"),         .fun = primitive_for                },
    { .name = _cstr_to_List("format"),      .fun = primitive_format             },
    { .name = _cstr_to_List("quote"),       .fun = primitive_quote              },
    { .name = _cstr_to_List("typeof"),      .fun = primitive_typeof             },
    { .name = _cstr_to_List("eval"),        .fun = primitive_eval               },
    { .name = _cstr_to_List("type"),        .fun = primitive_type               },
    { .name = _cstr_to_List("len"),         .fun = primitive_len                },
    { .name = _cstr_to_List("reference"),   .fun = primitive_reference          },
    { .name = _cstr_to_List("dereference"), .fun = primitive_dereference        },
    { .name = _cstr_to_List("list"),        .fun = primitive_list               },
    { .name = _cstr_to_List("array"),       .fun = primitive_array              },

};

/* uint32_t primitive_hash(const List str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8; // <- useless
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}
// parameters: s6436: c0 a71
static primitive_t map[71] = {0};
 */

static set_Primitive map = {0};
void init_primitive_map(void)
{
    static_for (i, keys)
        set_Primitive_insert(&map, keys[i]);
    // {
        // int h = primitive_hash(keys[i].name, 6436) % ARRAY_LEN(map);
        // map[h] = keys[i].fun;
    // }
}

primitive_t get_Primitive(const List op)
{
    // return map[primitive_hash(op, 6436)%ARRAY_LEN(map)];
    return struct_unwrap(set_Primitive_get(&map, (Primitive){ .name = op }), fun);
}

bool test_get_Primitive(void)
{
    init_primitive_map();

    static_for (i, keys)
    {
        primitive_t p = get_Primitive(keys[i].name);
        // printf("-> p %p <> .fun %p\n", p, keys[i].fun);
        assert(keys[i].fun == p); // map[keys[i].name]);
    }
    return true;
}

/*



typedef uint32_t (*hash_t)(const List str, uint32_t seed);

uint32_t hash0(const List str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.str[0] << 3*8;
    else
        f2chars = (str.str[0]          << 0*8)
                | (str.str[1]          << 1*8)
                | (str.str[str.size-2] << 2*8)
                | (str.str[str.size-1] << 3*8);
    
    return f2chars * seed;
}

uint32_t hash1(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                // | (str.arr[1]          << 1*8)
                // | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8)
    ;
    
    return f2chars * seed;
}

uint32_t hash2(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                ^ (str.arr[1]          << 0*8))
                | ((str.arr[str.size-2] << 1*8)
                ^ (str.arr[str.size-1] << 1*8))
    ;
    
    return f2chars ^ seed;
}

uint32_t hash3(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return f2chars ^ seed;
}

uint32_t hash4(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return (f2chars ^ seed) + seed;
}

uint32_t hash5(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = (str.arr[0]          << 0*8)
                | (str.arr[1]          << 1*8)
                | (str.arr[str.size-2] << 2*8)
                | (str.arr[str.size-1] << 3*8);
    
    return f2chars + seed;
}

uint32_t hash6(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                + (str.arr[1]          << 0*8))
                | ((str.arr[str.size-2] << 1*8)
                + (str.arr[str.size-1] << 1*8));
    
    return f2chars ^ seed;
}

uint32_t hash7(const Strv str, uint32_t seed)
{
    uint32_t f2chars = 0;
    if (str.size == 0) return 0;
    
    if (str.size == 1)
        f2chars = str.arr[0] << 3*8;
    else
        f2chars = ((str.arr[0]          << 0*8)
                ^ (str.arr[1]           << 1))
                | ((str.arr[str.size-2] << 1*8)
                ^ (str.arr[str.size-1]  << (1*8+1)));
    
    return f2chars ^ seed;
}

uint32_t hash8(const Strv str, uint32_t seed)
{
    return Strv_hash(str, seed);
}


const hash_t hashs[] = {
    hash0,
    // hash1,
    // hash2,
    // hash3,
    // hash4,
    // hash5,
    // hash6,
    // hash7,
    // hash8
};



typedef struct
{
    uint32_t seed;
    size_t area;
    size_t count;
    size_t hash_index;
} Strategie;


int Strategie_cmp(const void *a, const void *b)
{
    const Strategie *pa = a;
    const Strategie *pb = b;
    if (pa->count != pb->count)
        return (pa->count - pb->count);
    return (pa->area - pb->area);
}


DA_TYPEDEF_ARRAY(Strategie);
DA_TYPEDEF_ARRAY(int);


int count_for_strategie(size_t seed, size_t hash_index, da_int *slots, bool print_buckets)
{
    for (int i = 0; i < (int)ARRAY_LEN(keys); i++)
    {
        uint32_t h = hashs[hash_index](keys[i].name, seed) % slots->size;
        if (print_buckets) printf("%.*s\t-> %d\n", keys[i].name.size, keys[i].name.arr, h);
        slots->arr[h]++;
    }
    // if (print_buckets) printf("\n");
    int count  = 0;
    // printf("s.size %d\n", slots->size);
    for (int i = 0; i < slots->size; i++)
    {
        if (print_buckets) printf("(%d)%d, ", i, slots->arr[i]);

        if (slots->arr[i] > 1)
        {
            count += slots->arr[i];
        }
        
    }
    if (print_buckets) printf("\n");
    memset(slots->arr, 0, slots->size * sizeof(slots->arr[0]));

    return count;
}

uint32_t search_seed(void)
{
    da_int slots = {0};
    da_push_nzeros(&slots, ARRAY_LEN(keys));
    
    da_Strategie counts = {0};
    da_push(&counts, (Strategie){ .count = INT32_MAX, .area = slots.size, .seed = 1 });
    

    static_for (h_fun, hashs)
    {
        for (int area = 0;  area < 80; area++)
        {
            (void)primes;
            // for (uint32_t seed = 1; seed < ARRAY_LEN(primes); seed++)
            for (uint32_t seed = 1; seed < 10000; seed++)
            {
                // int count  = count_for_strategie(primes[seed], h_fun, &slots, false);
                int count  = count_for_strategie(seed, h_fun, &slots, false);
                
                da_push(&counts, (Strategie){ 
                    .hash_index = h_fun,
                    .seed = seed,
                    .count = count,
                    .area = slots.size
                });
                
                da_qsort(&counts, Strategie_cmp);
                
                counts.size = MIN(30, counts.size);
            }
            da_push_zero(&slots);
        }
        slots.size = ARRAY_LEN(keys);
    }


    da_for (Strategie, it, &counts)
        printf("s%u: c%zu a%zu fun%ld\n", it->seed, it->count, it->area, it->hash_index);
    

    return counts.arr[0].seed;
}
*/
int _main(void)
{
    test_get_Primitive();

    // s9472: c1 a37 fun1
    // s2231: c2 a55 fun0
    // if (0)
    // {
    //     da_int slots = {0};
    //     da_push_nzeros(&slots, 55);
    //     printf("-> %d\n", count_for_strategie(2231, 0, &slots, true));
    // }
    // else
    //     search_seed();
    return 0;
}
