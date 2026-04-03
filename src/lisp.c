#define STRING_IMPLEMENTATION
#define AR_IMPLEMENTATION
#include "lisp.h"


Strb error = {0};
Lisp_context *g_ctx = NULL;

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

SET_IMPLEMENT_HASH_SET(Variable, VAR_IS_NULL, VAR_SET_NULL, 4, 0.8, 64);



Variable *get_local_Variable(List name)
{   
    if (g_ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&g_ctx->args_stack))
        {
            TRY(it->name.tag == tag_symbole);
            if (List_str_equal(it->name, name))
                return it;
        }
    return NULL;
}

Variable *get_global_Variable(List name)
{   
    TRY(name.tag == tag_symbole);
    return set_Variable_get(&g_ctx->variables, (Variable){ .name = name });
}

/* Variable *get_function(List name)
{   
    assert(name.tag == tag_symbole);
    return set_Variable_get(&g_ctx->functions, (Variable){ .name = name });
} */

Variable *get_Variable(List name)
{
    TRY(name.tag == tag_symbole);
    Variable *res = get_local_Variable(name);
    if (res)
        return res;
    return get_global_Variable(name);
}

/* Variable *get_type(List name)
{
    assert(name.tag == tag_symbole);
    return set_Variable_get(&g_ctx->types, (Variable){ .name = name });
} */

// return index in the call stack
size_t local_Variable(Variable var)
{
    da_Variable *frame = &da_top(&g_ctx->args_stack);
    if (g_ctx->args_stack.size > 0)
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
bool global_Variable(Variable var)
{
    TRY(var.name.tag == tag_symbole);
    // set or replace variable var.name
    Variable *old = set_Variable_emplace(&g_ctx->variables, var);
    if (!VAR_IS_NULL(*old))
        return false;
    *old = var;
    return true;
}

// false on not found
bool mutate_Variable(Variable var)
{
    if (g_ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&g_ctx->args_stack))
            if (List_str_equal(it->name, var.name))
            {
                TRY(is_of_type(var.value, it->type), error_log("mutate value in stack to a value of an unexpected type"));
                it->value = var.value;
                return true;
            }
    
    Variable *old = set_Variable_emplace(&g_ctx->variables, var);
    if (!VAR_IS_NULL(*old))
    {
        TRY(is_of_type(var.value, old->type), error_log("mutate value in stack to a value of an unexpected type"));
        old->value = var.value;
        return true;
    }

    return false;
}


static inline bool prepare_function(da_Variable *new_frame, List *return_type, const List li, const List args_def)
{    
    // push args with their names in stack
    bool have_a_type_hint = true; // the last argument got a type hint -> if new hint -> it's the return type hint
    int arg_position = 1;

    for (int i = 0; i < args_def.size; i++)
    {
        
        List type = NIL_LIST;
        Variable *var_type = NULL;
        if (args_def.list[i].tag == tag_type)
            type = args_def.list[i];
        else if ((var_type = get_Variable(args_def.list[i])) && var_type->value.tag == tag_type)
            type = var_type->value;
        else
        { // normal argument
            TRY(args_def.list[i].tag == tag_symbole);

            have_a_type_hint = false; 
            da_push(new_frame, (Variable){ .name = args_def.list[i], .type = ANY_TYPE });
            TRY(eval(li.list[arg_position++], &da_top(new_frame).value));
            continue;
        }
        
        if (have_a_type_hint) 
        { // if this is an hint and the last argument have already been hinted this as to be the last hint for the return value
            TRY(i + 1 == args_def.size, error_log("two function argument hint not at the end of the argument list"));
            *return_type = type;
            break;
        }
        // normal type hint
        TRY(i > 0, error_log("type decoration goes after a symbole"));
        da_top(new_frame).type = type;
        TRY(is_of_type(da_top(new_frame).value, da_top(new_frame).type), error_log("bad argument type"));
        have_a_type_hint = true;
    }
    return true;
}

// li: (((args_def ...) statements ...) args_call)
// or
// function_def != NULL => li: (name args_call) and function_def: ((args_def ...) statements ...)
bool eval_function(const List li, const List *function_def, List *out)
{
    const List func_def = function_def ? *function_def : li.list[0];
    TRY(func_def.tag == tag_list && func_def.size >= 2, error_log("not a function definition"));

    const List args_def = func_def.list[0];
    TRY(args_def.tag == tag_list, error_log("argument definition is not a list got %s", tag_to_string(args_def.tag)));

    
    List return_type = ANY_TYPE; 
    da_Variable new_frame = {0};
    TRY(prepare_function(&new_frame, &return_type, li, args_def));
    da_push(&g_ctx->args_stack, new_frame);

    bool res = false;
    // execute statements
    for (int i = 1; i+1 < func_def.size; i++)
        if (!eval(func_def.list[i], out) && g_ctx->in_return)
        { // return have been call
            error.size = 0;         // reset error need to find solution for that
            g_ctx->in_return = false; // not in return anymore
            res = true;
            goto end; // terminate
        }
    
    // return the last one
    GOTRY(eval(func_def.list[func_def.size-1], out));
    GOTRY(is_of_type(*out, return_type), error_log("function return unexpected type"));
end:
    res = true;
fail:
    assert(g_ctx->args_stack.size > 0);
    TRY(g_ctx->args_stack.size > 0, error_log("return from root"));
    da_free(&da_top(&g_ctx->args_stack));
    g_ctx->args_stack.size--;
    
    return res;
}






bool eval(const List li, List *out)
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
    case tag_reference: {
        *out = *li.list;
    } return true;

    case tag_symbole: {
        Variable *var = get_Variable(li);
        TRY(var, error_log("no variable nor function named: %.*s", li.size, li.str));
        *out = var->value;
    } return true;
    case tag_list: {

        // nil|false
        if (IS_NIL(li))
        {
            *out = li;
            return true;
        }
        
        if (li.list[0].tag == tag_list) // inline function
        {
            TRY(eval_function(li, &li.list[0], out), error_log("failed to call inline function"));
            return true;
        }

        const List op = *li.list; 
        TRY(op.tag == tag_symbole, error_log("unkown first list element primitive"));
        
        // TODO transform into an prefect hash table
        // uint16_t a = *(uint16_t)&op.str.arr;
        primitive_t primitive = get_Primitive(op);
        if (primitive) return primitive(li, out);
        
        Variable *var = get_Variable(op);
        TRY(var, error_log("no primitive '%.*s' found to evaluate a list", op.size, op.str));
        TRY(eval_function(li, &var->value, out));
    } return true;

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


List List_copy(const List li)
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
            .list = List_alloc(li.size * sizeof(List))
        };

        for (size_t i = 0; i < li.size; i++)
            res.list[i] = List_copy(li.list[i]);
        
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

Lisp_context *Lisp_context_init(List root)
{
    assert(root.tag == tag_list);
    Lisp_context *res = calloc(1, sizeof(*res));
    res->gc = add_to_gc_context((set_void_ptr){0}, root);
    res->root = root;
    da_push_zero(&res->args_stack);

    Lisp_context *old = g_ctx;
    start_body_end (set_Lisp_context(res), set_Lisp_context(old))
    { // buildin types
        add_simple_type("list",    (List){ .tag = tag_type, .type_tag = tag_list, .size = TYPE_UNDEFINED_LIST_SIZE, });
        add_simple_type("symbole", (List){ .tag = tag_type, .type_tag = tag_symbole   });
        add_simple_type("int",     (List){ .tag = tag_type, .type_tag = tag_integer   });
        add_simple_type("float",   (List){ .tag = tag_type, .type_tag = tag_real      });
        add_simple_type("string",  (List){ .tag = tag_type, .type_tag = tag_string    });
        add_simple_type("type",    (List){ .tag = tag_type, .type_tag = tag_type      });
        add_simple_type("any",     (List){ .tag = tag_type, .type_tag = ttag_any_type });
    }
    

    return res;
}

void set_Lisp_context(Lisp_context *ctx)
{
    g_ctx = ctx;
}

void Lisp_context_free(void)
{
    assert(g_ctx->args_stack.size >= 1);

    { // free memory not tracked by gc
        da_for (da_Variable, it, &g_ctx->args_stack)
            da_free(it);
        da_free(&g_ctx->args_stack);
    
        set_Variable_free(&g_ctx->variables);
        g_ctx->root = NIL_LIST;
    }

    // GGGGGGGGGGGGGC!!
    garbage_collector();

    
    set_void_ptr_free(&g_ctx->gc);
}

