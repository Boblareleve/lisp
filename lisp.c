

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
                *it = var;
                return true;
            }
    
    Variable *old = set_Variable_emplace(&ctx->variables, var);
    if (!VAR_IS_NULL(*old))
    {
        *old = var;
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

    TRY(args_def.size == li.size - 1, error_log("expected %d arguments got %d", args_def.size, li.size-1));
    
    { // push args with their names in stack
        da_Variable new_frame = {0};
        for (int i = 0; i < args_def.size; i++)
        {
            // TODO set_local_Variable ?
            da_push(&new_frame, (Variable){ .name = args_def.list[i] });
            GOTRY(eval(ctx, li.list[i+1], &da_top(&new_frame).value));
        }
        da_push(&ctx->args_stack, new_frame);
    }

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
    

end:
    res = true;
fail:
    assert(ctx->args_stack.size > 0);
    TRY(ctx->args_stack.size > 0, error_log("return from root"));
    da_free(&da_top(&ctx->args_stack));
    ctx->args_stack.size--;
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
    case tag_string:
    case tag_true:
    case tag_number: {
        *out = li;
    } return true;

    case tag_symbole: {

        Variable *var = get_Variable(ctx, li);
        if (var)
        {
            *out = var->value;
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

            Variable var = { .name = li.list[1] };
            TRY(eval(ctx, li.list[2], &var.value));

            local_Variable(ctx, var);

            return true;
        }
        if (List_equal_lit(op, "global")) // create and initilize a global variable
        {
            TRY(li.size == 3, error_log("expected 3 element list for local got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1] };
            TRY(eval(ctx, li.list[2], &var.value));

            global_Variable(ctx, var);

            return true;
        }
        if (List_equal_lit(op, "=")) // change value of a variable
        {
            TRY(li.size == 3, error_log("expected 3 element list for set got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to set to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1] };
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
            TRY(index_l.tag == tag_number, error_log("expected an index %s", tag_to_string(index_l.tag)));
            int i_index_l = index_l.number;
            TRY(0 <= i_index_l && i_index_l < list.size, error_log("out of bounds %d is not range of list of size %d", i_index_l, list.size));

            List index_h = {0};
            if (li.size == 4)
            {
                TRY(eval(ctx, li.list[3], &index_h));
                TRY(index_h.tag == tag_number, error_log("expected an index %s", tag_to_string(index_h.tag)));
                int i_index_h = index_h.number;
                TRY(i_index_l < i_index_h && i_index_h <= list.size, error_log("out of bounds %d is not range of list of size %d", i_index_h, list.size));
                *out = list;
                out->size = i_index_h - i_index_l; // [] '(1 2 3) 1 2 -> .size = 1  
                out->offset += i_index_l;          //                 -> offset+1
                out->list   += i_index_l;          //                 -> ptr + 1
                printf("->> [%d:%d] %d %d\n", i_index_l, i_index_h, out->size, out->offset);
                return true;
            }

            *out = list.list[i_index_l];
            return true;
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
            
            // REFMAYBE
            ctx->in_return = true;
            return false;
        }
        if (List_equal_lit(op, "+"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '+' got %d", li.size));
            List res = {
                .tag = tag_number,
                .number = 0
            };
            for (int i = 1; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to add got %s", tag_to_string(operand.tag)));
    
                res.number += operand.number;
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
            TRY(to_inc->value.tag == tag_number, error_log("try to increment %s", tag_to_string(to_inc->value.tag)));
            to_inc->value.number += 1;
            *out = to_inc->value;
            return true;
        }
        if (List_equal_lit(op, "--"))
        {
            TRY(li.size == 2);
            TRY(li.list[1].tag == tag_symbole, error_log("can only decrement variable got %s", tag_to_string(li.list[1].tag)));
            
            Variable *to_dec = get_Variable(ctx, li.list[1]);
            TRY(to_dec, error_log("variable \"%.*s\" to decrement not found", li.list[1].size, li.list[1].str));
            TRY(to_dec->value.tag == tag_number, error_log("try to decrement %s", tag_to_string(to_dec->value.tag)));
            to_dec->value.number -= 1;
            *out = to_dec->value;
            return true;
        }
        if (List_equal_lit(op, "-"))
        {
            TRY(li.size >= 3, error_log("expected at least 3 elements for '-' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == tag_number);

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to subtruct got %s", tag_to_string(operand.tag)));
                
                res.number -= operand.number;
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "*"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '*' got %d", li.size));
            List res = { 
                .tag = tag_number,
                .number = 1
            };
            for (int i = 1; i < li.size; i++)
            {
                List operand = {0};
    
                TRY(eval(ctx, li.list[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to multiply got %s", tag_to_string(operand.tag)));
                
                res.number *= operand.number;
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "/"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '/' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == tag_number);

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to divide got %s", tag_to_string(operand.tag)));
                
                res.number /= operand.number;
            }
            *out = res;
            return true;
        }
        if (List_equal_lit(op, "//"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '//' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == tag_number);

            for (int i = 2; i < li.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to divide integer got %s", tag_to_string(operand.tag)));
                
                res.number /= operand.number;
                res.number = round(res.number);
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
            TRY(li.size == 3, error_log("expected 2 elements for '<=' got %d", li.size));
            
            List left = {0};
            TRY(eval(ctx, li.list[1], &left));
            TRY(left.tag == tag_number);
            List right = {0};
            TRY(eval(ctx, li.list[2], &right));
            TRY(right.tag == tag_number);

            if (left.number <= right.number)
                *out = TRUE_LIST;
            return true;
        }
        if (List_equal_lit(op, ">="))
        {
            TRY(li.size == 3, error_log("expected 2 elements for '>=' got %d", li.size));
            
            List left = {0};
            TRY(eval(ctx, li.list[1], &left));
            TRY(left.tag == tag_number);
            List right = {0};
            TRY(eval(ctx, li.list[2], &right));
            TRY(right.tag == tag_number);

            if (left.number >= right.number)
                *out = TRUE_LIST;
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
            TRY(out->size > 0,              *out = NIL_LIST; error_log("can't take first element of an empty list"));
            
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

            size_t it_idx = local_Variable(ctx, (Variable){ .name = li.list[1] }); 
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
        /* if (List_equal_lit(op, "$"))
        {
            TRY(li.size >= 3, error_log("expected at least 2 elements for '$' got %d", li.size));
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            TRY(res.tag == tag_number);

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
    case tag_number: return fabs(li1.number - li2.number) < 1.0E-10;
    case tag_list: {
        TRY(li1.size == li2.size);
        for (int i = 0; i < li1.size; i++)
            TRY(List_equal(li1.list[i], li2.list[i]));
    } return true;
    case tag_string:    return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_symbole:   return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_true:      return li2.tag == tag_true;
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

