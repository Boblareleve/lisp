#define STRING_IMPLEMENTATION
#define AR_IMPLEMENTATION
#include "lisp.h"


Strb static_error = {0};
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
    for (int i = g_ctx->stack.size - 1; i >= g_ctx->frame_start; i--)
    {
        TRY(g_ctx->stack.arr[i].name.tag == tag_symbole);
        if (List_str_equal(g_ctx->stack.arr[i].name, name))
            return &g_ctx->stack.arr[i];
    }
    return NULL;
}
Variable *get_global_Variable(List name)
{   
    TRY(name.tag == tag_symbole);
    return set_Variable_get(&g_ctx->variables, (Variable){ .name = name });
}

Variable *get_Variable(List name)
{
    TRY(name.tag == tag_symbole);
    Variable *res = get_local_Variable(name);
    if (res)
        return res;
    return get_global_Variable(name);
}

// return index in the call stack
size_t local_Variable(Variable var)
{
    for (int i = g_ctx->stack.size-1; i >= g_ctx->frame_start; i--)
    {
        if (List_str_equal(g_ctx->stack.arr[i].name, var.name))
        {
            g_ctx->stack.arr[i] = var;
            return i;
        }
    }
    da_push(&g_ctx->stack, var);
    return g_ctx->stack.size-1;
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

bool push_stack_frame(void)
{
    assert(g_ctx);

    da_push(&g_ctx->stack, (Variable){
        .name = _cstr_to_List_symbole(""),
        .value = { .tag = ttag_frame, .integer = g_ctx->frame_start },
        .type = ANY_TYPE
    });
    g_ctx->frame_start = g_ctx->stack.size;
    return true;
}

// push a marker to pop to (do linear search as it can be mouved)
bool pop_stack_frame(void)
{
    assert(g_ctx);

    TRY(g_ctx->frame_start > 0, error_log("try to return from root stack frame"));
    g_ctx->stack.size = g_ctx->frame_start-1;
    assert(g_ctx->stack.arr[g_ctx->stack.size].value.tag == ttag_frame);
    g_ctx->frame_start = g_ctx->stack.arr[g_ctx->stack.size].value.integer;

    // while (g_ctx->stack.size > 0 && da_top(&g_ctx->stack).value.tag != ttag_frame)
    //     g_ctx->stack.size--;
    // if (g_ctx->stack.size > 0)
    //     g_ctx->stack.size--;
    return true;
}


// 2[(_, ...call_arguments)] 1[((...call_arguments_definition) ...function_body)]
bool eval_function(void)
{
    TRY(g_ctx->vm_stack.size >= 2, error_log("expected two vm args to eval a function"));
    TRY(VM_top1.tag == tag_list, error_log("function definition not a list"));
    TRY(VM_top1.size >= 2, error_log("function definition too short expected at least the aguments then one statement"));
    TRY(have_function_arguments_shape(VM_top1.list[0]), error_log("try to call a list that didn't match a function shape"));
    TRY(VM_top2.size >= 1, error_log("expected anonyme for function call"));
    
    
    List return_type = ANY_TYPE;
    
    VM_push(NIL_LIST); { // parse and check arguments
        #define EF_VM_arg_call(i) (VM_top3.list[i+1])
        #define EF_VM_arg_call_count (VM_top3.size - 1)
        #define EF_VM_arg_def(i) (VM_top2.list[0].list[i])
        #define EF_VM_arg_def_count (VM_top2.list[0].size)

        static da_Variable args = {0};
        int args_point = args.size;
        // args.size = 0;
        
        bool last_argument_have_hint = true;
        for (int i = 0; i < EF_VM_arg_def_count; i++)
        {
            Variable *s = get_global_Variable(EF_VM_arg_def(i));
            if (s && s->type.tag == tag_type && s->type.type_tag == tag_type)
            {
                if (last_argument_have_hint)
                { // function type
                    TRY(i+1 == EF_VM_arg_def_count, error_log("two type not at the end")); // TODO '|' || (VM_top2.list[i+2].tag == tag_symbole && ))
                    TRY(args.size - args_point == EF_VM_arg_call_count);
                    return_type = s->value;
                    continue;
                }
                da_top(&args).type = s->value;
                TRY(is_of_type(da_top(&args).value, da_top(&args).type), error_log("argument %d did not match it's type hint", args.size - args_point - 1));
                last_argument_have_hint = true;
                continue;
            }
            
            TRY(args.size - args_point < EF_VM_arg_call_count, error_log("not enough argument provided for function call"));
            VM_top1 = EF_VM_arg_call(args.size - args_point);
            if (EF_VM_arg_def(i).quote_count == 0)
                TRY(eval());
            
            da_push(&args, (Variable){
                .name = EF_VM_arg_def(i),
                .type = ANY_TYPE,
                .value = VM_top1
            });
            if (EF_VM_arg_def(i).quote_count != 0)
                da_top(&args).name.quote_count--;
            
            last_argument_have_hint = false;
        }
        
        push_stack_frame();
        for (int i = args_point; i < args.size; i++)
            da_push(&g_ctx->stack, args.arr[i]);
        args.size = args_point;
    } VM_pop;


    const int vm_stack_sp = g_ctx->vm_stack.size;

    // execute statements
    for (int i = 1; i+1 < VM_top1.size; i++)
    {
        VM_push(VM_top1.list[i]);
        if (!eval())
        {
            if (g_ctx->in_return) // return have been call
            {
                reset_error(); // need to find solution for that
                g_ctx->in_return = false; // not in return anymore
                TRY(pop_stack_frame());
                
                // pop vm_stack frame
                g_ctx->vm_stack.arr[vm_stack_sp-2] = VM_top1;
                g_ctx->vm_stack.size = vm_stack_sp-1;
                return true; // terminate
            }
            return false; // true error
        }
        VM_pop;
    }
    
    // return the last one
    VM_top2 = VM_top1.list[VM_top1.size-1]; // ¿return?
    VM_pop;
    TRY(eval(), pop_stack_frame());
    TRY(is_of_type(VM_top1, return_type), pop_stack_frame(); error_log("function return unexpected type"));
    
    TRY(pop_stack_frame());
    return true;
}

bool eval(void)
{
    // dec ref count
    if (VM_top1.quote_count > 0)
    {
        VM_top1.quote_count--;
        return true;
    }
    if (VM_top1.tag == tag_list)
    {
        // nil|false
        if (IS_NIL(VM_top1))
            return true;
        
        if (VM_top1.list[0].tag == tag_list) // inline function
        {
            VM_push(VM_top1.list[0]);
            TRY(eval());
            TRY(eval_function(), error_log("failed to call inline function"));
            return true;
        }
        
        TRY(VM_top1.list[0].tag == tag_symbole, error_log("unkown first list element primitive"));
        
        primitive_t primitive = get_Primitive(VM_top1.list[0]);
        if (primitive) return primitive();


        Variable *var = get_Variable(VM_top1.list[0]);
        TRY(var, error_log("no primitive '%.*s' found to evaluate a list", VM_top1.list->size, VM_top1.list->str));
        VM_push(var->value);
        TRY(eval_function());
        return true;
    }
    if (VM_top1.tag == tag_symbole)
    {
        Variable *var = get_Variable(VM_top1);
        TRY(var, error_log("no variable nor function named: %.*s", VM_top1.size, VM_top1.str));
        VM_top1 = var->value;
        return true;
    }
    if (VM_top1.tag == tag_reference)
    { // auto dereference
        VM_top1 = *VM_top1.list;
        TRY(eval());
        return true;
    }
    if (is_list_self_evaluating(VM_top1.tag))
    { // self-evaluating
        return true;
    }
    
    UNREACHABLE("eval switch");
    return false;
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
void List_free(List li)
{    
    if (li.tag == tag_list)
    {
        for (size_t i = 0; i < li.size; i++)
            List_free(li.list[i]);
    }
    
    assert(!IS_NIL(li) || li.list == NULL);
    free(List_get_ptr(li));
}


set_void_ptr add_to_gc_context(set_void_ptr gc, List root)
{
    if (List_get_ptr(root))
        set_void_ptr_insert(&gc, List_get_ptr(root));

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
    // res->stack_allocation_allowed = true;

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
    assert(g_ctx);
    // if (!g_ctx) return;
    { // free memory not tracked by gc
        da_free(&g_ctx->stack);
        da_free(&g_ctx->vm_stack);
        
        set_Variable_free(&g_ctx->variables);
        g_ctx->root = NIL_LIST;
    }

    // GGGGGGGGGGGGGC!!
    garbage_collector();

    set_void_ptr_free(&g_ctx->gc);
    Strb_free(g_ctx->error);
    free(g_ctx);

    g_ctx = NULL;
}

