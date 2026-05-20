
#ifdef LISP_H

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
static inline bool le_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer <= b.integer;
        else if (b.tag == tag_real)
            return a.integer <= b.real;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real <= b.integer;
        else if (b.tag == tag_real)
            return a.real <= b.real;
    }
    return false;
}
static inline bool ge_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer >= b.integer;
        else if (b.tag == tag_real)
            return a.integer >= b.real;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real >= b.integer;
        else if (b.tag == tag_real)
            return a.real >= b.real;
    }
    return false;
}
static inline bool l_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer < b.integer;
        else if (b.tag == tag_real)
            return a.integer < b.real;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real < b.integer;
        else if (b.tag == tag_real)
            return a.real < b.real;
    }
    return false;
}
static inline bool g_than_List(List a, List b)
{
    if (a.tag == tag_integer)
    {
        if (b.tag == tag_integer)
            return a.integer > b.integer;
        else if (b.tag == tag_real)
            return a.integer > b.real;
    }
    else if (a.tag == tag_real)
    {
        if (b.tag == tag_integer)
            return a.real > b.integer;
        else if (b.tag == tag_real)
            return a.real > b.real;
    }
    return false;
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
static inline List List_sublist(List li, uint16_t stride, uint16_t size)
{
    assert(li.tag == tag_list);
    if (li.size <= stride || size == 0)
        return NIL_LIST;
    return (List){
        .tag = tag_list,
        .offset = li.offset + stride,
        .size = size,
        .list = &li.list[stride]
    };
}
static inline List List_stride(List li, uint16_t stride)
{
    return List_sublist(li, stride, li.size - stride);
}



/* memory and variables */

bool primitive_upgrade(void)
{ // take a local variable and bring it to the previous stack frame
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "upgrade"));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    
    TRY(VM_top1.tag == tag_symbole, error_log("expected a symbole of variable but got %s", tag_to_string(VM_top1.tag)));
    Variable *var = get_local_Variable(VM_top1);
    TRY(var, error_log("variable %.*s not found or not local", var->name.size, var->name.str));
    VM_top1 = NIL_LIST;

    int idx = da_idx_for(var, &g_ctx->stack);
    while (idx > 0)
    {
        SWAP(g_ctx->stack.arr[idx], g_ctx->stack.arr[idx-1]);
        if (g_ctx->stack.arr[idx].value.tag == ttag_frame)
        {
            g_ctx->frame_start++; 
            return true; // swap the marker
        }
        idx--;
    }
    // did not found a marker in the stack
    error_log("already in root stack frame");
    return false;
}

bool primitive_local(void)
{ // create and initilize a local variable -> return is undefined
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "local"));

    if (VM_top1.size == 3)
    { // (local NAME VALUE)
        VM_push(VM_top1.list[1]);
        TRY(eval());
        TRY(VM_top1.tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(VM_top1.list[1].tag)));
        
        VM_push(VM_top2.list[2]);
        TRY(eval());
        
        local_Variable((Variable){
            .name = VM_top2,
            .type = ANY_TYPE,
            .value = VM_top1
        });
        VM_pop; VM_pop;

        VM_top1 = NIL_LIST;
        return true;
    }
    if (VM_top1.size == 4)
    { // (local NAME TYPE VALUE)
        VM_push(VM_top1.list[1]);
        TRY(eval());
        TRY(VM_top1.tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(VM_top1.list[1].tag)));

        VM_push(VM_top2.list[2]);
        TRY(eval());
        TRY(VM_top1.tag == tag_type, error_log("expected a type at position 2 of local got %s", tag_to_string(VM_top1.tag)));
        
        VM_push(VM_top3.list[3]);
        TRY(eval());
        
        TRY(is_of_type(VM_top1, VM_top2), error_log("uncompatible type"));
        local_Variable((Variable){ 
            .name = VM_top3,
            .type = VM_top2,
            .value = VM_top1
        });
        VM_pop; VM_pop; VM_pop;

        VM_top1 = NIL_LIST;
        return true;
    }

    error_log("expected 3 or 4 element for 'local' got %d", VM_top1.size);
    return false;
}

bool primitive_global(void)
{ // create and initilize a global variable
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "global"));
    TRY(VM_top1.size == 3, error_log("expected 3 element for 'global' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());
    TRY(VM_top1.tag == tag_symbole, error_log("expected a symbole to 'global' to got %s", tag_to_string(VM_top1.list[1].tag)));

    VM_push(VM_top2.list[2]);
    TRY(eval());
    
    Variable var = {
        .name = VM_top2, 
        .type = ANY_TYPE,
        .value = VM_top1
    };
    TRY(global_Variable(var), error_log("global variable %.*s already exist", STRV_UNPACK(List_to_Strv(var.name))));
    VM_pop; VM_pop;

    VM_top1 = NIL_LIST;
    return true;
}

bool primitive_assign(void)
{ // change value of a variable
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "="));
    TRY(VM_top1.size == 3, error_log("expected 3 element for '=' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    SWAP(VM_top1, VM_top2);

    VM_top1 = VM_top1.list[2];
    TRY(eval());

    // A <- B;
    // 2[A] 1[B]

    
    if (VM_top2.tag == tag_symbole)
    {
        Variable *var = get_Variable(VM_top2);
        TRY(var, error_log("left is a symbole (%.*s) but there is no variable with this name", VM_top2.size, VM_top2.str));
        var->value = VM_top1;
        // if (var->value.tag == tag_reference)
            // *var->value.list = VM_top1; // need a way to change a reference in a variable
        // else
            // var->value = VM_top1;
            
        TRY(is_of_type(var->value, var->type));
    }
    else if (VM_top2.tag == tag_reference)
    {
        assert(VM_top2.list);

        *VM_top2.list = VM_top1;
    }
    else
    {
        error_log("expected a symbole or a reference to assign to got %s", tag_to_string(VM_top2.tag));
        VM_pop;
        return false;
    }

    VM_top2 = VM_top1;
    VM_pop;
    
    return true;
}


/* controle flow */

bool primitive_exclamation_mark(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "?"));
    TRY(VM_top1.size == 4, error_log("expected 4 element for '?' got %d", VM_top1.size));
    // List cond = {0};
    VM_push(VM_top1.list[1]);
    TRY(eval());
    
    if (!IS_NIL(VM_top1))
    {
        VM_pop;
        VM_top1 = VM_top1.list[2];
        TRY(eval());
    }
    else
    {
        VM_pop;
        VM_top1 = VM_top1.list[3];
        TRY(eval());
    }
    return true;
}

bool primitive_if(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "if"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 element for 'if' got %d", VM_top1.size));
    VM_push(VM_top1.list[1]);
    TRY(eval());
    if (!IS_NIL(VM_top1))
        for (int i = 2; i < VM_top2.size; i++)
        {
            VM_top1 = VM_top2.list[2];
            TRY(eval());
        }
    VM_pop;
    return true;
}

bool primitive_while(void)
{ // return last value of the body and of the last iteration or () if no body
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "while"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for 'while' got %d", VM_top1.size));

    VM_push(NIL_LIST);
    for (;;)
    {
        // the condition can have side effects
        VM_push(VM_top2.list[1]);
        TRY(eval());
        if (IS_NIL(VM_top1))
        {
            VM_pop;
            break;
        }
        VM_pop;

        
        for (int i = 2; i < VM_top2.size; i++)
        {
            VM_top1 = VM_top2.list[i];
            TRY(eval());
        }
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

// (for IT LIST ...BODY)
bool primitive_for(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "for"));
    TRY(VM_top1.size >= 4, error_log("expected 4 elements for 'for' got %d", VM_top1.size));
    TRY(VM_top1.list[1].tag == tag_symbole, error_log("expected a symbole for the iterator name got %s", tag_to_string(VM_top1.list[1].tag)));
    
    // TYPED optionnal
    size_t it_idx = local_Variable((Variable){ 
        .name = VM_top1.list[1],
        .value = NIL_LIST,
        .type = ANY_TYPE
    });

    VM_push(VM_top1.list[2]);
    TRY(eval());
    TRY(VM_top1.tag == tag_list, error_log("expected a list to iterate in for loop got %s", tag_to_string(VM_top1.tag)));


    VM_push(NIL_LIST);
    for (int i = 0; i < VM_top2.size; i++)
    {
        // assert(g_ctx->stack.arr[it_idx].value.tag == tag_reference);
        // assert(g_ctx->stack.arr[it_idx].value.list);
        // *g_ctx->stack.arr[it_idx].value.list = VM_top2.list[i];

        g_ctx->stack.arr[it_idx].value = VM_top2.list[i];
        
        for (int j = 3; j < VM_top3.size; j++)
        {
            VM_top1 = VM_top3.list[j];
            TRY(eval(), error_log("while evaluating for loop body"));
        }
    }
    VM_top3 = VM_top1;
    VM_pop;
    VM_pop;
    return true;
}

bool primitive_return(void)
{
    assert(!g_ctx->in_return && !g_ctx->in_break);
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "return"));
    TRY(VM_top1.size == 1 || VM_top1.size == 2, error_log("expected 2 or 3 elements for 'return' got %d", VM_top1.size));
    if (VM_top1.size == 2)
    {
        VM_top1 = VM_top1.list[1];
        TRY(eval());
    }
    else
        VM_top1 = NIL_LIST;
    
    g_ctx->in_return = true;
    return false; // not a real error
}

bool primitive_break(void)
{
    assert(!g_ctx->in_return && !g_ctx->in_break);
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "break"));
    TRY(VM_top1.size == 1 || VM_top1.size == 2, error_log("expected 2 or 3 elements for 'break' got %d", VM_top1.size));
    if (VM_top1.size == 2)
    {
        VM_top1 = VM_top1.list[1];
        TRY(eval());
    }
    else
        VM_top1 = NIL_LIST;
    
    g_ctx->in_break = true;
    return false; // not a real error
}


/* maths */

bool primitive_plus(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "+"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '+' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        VM_top2 = add_List(VM_top2, VM_top1);
        TRY(!IS_NIL(VM_top2), error_log("couldn't add"));
        VM_pop;
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

bool primitive_increment(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "++"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for '++' got %d", VM_top1.size));

    if (VM_top1.list[1].tag == tag_symbole)
    {
        Variable *to_inc = get_Variable(VM_top1.list[1]);
        TRY(to_inc, error_log("variable \"%.*s\" to increment not found", VM_top1.list[1].size, VM_top1.list[1].str));
        if (to_inc->value.tag == tag_reference)
        {
            assert(to_inc->value.list);
            ++to_inc->value.list->integer;
            VM_top1 = *to_inc->value.list;
            return true;
        }
        TRY(to_inc->value.tag == tag_integer, error_log("try to increment %s", tag_to_string(to_inc->value.tag)));
        to_inc->value.integer += 1;
        VM_top1 = to_inc->value;
        return true;
    }
    VM_push(VM_top1.list[1]);
    TRY(eval());
    if (VM_top1.tag == tag_reference)
    {
        assert(VM_top1.list);
        ++VM_top1.list->integer;
        VM_top2 = *VM_top1.list;
        VM_pop;
        return true;
    }
    VM_pop;
    
    error_log("can only increment variable or reference got %s", tag_to_string(VM_top1.list[1].tag));
    return false;
}

bool primitive_decrement(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "--"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for '==' got %d", VM_top1.size));

    if (VM_top1.list[1].tag == tag_symbole)
    {
        Variable *to_inc = get_Variable(VM_top1.list[1]);
        TRY(to_inc, error_log("variable \"%.*s\" to decrement not found", VM_top1.list[1].size, VM_top1.list[1].str));
        if (to_inc->value.tag == tag_reference)
        {
            assert(to_inc->value.list);
            --to_inc->value.list->integer;
            VM_top1 = *to_inc->value.list;
            return true;
        }
        TRY(to_inc->value.tag == tag_integer, error_log("try to decrement %s", tag_to_string(to_inc->value.tag)));
        to_inc->value.integer -= 1;
        VM_top1 = to_inc->value;
        return true;
    }
    VM_push(VM_top1.list[1]);
    TRY(eval());
    if (VM_top1.tag == tag_reference)
    {
        assert(VM_top1.list);
        --VM_top1.list->integer;
        VM_top2 = *VM_top1.list;
        VM_pop;
        return true;
    }
    VM_pop;
    
    error_log("can only decrement variable got %s", tag_to_string(VM_top1.list[1].tag));
    return false;
}

bool primitive_minus(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "-"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '-' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        VM_top2 = sub_List(VM_top2, VM_top1);
        TRY(!IS_NIL(VM_top2), error_log("couldn't add"));
        VM_pop;
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

bool primitive_product(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "*"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '*' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        VM_top2 = mult_List(VM_top2, VM_top1);
        TRY(!IS_NIL(VM_top2), error_log("couldn't multiply"));
        VM_pop;
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

bool primitive_div(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "/"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '/' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        VM_top2 = div_List(VM_top2, VM_top1);
        TRY(!IS_NIL(VM_top2), error_log("couldn't div"));
        VM_pop;
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

bool primitive_integer_div(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "//"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '//' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        VM_top2 = idiv_List(VM_top2, VM_top1);
        TRY(!IS_NIL(VM_top2), error_log("couldn't idiv"));
        VM_pop;
    }
    VM_top2 = VM_top1;
    VM_pop;
    return true;
}

bool primitive_equal(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "=="));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '==' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        if (!List_equal(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_less_or_equal_than(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "<="));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '<=' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        if (!le_than_List(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_more_or_equal_than(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], ">="));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '>=' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        if (!ge_than_List(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_more_than(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], ">"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '>' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        if (!g_than_List(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_less_than(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "<"));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '<' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());
        
        if (!l_than_List(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_not_equal(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "!="));
    TRY(VM_top1.size >= 3, error_log("expected at least 3 elements for '!=' got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());

    for (int i = 2; i < VM_top2.size; i++)
    {
        VM_push(VM_top2.list[i]);
        TRY(eval());

        if (List_equal(VM_top2, VM_top1))
        {
            VM_pop;
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_pop;
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_not(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "!"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for '!' got %d", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    VM_top1 = IS_NIL(VM_top1) ? TRUE_LIST : NIL_LIST;

    return true;
}

bool primitive_and(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "&&"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for '&&' got %d", VM_top1.size));
    
    for (int i = 1; i < VM_top1.size; i++)
    {
        VM_push(VM_top1.list[i]);
        TRY(eval());
        
        if (IS_NIL(VM_top1))
        {
            VM_pop;
            VM_top1 = NIL_LIST;
            return true;
        }
        VM_pop;
    }
    VM_top1 = TRUE_LIST;
    return true;
}

bool primitive_or(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "||"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for '||' got %d", VM_top1.size));

    for (int i = 1; i < VM_top2.size; i++)
    {
        VM_push(VM_top1.list[i]);
        TRY(eval());
        
        if (!IS_NIL(VM_top1))
        {
            VM_pop;
            VM_top1 = TRUE_LIST;
            return true;
        }
        VM_pop;
    }
    VM_top1 = NIL_LIST;
    return true;
}



/* List */

// (== (parse "1 2 3") (1 2 3))
bool primitive_parses(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "parses"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'parses' got %d", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    
    TRY(VM_top1.tag == tag_string, error_log("expected a string to parse got %s", tag_to_string(VM_top1.tag)));
    TRY(lists(List_to_Strv(VM_top1), &VM_top1));

    return true;
}


bool primitive_list(void)
{ // create a copy of element 1 but with evaluated elements
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "list"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'list' got %d", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    
    TRY(eval());
     
    TRY(VM_top1.tag == tag_list, error_log("expected a list to build"));
    VM_push((List){
        .tag = tag_list,
        .size = VM_top1.size,
        .list = List_alloc(sizeof(List) * (VM_top1.size))
    });
    VM_push(NIL_LIST);
    for (int i = 0; i < VM_top3.size; i++)
    {
        VM_top1 = VM_top3.list[i];
        TRY(eval());
        VM_top2.list[i] = VM_top1;
    }
    VM_top3 = VM_top2;

    VM_pop; VM_pop;
    return true;
}

bool primitive_eval(void)
{ // eval every element of element 1 then return the last element of elements 1 eval.
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "eval"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'eval' got %d", VM_top1.size));

    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_list, error_log("expected a list to eval in the eval primitive got %s", tag_to_string(VM_top1.tag)));

    VM_push(NIL_LIST);
    for (int i = 0; i < VM_top2.size; i++)
    {
        VM_top1 = VM_top2.list[i];
        TRY(eval());
    }
    VM_top2 = VM_top1;

    VM_pop;
    return true;
}

bool primitive_copy(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "copy"));
    TRY(VM_top1.size == 2, error_log("expected 2 argument for 'copy' got %d elements", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    VM_top1 = List_copy(VM_top1);
    return true;
}

bool primitive_array(void)
{ // create an array of size n fill of NIL_LIST
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "array"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'array' got %d", VM_top1.size));

    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_integer);
    
    VM_top1 = (List){
        .tag = tag_list,
        .size = VM_top1.integer,
        .list = List_alloc(sizeof(List) * VM_top1.integer)
    };
    return true;
}

bool primitive_len(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "len"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'len' got %d", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_list 
     || VM_top1.tag == tag_string
     || VM_top1.tag == tag_symbole);
    VM_top1 = (List){
        .integer = VM_top1.size,
        .tag = tag_integer,
    };
    return true;
}

bool primitive_ref_square_bracket(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "&[]"));
    TRY(VM_top1.size == 3 || VM_top1.size == 4, error_log("expected 3 or 4 element for '&[]' got %d", VM_top1.size));
    
    // List list = {0};
    VM_push(VM_top1.list[1]);
    TRY(eval());
    TRY(VM_top1.tag == tag_list, error_log("expected a list to index got %s", tag_to_string(VM_top1.tag)));

    
    // List index_l = {0};
    // int i_index_l = index_l.integer;
    VM_push(VM_top2.list[2]);
    TRY(eval());
    TRY(VM_top1.tag == tag_integer, error_log("expected an index got %s", tag_to_string(VM_top1.tag)));
    TRY(0 <= VM_top1.integer && VM_top1.integer < VM_top2.size, error_log("out of bounds %d is not range of list of size %d", VM_top1.integer, VM_top2.size));

    // List index_h = {0};
    // int i_index_h = index_h.integer;
    if (VM_top3.size == 4)
    {
        VM_push(VM_top3.list[3]);
        TRY(eval());
        TRY(VM_top1.tag == tag_integer, error_log("expected an index got %s", tag_to_string(VM_top1.tag)));
        TRY(VM_top2.integer < VM_top1.integer && VM_top1.integer <= VM_top3.size, error_log("out of bounds %d is not range of list of size %d", VM_top1.integer, VM_top3.size));

        VM_top4 = List_sublist(VM_top3, VM_top2.integer, VM_top1.integer - VM_top2.integer);
        
        VM_pop; VM_pop; VM_pop;
        return true;
    }
    
    VM_top3 = (List){
        .tag = tag_reference,
        .list = &VM_top2.list[VM_top1.integer],
        .offset = VM_top1.integer
    };

    VM_pop; VM_pop;
    return true;
}

bool primitive_square_bracket(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "[]"));
    TRY(VM_top1.size == 3 || VM_top1.size == 4, error_log("expected 3 or 4 element for '[]' got %d", VM_top1.size));
    
    VM_push(VM_top1.list[1]);
    TRY(eval());
    TRY(VM_top1.tag == tag_list, error_log("expected a list to index got %s", tag_to_string(VM_top1.tag)));

    VM_push(VM_top2.list[2]);
    TRY(eval());
    TRY(VM_top1.tag == tag_integer, error_log("expected an index got %s", tag_to_string(VM_top1.tag)));
    TRY(0 <= VM_top1.integer && VM_top1.integer < VM_top2.size, error_log("out of bounds %d is not range of list of size %d", VM_top1.integer, VM_top2.size));

    if (VM_top3.size == 4)
    {
        VM_push(VM_top3.list[3]);
        TRY(eval());
        TRY(VM_top1.tag == tag_integer, error_log("expected an index got %s", tag_to_string(VM_top1.tag)));
        TRY(VM_top2.integer <= VM_top1.integer && VM_top1.integer <= VM_top3.size, error_log("out of bounds %d is not range of list of size %d", VM_top1.integer, VM_top3.size));

        VM_top4 = List_sublist(VM_top3, VM_top2.integer, VM_top1.integer - VM_top2.integer);
        
        VM_pop; VM_pop; VM_pop;
        return true;
    }
    
    VM_top3 = VM_top2.list[VM_top1.integer];

    VM_pop; VM_pop;
    return true;
}



/* Reference */

bool primitive_reference(void)
{ // return a reference to a new memory with the element parameter as the value 
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "reference"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'reference' got %d", VM_top1.size));

    VM_top1 = (List){
        .tag = tag_reference,
        .list = List_duplicate(&VM_top1.list[1], sizeof(VM_top1.list[1]))
    };
    
    return true;
}

bool primitive_dereference(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "dereference"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'dereference' got %d", VM_top1.size));
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_reference, error_log("exprected a tag_reference but got %s", tag_to_string(VM_top1.tag)));
    VM_top1 = *VM_top1.list;

    return true;
}


/* Others */

bool primitive_print(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "print"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for 'print' got %d", VM_top1.size));



    VM_push(NIL_LIST);
    for (int i = 1; i < VM_top2.size; i++)
    {
        VM_top1 = VM_top2.list[i];
        TRY(eval(), error_log("failed to eval to 'print'"));
        TRY(List_print(VM_top1), error_log("failed to print"));
    }
    VM_pop;
    VM_top1 = NIL_LIST;
    return true;
}

bool primitive_quote(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "quote"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'quote' got %d", VM_top1.size));

    VM_top1 = VM_top1.list[1];
    return true;
}

bool primitive_dollar(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "$"));
    TODO("$");

    // ($ 1 + 1) -> (+ 1 1) -> 2
    // ($ 1 print 1) -> (print 1 1) -> stdout: "11"
    // the temp list can be allocated in the vm_stack
    // or
    
    return true;
}
bool primitive_math(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "@"));
    TODO("@");

    // (@ 1 + 2 * 3) -> (+ 1 (* 2 3)) -> 7
    // (@ 1 * (2 + 1)) -> (* 1 (+ 2 1))
    // (@ 1 + 2 + 1) -> (+ 1 2 1)
    // harder, but more useful

    return true;
}

bool primitive_multi(void)
{ // eval multiple time value
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "multi"));
    TRY(VM_top1.size == 3, error_log("expected 3 elements (multi COUNT TO_EVAL) got %d", VM_top1.size));

    VM_push(VM_top1.list[1]);
    TRY(eval());
    TRY(VM_top1.tag == tag_integer, error_log("expected an integer for multi got %s", tag_to_string(VM_top1.tag)));

    VM_push(VM_top2.list[2]);
    for (int i = 0; i < VM_top2.integer; i++)
        TRY(eval(), error_log("failed multi iteration %d/%d", i, VM_top2.integer));

    VM_top3 = VM_top1;
    VM_pop; VM_pop;

    return true;
}

bool primitive_garbage(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "garbage"));
    TRY(VM_top1.size == 1, error_log("expected 1 elements (garbage) got %d", VM_top1.size));

    if (!g_ctx->euristics.paused)
        trigger_gc();

    return true;
}


/* String */

bool primitive_format(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "format"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for 'format' got %d", VM_top1.size));

    static Strb acc = {0};
    acc.size = 0;

    for (int i = 1; i < VM_top1.size; i++)
    {
        // List tmp = {0};
        VM_push(VM_top1.list[i]);
        TRY(eval());
        TRY(dump(&acc, VM_top1));
        VM_pop;
    }
    
    VM_top1 = (List){
        .tag = tag_string,
        .size = acc.size,
        .str = List_duplicate(acc.arr, acc.size)
    };
    return true;
}


static inline bool string_symbole_like(const List str)
{
    TRY(VM_top1.tag == tag_string, error_log("expected a string got %s", tag_to_string(VM_top1.tag)));
    for (int i = 0; i < str.size; i++)
    {
        TRY(!isspace(str.str[i]), error_log("symbole can't contains white space"));
    }
    return true;
}

bool primitive_symbole(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "symbole"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for 'symbole' got %d", VM_top1.size));
    TRY(string_symbole_like(VM_top1));
    VM_top1.tag = tag_symbole;
    
    return true;
}

bool primitive_string(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "string"));
    TRY(VM_top1.size >= 2, error_log("expected at least 2 elements for 'string' got %d", VM_top1.size));
    TRY(VM_top1.tag == tag_symbole, error_log("expected a symbole got %s", tag_to_string(VM_top1.tag)));
    VM_top1.tag = tag_string;
    
    return true;
}



/* Types */

bool primitive_typeof(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "typeof"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'typeof' got %d", VM_top1.size));

    VM_top1 = VM_top1.list[1];
    TRY(eval());
    VM_top1 = typeof_List(VM_top1);

    return true;
}


// take an offseted from primitive_type
bool type(void)
{
    TRY(VM_top1.size > 0, error_log("empty type"));

    // simple type
    if (VM_top1.size == 1)
    {
        VM_top1 = VM_top1.list[0];
        TRY(eval());

        // directly a type
        if (VM_top1.tag != tag_type)
        {
            // or infer it
            
            if (VM_top1.tag == tag_list)
            {
                // () -> n = 0 tuple
                // (1) -> (int)
                // 
                VM_push((List){
                    .tag = tag_type,
                    .type_tag = ttag_tuple_type,
                    .size = VM_top1.size,
                    .list = List_alloc(sizeof(List) * VM_top1.size)
                });

                TODO("");
                return true;
            }
            VM_top1 = (List){
                .tag = tag_type,
                .type_tag = VM_top1.tag,
            };
        }
        return true;
    }

    if (List_equal_lit(VM_top1.list[1], "|"))
    {
        return true;
    }
    
    // tuple type 

    return true;
}

// (type 1 | 1.0) -> tuple 1 element of int or float
// (type 1 1.0) -> tuple 2 elements: int, float
// (type type) -> typle 1 elements: type (meta)
bool primitive_type(void)
{ // make a type out of a list
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "type"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'type' got %d", VM_top1.size));
    VM_top1 = VM_top1.list[1];

    


    // for (int i = 1; i < VM_top2.size; i++)
    // {
    //     if (i + 1 < VM_top2.size)
    //     {
    //         TRY(List_equal_lit(VM_top2.list[i+1], "|"), error_log("invalid type desc expected a '|' for a union type"));
    //         TRY(i+2 < VM_top2.size, error_log("invalid type desc expected another type after '|'"));
    //         ;
    //         continue;
    //     }
    // }
    // if (VM_top1.type_tag == tag_list)
    // {
    //     VM_top1.size = VM_top2.list[1].size,
    //     VM_top1.list = List_alloc(sizeof(List) * VM_top1.size);
    //     VM_push(NIL_LIST);
    //     for (int i = 0; i < VM_top3.list[1].size; i++)
    //     {
    //         VM_top1 = VM_top3.list[1].list[i];
    //         TRY(primitive_type());
    //         VM_top2.list[i] = VM_top1;
    //     }
    //     VM_pop;
    // }
    // VM_top2 = VM_top1;
    // VM_pop;
    
    return true;
}


/* Files */

// (read_all "file.txt")
bool primitive_read_all(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "read_all"));
    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'read_all' got %d", VM_top1.size));


    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_string);
    
    char file_name[512];
    TRY(VM_top1.size+1 < 512, error_log("file name too long"));
    strncpy(file_name, VM_top1.str, MIN(sizeof(file_name), VM_top1.size));
    file_name[VM_top1.size] = '\0';

    errno = 0;
    Strb str = {0};
    TRY(Str_error_no_error == Strb_cat_file(&str, file_name), Strb_free(str); error_log("can't read file \"%s\": %s", file_name, strerror(errno)));
    TRY(str.size + 1 < UINT16_MAX, Strb_free(str));

    VM_top1 = (List){
        .tag = tag_string,
        .size = str.size,
        .str = List_delc_alloc(str.arr, str.size + 1)
    };

    return true;
}

// (import "file.txt")
bool primitive_import(void)
{
    EVAL_LIST_ASSERT(List_equal_lit(VM_top1.list[0], "import"));

    TRY(VM_top1.size == 2, error_log("expected 2 elements for 'import' got %d", VM_top1.size));
    
    VM_top1 = VM_top1.list[1];
    TRY(eval());
    TRY(VM_top1.tag == tag_string, error_log("expected a string as file name to import got %s", tag_to_string(VM_top1.tag)));
    

    Strb str = {0}; { // set VM_TOP1 a string of the file (str hold the memory)
        
        bool found = false;
        char file_name[512];
        TRY(VM_top1.size + 1u < sizeof(file_name), error_log("file name too long"));

        da_for (List, path, &g_ctx->paths)
        {
            size_t sb_size = 0;
            if (sb_size + path->size + 1u >= sizeof(file_name)) continue;
            strncpy(&file_name[sb_size], path->str, path->size);
            sb_size += path->size;

            if (sb_size + path->size + 1u >= sizeof(file_name)) continue;
            strncpy(&file_name[sb_size], VM_top1.str, VM_top1.size);
            sb_size += VM_top1.size;

            file_name[sb_size] = '\0';

            str.size = 0;
            errno = 0;
            if (Strb_cat_file(&str, file_name) == Str_error_no_error)
            {
                found = true;
                TRY(str.size + 1 < UINT16_MAX, Strb_free(str));
                break;
            }
        }
        TRY(found, Strb_free(str); error_log("can't read file \"%sv\": %s", &List_to_Strv(VM_top1), strerror(errno)));
        
        
        VM_top1 = (List){
            .tag = tag_string,
            .size = str.size,
            .str = str.arr
        };
    }

    TRY(lists(List_to_Strv(VM_top1), &VM_top1), Strb_free(str));
    Strb_free(str);
    assert(VM_top1.tag == tag_list);
    VM_push(NIL_LIST);
    for (int i = 0; i < VM_top2.size; i++)
    {
        VM_top1 = VM_top2.list[i];
        TRY(eval());
    }

    VM_top2 = VM_top1;
    VM_pop;
    return true;
}



static const Primitive keys[] = {
    { .name = _cstr_to_List("local"),       .fun = primitive_local              },
    { .name = _cstr_to_List("upgrade"),     .fun = primitive_upgrade            },
    { .name = _cstr_to_List("global"),      .fun = primitive_global             },
    { .name = _cstr_to_List("="),           .fun = primitive_assign             },
    { .name = _cstr_to_List("[]"),          .fun = primitive_square_bracket     },
    { .name = _cstr_to_List("&[]"),         .fun = primitive_ref_square_bracket },
    { .name = _cstr_to_List("copy"),        .fun = primitive_copy               },
    { .name = _cstr_to_List("?"),           .fun = primitive_exclamation_mark   },
    { .name = _cstr_to_List("if"),          .fun = primitive_if                 },
    { .name = _cstr_to_List("while"),       .fun = primitive_while              },
    { .name = _cstr_to_List("return"),      .fun = primitive_return             },
    { .name = _cstr_to_List("break"),       .fun = primitive_break              },
    { .name = _cstr_to_List("print"),       .fun = primitive_print              },
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
    { .name = _cstr_to_List("for"),         .fun = primitive_for                },
    { .name = _cstr_to_List("format"),      .fun = primitive_format             },
    { .name = _cstr_to_List("garbage"),     .fun = primitive_garbage            },
    { .name = _cstr_to_List("quote"),       .fun = primitive_quote              },
    { .name = _cstr_to_List("typeof"),      .fun = primitive_typeof             },
    { .name = _cstr_to_List("eval"),        .fun = primitive_eval               },
    { .name = _cstr_to_List("type"),        .fun = primitive_type               },
    { .name = _cstr_to_List("len"),         .fun = primitive_len                },
    { .name = _cstr_to_List("reference"),   .fun = primitive_reference          },
    { .name = _cstr_to_List("dereference"), .fun = primitive_dereference        },
    { .name = _cstr_to_List("list"),        .fun = primitive_list               },
    { .name = _cstr_to_List("array"),       .fun = primitive_array              },
    { .name = _cstr_to_List("multi"),       .fun = primitive_multi              },
    { .name = _cstr_to_List("read_all"),    .fun = primitive_read_all           },
    { .name = _cstr_to_List("symbole"),     .fun = primitive_symbole            },
    { .name = _cstr_to_List("string"),      .fun = primitive_string             },
    { .name = _cstr_to_List("parses"),      .fun = primitive_parses             },
    { .name = _cstr_to_List("import"),      .fun = primitive_import             },
    
};

#endif /* LISP_H */
