

#define STRING_IMPLEMENTATION
#define AR_IMPLEMENTATION
#include "lisp.h"


Strb error = {0};

#define VAR_IS_NULL(var)  ((var).name.str == NULL)
#define VAR_SET_NULL(var) ((var).name.str = NULL)



uint64_t set_Variable_hash(const Variable value, uint64_t seed)
{
    return Strv_hash(List_to_Strv(value.name), seed);
}
int set_Variable_equal(const Variable v1, const Variable v2)
{
    return List_str_equal(v1.name, v2.name);
}

SET_IMPLEMENT_HASH_SET(Variable, VAR_IS_NULL, VAR_SET_NULL, 4, 0.8, 4);



Variable *get_local_Variable(Lisp_context *ctx, List name)
{   
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&ctx->args_stack))
            if (List_str_equal(it->name, name))
                return it;
    return NULL;
}

Variable *get_global_Variable(Lisp_context *ctx, List name)
{   
    return set_Variable_get(&ctx->variables, (Variable){ .name = name });
}

Variable *get_function(Lisp_context *ctx, List name)
{   
    return set_Variable_get(&ctx->functions, (Variable){ .name = name });
}

Variable *get_Variable(Lisp_context *ctx, List name)
{
    Variable *res = get_local_Variable(ctx, name);
    if (res)
        return res;
    return get_global_Variable(ctx, name);
}

Variable *get_type(Lisp_context *ctx, List name)
{
    return set_Variable_get(&ctx->types, (Variable){ .name = name });
}

// return index in the call stack
size_t local_Variable(Lisp_context *ctx, Variable var)
{
    da_Variable *frame = &da_top(&ctx->args_stack);
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, frame)
            if (List_str_equal(it->name, var.name))
            {
                *it = var;
                return da_idx_for(it, frame);
            }
    da_push(frame, var);
    
    return frame->size - 1;
}

// return true if it remplace a global variable
bool global_Variable(Lisp_context *ctx, Variable var)
{
    // set or replace variable var.name
    Variable *old = set_Variable_emplace(&ctx->variables, var);
    *old = var;
    return !VAR_IS_NULL(*old);
}

// false on not found
bool mutate_Variable(Lisp_context *ctx, Variable var)
{
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&ctx->args_stack))
            if (List_str_equal(it->name, var.name))
            {
                TRY(is_of_type(var.value, it->type), error_log("mutate value in stack to a value of an unexpected type"));
                it->value = var.value;
                return true;
            }
    
    Variable *old = set_Variable_emplace(&ctx->variables, var);
    if (!VAR_IS_NULL(*old))
    {
        TRY(is_of_type(var.value, old->type), error_log("mutate value in stack to a value of an unexpected type"));
        old->value = var.value;
        return true;
    }

    return false;
}


// li: (((args_def ...) statements ...) args_call)
// or
// function_def != NULL => li: (name args_call) and function_def: ((args_def ...) statements ...)
bool eval_function(Lisp_context *ctx, const List li, const List *function_def, List *out)
{
    bool res = false;
    
    const List func_def = function_def ? *function_def : li.list[0];
    TRY(func_def.tag == tag_list && func_def.size >= 2, error_log("not a function definition"));

    const List args_def = func_def.list[0];
    TRY(args_def.tag == tag_list);

    
    int arg_position = 0;
    List return_type = ANY_TYPE; 
    { // push args with their names in stack
        da_Variable new_frame = {0};
        bool have_a_type_hint = true; // the last argument got a type hint -> if new hint -> it's the return type hint
        for (int i = 0; i < args_def.size; i++)
        {
            if (args_def.list[i].tag == tag_type)
            {
                if (have_a_type_hint)
                {
                    TRY(i + 1 == args_def.size, error_log("two function argument hint not at the end of the argument list"));
                    return_type = args_def.list[i];
                    continue; // <=> break;
                }
                TRY(i > 0, error_log("type decoration goes after a symbole"));
                da_top(&new_frame).type = args_def.list[i];
                TRY(is_of_type(da_top(&new_frame).value, da_top(&new_frame).type));
                have_a_type_hint = true;
                continue;
            }
            TRY(args_def.list[i].tag == tag_symbole);
            Variable *type = get_type(ctx, args_def.list[i]);
            if (type)
            {
                if (have_a_type_hint)
                {
                    TRY(i + 1 == args_def.size, error_log("two function argument hint not at the end of the argument list"));
                    return_type = type->value;
                    continue; // <=> break;
                }
                TRY(i > 0, error_log("type decoration goes after a symbole"));
                da_top(&new_frame).type = type->value;
                TRY(is_of_type(da_top(&new_frame).value, da_top(&new_frame).type));
                have_a_type_hint = true;
                continue;
            }
            have_a_type_hint = false; 
            da_push(&new_frame, (Variable){ .name = args_def.list[i], .type = ANY_TYPE });
            TRY(eval(ctx, li.list[arg_position++ +1], &da_top(&new_frame).value));
        }
        da_push(&ctx->args_stack, new_frame);
    }
    // TRY(args_def.size == li.size - 1, error_log("expected %d arguments got %d", args_def.size, li.size-1));
    
    // execute statements
    for (int i = 1; i+1 < func_def.size; i++)
        if (!eval(ctx, func_def.list[i], out) && ctx->in_return)
        { // return have been call
            error.size = 0;         // reset error need to find solution for that
            ctx->in_return = false; // not in return anymore
            res = true;
            goto end; // terminate
        }
    
    // return the last one
    GOTRY(eval(ctx, func_def.list[func_def.size-1], out));
    GOTRY(is_of_type(*out, return_type), error_log("function return unexpected type"));
end:
    res = true;
fail:
    assert(ctx->args_stack.size > 0);
    TRY(ctx->args_stack.size > 0, error_log("return from root"));
    da_free(&da_top(&ctx->args_stack));
    ctx->args_stack.size--;
    
    return res;
}


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
static inline List typeof_List(Lisp_context *ctx, List li)
{
    UNUSED(ctx);
    List res = {
        .tag = tag_type,
        .type_tag = li.tag,
        .size = (li.tag == tag_list) ? li.size : 0
    };
    return res;
}


bool eval(Lisp_context *ctx, const List li, List *out)
{
    TRY(out, error_log("no output"));
    *out = NIL_LIST;

    // dec ref count
    if (li.quote_count > 0)
    {
        *out = li;
        out->quote_count--;
        return true;
    }

    switch (li.tag)
    {
    // self-evaluating
    case tag_type:
    case tag_string:
    case tag_true:
    case tag_integer:
    case tag_real: {
        *out = li;
    } return true;

    case tag_symbole: {

        Variable *var = get_Variable(ctx, li);
        if (var)
        {
            *out = var->value;
            return true;
        }

        Variable *type = get_type(ctx, li);
        if (type)
        {
            *out = type->value;
            return true;
        }

        TRY(get_function(ctx, li),
            error_log("unexpected function symbole: %.*s", li.size, li.str)
        );
        error_log("no variable nor function named: %.*s", li.size, li.str);
    } return false;
    case tag_list: {

        // nil|false
        if (IS_NIL(li))
        {
            *out = li;
            return true;
        }

        const List op = *li.list; 
        if (op.tag != tag_symbole)
        { // can be an inline function
            TRY(eval_function(ctx, li, NULL, out), error_log("failed to call inline function"));
            return true;
        }
        
        // TODO transform into an prefect hash table
        // uint16_t a = *(uint16_t)&op.str.arr;
        
        if (List_equal_lit(op, "local")) // create and initilize a local variable
        {
            TRY(li.size == 3, error_log("expected 3 element list for local got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1], .type = ANY_TYPE };
            TRY(eval(ctx, li.list[2], &var.value));

            local_Variable(ctx, var);

            return true;
        }
        if (List_equal_lit(op, "tlocal")) // create and initilize a local variable
        {
            TRY(li.size == 4, error_log("expected 4 element list for tlocal (type local) got %d", li.size));

            const List name = li.list[1];
            TRY(name.tag == tag_symbole, error_log("expected a symbole in tlocal to got %s", tag_to_string(li.list[1].tag)));
            
            List type = {0};
            TRY(eval(ctx, li.list[2], &type));
            TRY(type.tag == tag_type, error_log("expected a type in tlocal got %s", tag_to_string(type.tag)));

            Variable var = { .name = name, .type = type };
            TRY(eval(ctx, li.list[3], &var.value));
            TRY(is_of_type(var.value, var.type), error_log("set value in tlocal is not in the expected type"));

            local_Variable(ctx, var);

            return true;
        }
        if (List_equal_lit(op, "global")) // create and initilize a global variable
        {
            TRY(li.size == 3, error_log("expected 3 element list for local got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1], .type = ANY_TYPE };
            TRY(eval(ctx, li.list[2], &var.value));

            global_Variable(ctx, var);

            return true;
        }
        if (List_equal_lit(op, "=")) // change value of a variable
        {
            TRY(li.size == 3, error_log("expected 3 element list for set got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to set to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1], .type = ANY_TYPE };
            TRY(eval(ctx, li.list[2], &var.value));
            
            TRY(mutate_Variable(ctx, var));

            return true;
        }
        if (List_equal_lit(op, "[]"))
        {
            TRY(li.size == 3 || li.size == 4, error_log("expected 3 or 4 element list for [] got %d", li.size));
            
            List list = {0};
            TRY(eval(ctx, li.list[1], &list));
            TRY(list.tag == tag_list, error_log("expected a list to index %s", tag_to_string(list.tag)));
            
            List index_l = {0};
            TRY(eval(ctx, li.list[2], &index_l));
            TRY(index_l.tag == tag_integer, error_log("expected an index %s", tag_to_string(index_l.tag)));
            int i_index_l = index_l.integer;
            TRY(0 <= i_index_l && i_index_l < list.size, error_log("out of bounds %d is not range of list of size %d", i_index_l, list.size));

            List index_h = {0};
            if (li.size == 4)
            {
                TRY(eval(ctx, li.list[3], &index_h));
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

            *out = list.list[i_index_l];
            return true;
        }
        if (List_equal_lit(op, "[]="))
        {
            TRY(li.size == 4, error_log("expected 4 element list for []= got %d", li.size));
            
            List list = {0};
            TRY(eval(ctx, li.list[1], &list));
            TRY(list.tag == tag_list, error_log("expected a list to index %s", tag_to_string(list.tag)));
            
            List index_l = {0};
            TRY(eval(ctx, li.list[2], &index_l));
            TRY(index_l.tag == tag_integer, error_log("expected an index %s", tag_to_string(index_l.tag)));
            int i_index_l = index_l.integer;
            TRY(0 <= i_index_l && i_index_l < list.size, error_log("out of bounds %d is not range of list of size %d", i_index_l, list.size));
            
            List res = {0};
            TRY(eval(ctx, li.list[3], &res));
            list.list[i_index_l] = res;
            return true;

            // TODO: pack unpack (py)->  [a, b] = [1, 2][:]
            // ([]= '(a b c) 0 3 '(A B C))
        }
        if (List_equal_lit(op, "copy"))
        {
            TRY(li.size == 2, error_log("expected only 1 argument to be copyed got %d elements", li.size));

            List to_copy = {0};
            TRY(eval(ctx, li.list[1], &to_copy));
            *out = List_copy(ctx, to_copy);
            return true;
        }
        if (List_equal_lit(op, "defun"))
        {
            TRY(li.size == 3, error_log("expected 3 element list for defun got %d elements", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to defun to got %s", tag_to_string(li.list[1].tag)));

            Variable var = {
                .name = li.list[1],
                .value = li.list[2]
            };

            // set or replace function var.name
            Variable *old = set_Variable_emplace(&ctx->functions, var);
            
            *old = var;
            return true;
        }
        if (List_equal_lit(op, "?"))
        {
            TRY(li.size == 4, error_log("expected 4 element for '?' got %d", li.size));
            List cond = {0};
            TRY(eval(ctx, li.list[1], &cond));
            return eval(ctx, li.list[(!IS_NIL(cond)) ? 2 : 3], out);
        }
        if (List_equal_lit(op, "if"))
        {
            TRY(li.size == 3, error_log("expected 3 element for 'if' got %d", li.size));
            List cond = {0};
            TRY(eval(ctx, li.list[1], &cond));
            if (!IS_NIL(cond))
                return eval(ctx, li.list[2], out);
            return true;
        }
        if (List_equal_lit(op, "print"))
        {
            TRY(li.size >= 2, error_log("expected at least 2 elements for 'print' got %d", li.size));

            for (int i = 1; i < li.size; i++)
            {
                List li_to_print = {0};
                TRY(eval(ctx, li.list[i], &li_to_print), error_log("failed to eval to print"));
                TRY(List_print(li_to_print), error_log("failed to print"));
            }
            return true;
        }
        if (List_equal_lit(op, "while"))
        { // return last value of the body and of the last iteration or () if no body
            TRY(li.size >= 2); // the condition can have side effects
            for (;;)
            {
                List cond = {0};
                TRY(eval(ctx, li.list[1], &cond));
                
                if (IS_NIL(cond))
                    break;
                for (int i = 2; i < li.size; i++)
                    TRY(eval(ctx, li.list[i], out));
            }
            return true;
        }
        if (List_equal_lit(op, "return"))
        {
            TRY(li.size == 1 || li.size == 2);
            if (li.size == 2)
                TRY(eval(ctx, li.list[1], out));
            
            ctx->in_return = true;
            return false;
        }
        if (List_equal_lit(op, "+"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '+' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = add_List(res, operand)).tag != tag_list);
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "++"))
        {
            TRY(li.size == 2);
            TRY(li.list[1].tag == tag_symbole, error_log("can only increment variable got %s", tag_to_string(li.list[1].tag)));
            
            Variable *to_inc = get_Variable(ctx, li.list[1]);
            TRY(to_inc, error_log("variable \"%.*s\" to increment not found", li.list[1].size, li.list[1].str));
            TRY(to_inc->value.tag == tag_integer, error_log("try to increment %s", tag_to_string(to_inc->value.tag)));
            to_inc->value.integer += 1;
            *out = to_inc->value;
            return true;
        }
        if (List_equal_lit(op, "--"))
        {
            TRY(li.size == 2);
            TRY(li.list[1].tag == tag_symbole, error_log("can only decrement variable got %s", tag_to_string(li.list[1].tag)));
            
            Variable *to_dec = get_Variable(ctx, li.list[1]);
            TRY(to_dec, error_log("variable \"%.*s\" to decrement not found", li.list[1].size, li.list[1].str));
            TRY(to_dec->value.tag == tag_integer, error_log("try to decrement %s", tag_to_string(to_dec->value.tag)));
            to_dec->value.integer -= 1;
            *out = to_dec->value;
            return true;
        }
        if (List_equal_lit(op, "-"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '-' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == tag_integer);

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = sub_List(res, operand)).tag != tag_list, error_log("expected a number to subtruct got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "*"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '*' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = mult_List(res, operand)).tag != tag_list, error_log("expected a number to multiply got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "/"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '/' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = div_List(res, operand)).tag != tag_list, error_log("expected a number to divide got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "//"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '//' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = idiv_List(res, operand)).tag != tag_list, error_log("expected a number to divide integer got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "=="))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '==' got %d", li.size));
            
            List acc = {0};
            TRY(eval(ctx, li.list[1], &acc));
            
            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                
                if (!List_equal(acc, operand))
                    return true; // out is already set to nil 
                
            }
            *out = TRUE_LIST;
            return true;
        }
        if (List_equal_lit(op, "<="))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '<=' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = le_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, ">="))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '>=' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = ge_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, ">"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '>' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = g_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "<"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '<' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                TRY((res = l_than_List(res, operand)).tag != tag_list, error_log("expected a number to compare got %s", tag_to_string(operand.tag)));
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "!="))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '!=' got %d", li.size));
            
            List acc = {0};
            TRY(eval(ctx, li.list[1], &acc));
            
            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                
                if (List_equal(acc, operand))
                    return true; // out is already set to nil 
            }
            *out = TRUE_LIST;
            return true;
        }
        if (List_equal_lit(op, "!"))
        {
            TRY(li.size >= 2, error_log("expected at least 2 elements for '==' got %d", li.size));
            
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            if (IS_NIL(res))
                *out = TRUE_LIST;
            return true;
        }
        if (List_equal_lit(op, "&&"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '&&' got %d", li.size));
            
            for (int i = 1; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                
                if (IS_NIL(operand))
                    return true; // out is already set to nil (false)
            }
            *out = TRUE_LIST;
            return true;
        }
        if (List_equal_lit(op, "||"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '||' got %d", li.size));
            
            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list[i], &operand));
                
                if (!IS_NIL(operand))
                {
                    *out = TRUE_LIST;
                    return true;
                }
            }
            return true; // out is already set to nil (false)
        }
        if (List_equal_lit(op, "first"))
        {
            TRY(li.size == 2);
            TRY(eval(ctx, li.list[1], out),  *out = NIL_LIST);
            TRY(out->tag == tag_list,        *out = NIL_LIST);
            TRY(out->size > 0,               *out = NIL_LIST; error_log("can't take first element of an empty list"));
            
            *out = out->list[0];
            return true;
        }
        if (List_equal_lit(op, "next"))
        {
            TRY(li.size == 2);
            TRY(eval(ctx, li.list[1], out));
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
        if (List_equal_lit(op, "for"))
        {
            TRY(li.size >= 4);
            TRY(li.list[1].tag == tag_symbole);
            
            List iterable = {0};
            TRY(eval(ctx, li.list[2], &iterable));
            TRY(iterable.tag == tag_list);
            // TYPED optionnal
            size_t it_idx = local_Variable(ctx, (Variable){ .name = li.list[1], .type = ANY_TYPE }); 
            size_t frame_idx = ctx->args_stack.size - 1;
            for (int i = 0; i < iterable.size; i++)
            {
                ctx->args_stack.arr[frame_idx].arr[it_idx].value = iterable.list[i];
                
                for (int j = 3; j < li.size; j++)
                    TRY(eval(ctx, li.list[j], out));
            }
            
            return true;
        }
        if (List_equal_lit(op, "format"))
        { // catstr
            TRY(li.size >= 2);

            static Strb acc = {0};
            acc.size = 0;

            for (int i = 1; i < li.size; i++)
            {
                List tmp = {0};
                TRY(eval(ctx, li.list[i], &tmp));
                TRY(dump(&acc, tmp));
            }

            // Strb_fit(&acc);
            // acc.arr = List_delc_alloc(ctx, acc.arr, acc.size);


            // Strb_fit(&acc);
            *out = (List){
                .tag = tag_string,
                .size = acc.size,
                .str = List_duplicate(ctx, acc.arr, acc.size)
            };
            // Strb_free(acc);
            return true;
        }
        if (List_equal_lit(op, "quote"))
        { // catstr
            TRY(li.size == 2);
            *out = li.list[1];
            return true;
        }
        if (List_equal_lit(op, "typeof"))
        {
            TRY(li.size == 2);
            TRY(eval(ctx, li.list[1], out));
            *out = typeof_List(ctx, *out);
            return true;
        }
        if (List_equal_lit(op, "eval"))
        {
            TRY(li.size >= 2);
            *out = (List){
                .tag = tag_list,
                .size = li.size-1,
                .list = List_alloc(ctx, sizeof(List) * (li.size - 1))
            };
            for (int i = 1; i < li.size; i++)
                TRY(eval(ctx, li.list[i], &out->list[i-1]));
            
            return true;
        }
        if (List_equal_lit(op, "type"))
        {
            TODO("type");
            TRY(li.size == 2);
            *out = (List){
                .tag = tag_type,
                .type_tag = li.list[1].tag,
            };
            if (out->tag == tag_list)
            {
                out->size = li.list[1].size,
                out->list = List_alloc(ctx, sizeof(List) * (li.size - 1));
            }
            for (int i = 1; i < li.size; i++)
                TRY(eval(ctx, li.list[i], &out->list[i-1]));
            
            return true;
        }
        if (List_equal_lit(op, "len"))
        {
            TRY(li.size == 2);
            List list = {0};
            TRY(eval(ctx, li.list[1], &list));
            TRY(list.tag == tag_list 
             || list.tag == tag_string
             || list.tag == tag_symbole);
            *out = (List){
                .tag = tag_integer,
                .integer = list.size
            };
            return true;
        }
        if (List_equal_lit(op, "list"))
        {
            TRY(li.size == 2);
            List count = {0};
            TRY(eval(ctx, li.list[1], &count));

            TRY(count.tag == tag_integer);
            TRY(0 <= count.integer && count.integer < UINT16_MAX, error_log("too large new list of size %i64", count.integer));
            *out = (List){
                .tag = tag_list,
                .size = count.integer,
                .list = List_alloc(ctx, count.integer), // set all to NIL_LIST
            };

            return true;
        }

        /* if (List_equal_lit(op, "$"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '$' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == );

            for (int i = 2; i < li.size; i++)
            {
            }
            *out = res;
            return true;
        } */

        { // variable or function
            Variable *var_fun;
            var_fun = get_Variable(ctx, op);
            if (var_fun)
            { // got a local variable
                TRY(eval(ctx, var_fun->value, out));
                return true;
            }
            var_fun = get_function(ctx, op);
            if (var_fun)
            { // got function
                TRY(eval_function(ctx, li, &var_fun->value, out));
                return true;
            }
        }
        
        error_log("no primitive '%.*s' found to evaluate a list", op.size, op.str);
    } return false;

    default: UNREACHABLE("eval switch"); return false;
    }
}


bool List_equal(const List li1, const List li2)
{
    if (li1.tag != li2.tag || li1.quote_count != li2.quote_count)
        return false;
    
    switch (li1.tag)
    {
    case tag_integer: return li1.integer == li2.integer;
    case tag_real:    return fabs(li1.real - li2.real) < 1.0E-10;
    case tag_list: {
        TRY(li1.size == li2.size);
        for (int i = 0; i < li1.size; i++)
            TRY(List_equal(li1.list[i], li2.list[i]));
    } return true;
    case tag_string:    return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_symbole:   return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_true:      return li2.tag == tag_true;
    case tag_type:      return type_compatible(li1, li2);
    default: UNREACHABLE("List equal");
    }
    return false;
}


List List_copy(Lisp_context *ctx, const List li)
{
    if (li.tag == tag_list)
    {
        if (IS_NIL(li))
            return li;
        
        List res = {
            .tag = tag_list,
            .quote_count = li.quote_count,
            .offset = 0,
            .size = li.size,
            .list = List_alloc(ctx, li.size * sizeof(List))
        };

        for (size_t i = 0; i < li.size; i++)
            res.list[i] = List_copy(ctx, li.list[i]);
        
        return res;
    }
    return li;
}




// !!shortcut GC!!
void List_free(List *li)
{
    if (!li) return ;
    
    if (li->tag == tag_list)
    {
        if (IS_NIL(*li)) return ;

        for (size_t i = 0; i < li->size; i++)
            List_free(&li->list[i]);
    }
    free(List_get_ptr(li));
}


set_void_ptr add_to_gc_context(set_void_ptr gc, List root)
{
    if (List_get_ptr(&root))
        set_void_ptr_insert(&gc, List_get_ptr(&root));

    if (root.tag == tag_list)
        for (int i = 0; i < root.size; i++)
            gc = add_to_gc_context(gc, root.list[i]);
    
    return gc;
}

Lisp_context Lisp_context_init(List root)
{
    assert(root.tag == tag_list);
    Lisp_context res = {
        .gc = add_to_gc_context((set_void_ptr){0}, root),
        .root = root,
    };
    da_push_zero(&res.args_stack);

    // buildin types
    add_simple_type(&res, "list",    (List){ .tag = tag_type, .type_tag = tag_list, .size = TYPE_UNDEFINED_LIST_SIZE, });
    add_simple_type(&res, "symbole", (List){ .tag = tag_type, .type_tag = tag_symbole   });
    add_simple_type(&res, "int",     (List){ .tag = tag_type, .type_tag = tag_integer   });
    add_simple_type(&res, "float",   (List){ .tag = tag_type, .type_tag = tag_real      });
    add_simple_type(&res, "string",  (List){ .tag = tag_type, .type_tag = tag_string    });
    add_simple_type(&res, "type",    (List){ .tag = tag_type, .type_tag = tag_type      });
    add_simple_type(&res, "any",     (List){ .tag = tag_type, .type_tag = ttag_any_type });

    return res;
}

void Lisp_context_free(Lisp_context *ctx)
{
    assert(ctx->args_stack.size >= 1);

    { // free memory not tracked by gc
        da_for (da_Variable, it, &ctx->args_stack)
            da_free(it);
        da_free(&ctx->args_stack);
    
        set_Variable_free(&ctx->functions);
        set_Variable_free(&ctx->variables);
        set_Variable_free(&ctx->types);
        ctx->root = NIL_LIST;
    }

    // GGGGGGGGGGGGGC!!
    garbage_collector(ctx);

    
    set_void_ptr_free(&ctx->gc);


    // da_free(&ctx->args_stack.arr[0]);
    // da_free(&ctx->args_stack);
    // set_Variable_free(&ctx->variables);
    // set_Variable_free(&ctx->functions);
    // da_for (void *, it, &ctx->gc)
    //     free(*it);
    // da_free(&ctx->gc);
}

