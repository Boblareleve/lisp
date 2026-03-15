

#define STRING_IMPLEMENTATION
#define AR_IMPLEMENTATION
#include "lisp.h"

#include "rc.c"


Strb error = {0};

#define VAR_IS_NULL(var)  ((var).name.arr == NULL)
#define VAR_SET_NULL(var) ((var).name.arr = NULL)

uint64_t set_Variable_hash(const Variable value, uint64_t seed)
{
    return Strv_hash(value.name, seed);
}
int set_Variable_equal(const Variable v1, const Variable v2)
{
    return Strv_equal(v1.name, v2.name);
}

SET_IMPLEMENT_HASH_SET(Variable, VAR_IS_NULL, VAR_SET_NULL, 4, 0.8, 4);


const char *tag_to_string(int tag)
{
    static const char *table[] = {
        [tag_true]      = "tag_true",
        [tag_symbole]   = "tag_symbole",
        [tag_string]    = "tag_string",
        [tag_number]    = "tag_number",
        [tag_list]      = "tag_list",
    };
    return table[tag];
}


#define GOTRY_consume(str) GOTRY(consume(str), error_log("unexpected EOF"))
bool consume(Strv *str)
{
    Strv_inc(str);
    return Strv_first(*str) != '\0';
}
void skip_space(Strv *str)
{
    while (isspace(Strv_first(*str)))
        Strv_inc(str);
}
void skip_comment(Strv *str)
{
    skip_space(str);

    while (Strv_first(*str) == ';')
    {
        Strv_c_substr(str, '\n');
        skip_space(str);
    }
}


int is_end(int c)     { return c == '(' || c == ')' || isspace(c); }
int not_is_end(int c) { return !is_end(c); }



bool is_unary_sign(char c)
{
    return c == '+' || c == '-';
}

// (()(()))
// return char after closing
// (Strv){0} on error
Strv get_pair(Strv str)
{
    Strv res = {0};
    GOTRY(Strv_first(str) == '(');

    GOTRY_consume(&str);
    int depth = 1;
    while (depth > 0)
    {
        if (Strv_first(str) == '(')
            depth++;
        else if (Strv_first(str) == ')')
            depth--;
        else if (Strv_first(str) == '"')
        { // skip string
            do {
                // skip escaped
                if (Strv_first(str) == '\\')
                    GOTRY_consume(&str);
                // advance
                GOTRY_consume(&str);
            } while (Strv_first(str) != '"');
        }
    
        GOTRY_consume(&str);
    }

    res = str;
fail:
    return res;
}

bool skip_parent(Strv *str)
{
    *str = get_pair(*str);
    return str->arr != NULL;
}

bool skip_atom(Strv *str)
{
    while (str->size > 0 && !is_end(Strv_first(*str)))
        Strv_inc(str);
    return true;
}



ssize_t list_count(Strv str)
{    
    GOTRY(Strv_first(str) == '(');
    GOTRY_consume(&str);
    skip_comment(&str);
    GOTRY(str.size > 0);


    ssize_t count = 0;
    while (Strv_first(str) != ')')
    {
        skip_comment(&str);
        GOTRY(str.size > 0);

        if (Strv_first(str) == '(')
        {
            GOTRY(skip_parent(&str));
            skip_comment(&str);
            GOTRY(str.size > 0);
            count++;
            continue;
        }
        if (Strv_first(str) == '"')  // string with escape character
        {
            do {
                GOTRY_consume(&str);
                if (Strv_first(str) == '\\')
                {
                    GOTRY_consume(&str);
                    GOTRY_consume(&str);
                }
            }
            while (str.size > 0 && Strv_first(str) != '"');
            GOTRY_consume(&str);
            skip_comment(&str);
            GOTRY(str.size > 0);
            count++;
            continue;
        }
        if (Strv_first(str) == '\'') // reference
        {
            do GOTRY_consume(&str); while (Strv_first(str) == '\'');
            continue;
            // skip_comment(&str);
            // GOTRY(str.size > 0, error_log("expected atom after reference (') got EOF"));
        }


        GOTRY(skip_atom(&str));
        skip_comment(&str);
        GOTRY(str.size > 0);
        count++;
    }

    return count;
fail:
    return -1;
}

void test_list_count(void)
{
    assert(list_count(Strv_lit("()")) == 0);
    assert(list_count(Strv_lit("(a)")) == 1);
    assert(list_count(Strv_lit("(a aa)")) == 2);
    assert(list_count(Strv_lit("(  dqfs  d dd)")) == 3);
    assert(list_count(Strv_lit("((dq dd dd) a)")) == 2);
    assert(list_count(Strv_lit("((dq \"(\" dd) a)")) == 2);
    assert(list_count(Strv_lit("(('dq '''a) a '(a bdd (aad d)))")) == 3);
    
}


List escaping(Ar *arena, Strv str)
{
    Ar_save_point sp = Ar_save(arena);
    Strv res = Strv_make(Ar_alloc_align(arena, str.size * 2, sizeof(char)), 0);

    for (int i = 0; i < str.size; i++)
    {
        if (str.arr[i] == '\\')
        {
            i++;
            assert(i < str.size);
            switch (str.arr[i])
            {
            case '\\': res.arr[res.size++] = '\\'; break;
            case 'n':  res.arr[res.size++] = '\n'; break;
            case 't':  res.arr[res.size++] = '\t'; break;
            case 'r':  res.arr[res.size++] = '\r'; break;
            case 'v':  res.arr[res.size++] = '\v'; break;
            case 'a':  res.arr[res.size++] = '\a'; break;
            case 'b':  res.arr[res.size++] = '\b'; break;
            case 'f':  res.arr[res.size++] = '\f'; break;
            default:   res.arr[res.size++] = '?';  break; // unkown
            }
            continue;
        }
        res.arr[res.size++] = str.arr[i];
    }

    List result = {
        .tag = tag_string,
        .size = res.size,
        .str = memcpy(Rc_container_make(res.size, sizeof(char)), res.arr, res.size)
    };
    Ar_restore(arena, sp);
    return result;
}



bool list(Ar *arena, Strv *str, List *li)
{
    TRY(li, error_log("no output list to parse"));
    TRY(Strv_first(*str) != ')', error_log("closing parent at root"));
    TRY(str->size > 0, error_log("empty input"));
    
    skip_space(str);
    skip_comment(str);

    if (Strv_first(*str) == '(')
    {
        li->tag = tag_list;
        ssize_t count = list_count(*str);
        TRY(count != -1, error_log("count error"));
        
        TRY(consume(str), error_log("EOF"));
        skip_comment(str);
        TRY(str->size > 0, error_log("EOF"));
        
        if (count == 0)
        {
            *li = NIL_LIST;
            Strv_inc(str); // can't be the end of file
            return true;
        }

        const Strv save = *str;
        
        // li->list.arr = Ar_alloc(arena, count * sizeof(List));
        // li->list.arr = malloc(count * sizeof(List));
        li->list = Rc_container_make(count, sizeof(List));
        li->size = 0;
        do {
            li->list[li->size] = NIL_LIST;
            TRY(list(arena, str, &li->list[li->size]));
            li->size++;
            skip_comment(str);
        } while (li->size < count && str->size > 0 && Strv_first(*str) != ')');
        TRY(li->size == count, error_log("invalid list element count, expected %d got %d with: %sv", count, li->size, &save));
        TRY(Strv_first(*str) == ')', error_log("unexpected EOF or underestimate list_count() counted %d but there is more", count));
        Strv_inc(str);
        return true;
    }
    if (isdigit(Strv_first(*str)) || (is_unary_sign(Strv_first(*str)) && isdigit(Strv_char_at(*str, 1))))
    {
        char *it = str->arr;
        char *end = &str->arr[str->size];

        *li = (List){
            .tag = tag_number,
            .number = strtod(str->arr, &it)
            // .number = strtoll(str->arr, &it, 10)
        };
        if (it != str->arr)
        {
            *str = Strv_range(it, end); 
            return true;
        }
    }
    if (Strv_first(*str) == '"')
    {
        TRY(consume(str));
        char *begin = str->arr;
        while (Strv_first(*str) != '"')
        {
            TRY(consume(str));
            if (Strv_first(*str) == '\\')
            {
                TRY(consume(str));
                TRY(consume(str));
            }
        }

        *li = escaping(arena, Strv_range(begin, str->arr));
        Strv_inc(str);
        return true;
    }
    if (Strv_first(*str) == '\'')
    {
        // count ref depth
        uint32_t count = 1;

        do TRY(consume(str)); while (Strv_first(*str) == '\'');

        TRY(list(arena, str, li));
        li->quote_count = count;
        
        return true;
    }
    
    // any symbole (yes can be any character)
    Strv symbole = Strv_fun_substr(str, not_is_end); // maybe put that in arena
    *li = (List){
        .tag = tag_symbole,
        .str = symbole.arr,
        .size = symbole.size
    };

    // special case true
    if (List_str_equal(*li, "t"))
        *li = (List){ .tag = tag_true };
    
    return true;
}

bool _dump_indent(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_list: {
        if (li.size == 0)
        {
            Strb_cat(out, "()");
            break;
        }
        Strb_catf(out, "(\n", li.size);
        for (size_t i = 0; i < li.size; i++)
        {
            TRY(_dump_indent(out, li.list[i], indent + 2));
            Strb_cat(out, "\n");
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");
    } break;
    case tag_number: {
        if (fmod(li.number, 1.0) == 0.0)
            Strb_catf(out, "%.0f64", li.number);
        else
            Strb_catf(out, "%f64", li.number);
    } break;
    case tag_symbole:   Strb_catf(out, "%.*s", li.size, li.str); break;
    case tag_string:    Strb_catf(out, "%.*s", li.size, li.str); break;
    case tag_true:      Strb_cat(out, "true");                   break;
    default:            Strb_cat(out, "UNKOWN");                 break;
    }
    return true;
}
bool _dump_type_indent(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_symbole: {
        Strb_catf(out, "symbole: '%.*s'", li.size, li.str);
    } break;
    case tag_list: {
        if (li.size == 0)
        {
            Strb_cat(out, "()");
            break;
        }
        Strb_catf(out, "(list {%d}:\n", li.size);
        for (size_t i = 0; i < li.size; i++) // const List, it, &li.list)
        {
            TRY(_dump_type_indent(out, li.list[i], indent + 2));
            Strb_cat(out, "\n");
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");
    } break;
    case tag_string: {
        Strb_catf(out, "string: \"%.*s\"", li.size, li.str);
    } break;
    case tag_number: {
        if (fmod(li.number, 1.0) == 0.0)
            Strb_catf(out, "number: %.0f64", li.number);
        else
            Strb_catf(out, "number: %f64", li.number);
    } break;
    case tag_true: {
        Strb_cat(out, "true");
    } break;
    default: Strb_cat(out, "UNKOWN"); break;
    }
    return true;
}

bool dump(Strb *out, const List li)
{
    TRY(out, error_log("no output Strb"));

    switch (li.tag)
    {
    case tag_list: {    
        Strb_cat(out, "(");
        for (size_t i = 0; i < li.size; i++)
        {
            TRY(dump(out, li.list[i]));
            if (i+1 == li.size)
                Strb_cat_char(out, ' ');
        }
        Strb_cat(out, ")");
    } break;
    case tag_number: {
        if (fmod(li.number, 1.0) == 0.0)
            Strb_catf(out, "%.0f64", li.number);
        else
            Strb_catf(out, "%f64", li.number);
    } break;
    case tag_symbole:   Strb_catf(out, "%.*s", li.size, li.str);  break;
    case tag_string:    Strb_catf(out, "%.*s", li.size, li.str);  break;
    case tag_true:      Strb_cat(out, "true");                    break;
    default:            Strb_cat(out, "UNKOWN");                  break;
    }
    return true;
}
bool dump_indent(Strb *out, const List li)
{
    return _dump_indent(out, li, 0);
}
bool List_print(const List li)
{
    static Strb to_print = {0};
    to_print.size = 0;

    TRY(dump_indent(&to_print, li));
    printf(STRV_FMT, STRV_UNPACK(to_print.view));
    return true;
}

// name: variable with .name set
Variable *get_variable_in_stack(Lisp_context *ctx, Variable name)
{   
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&ctx->args_stack))
            if (Strv_equal(it->name, name.name))
                return it;
    return NULL;
}


// return true if it remplace an exsiting variable false if it push 
Variable *set_stack_Variable(Lisp_context *ctx, Variable var)
{
    assert(ctx->args_stack.size > 0);

    Rc_inc_List(var.value);

    da_Variable *frame = &da_top(&ctx->args_stack);

    da_for (Variable, it, frame)
        if (Strv_equal(it->name, var.name))
        {
            Rc_dec_List(it->value);
            *it = var; 
            return it;
        }
    da_push(frame, var);
    return &da_top(frame);
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
            // TODO set_stack_Variable ?
            da_push(&new_frame, (Variable){ .name = args_def.list[i].str });
            GOTRY(eval(ctx, li.list[i+1], &da_top(&new_frame).value));
            Rc_inc_List(da_top(&new_frame).value);
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
    GOTRY(eval(ctx, func_def.list[0], out));

end:
    res = true;
fail:
    if (da_top(&ctx->args_stack).size > 1) // first stack frame should never be pop
    {
        da_for (Variable, it, &da_top(&ctx->args_stack))
        {
            Rc_dec_List(it->value);
        }
        da_free(&da_top(&ctx->args_stack));
        ctx->args_stack.size--;
    }
    return res;
}

// return true if it remplace or destroy one (or more) local or global variable
bool set_reset_Variable(Lisp_context *ctx, Variable var)
{
    bool remplace = false;
    // first look in the stack frame
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&ctx->args_stack))
            if (Strv_equal(it->name, var.name))
            {
                Rc_dec_List(it->value);
                remplace = true;
                break;
            }
    
    // set or replace variable var.name
    Variable *old = set_Variable_emplace(&ctx->variables, var);
    if (!VAR_IS_NULL(*old))
    {
        Rc_dec_List(old->value);
        remplace = true;
    }

    Rc_inc_List(var.value);
    *old = var;
    
    return remplace;
}

// return true if it remplace a local variable
bool local_Variable(Lisp_context *ctx, Variable var)
{
    Rc_inc_List(var.value);
    if (ctx->args_stack.size > 0)
        da_for (Variable, it, &da_top(&ctx->args_stack))
            if (Strv_equal(it->name, var.name))
            {
                Rc_dec_List(it->value);
                *it = var;
                return true;
            }
    da_push(&da_top(&ctx->args_stack), var);
    

    return false;
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
    case tag_string: {
        // REFMAYBE
        *out = li;
    } return true;
    case tag_true:
    case tag_number: {
        *out = li;
    } return true;

    case tag_symbole: {

        Variable *var_fun;
        Variable key = { .name = li.str };

        var_fun = get_variable_in_stack(ctx, key);
        if (var_fun)
        {
            TRY(eval(ctx, var_fun->value, out));
            return true;
        }
        var_fun = set_Variable_get(&ctx->variables, key);
        if (var_fun)
        {
            TRY(eval(ctx, var_fun->value, out));
            return true;
        }
        TRY(set_Variable_get(&ctx->functions, key), 
            error_log("unexpected function symbole: %sv", &key.name)
        );
        error_log("no variable nor function named: %sv", &key.name);
    } return false;
    case tag_list: {

        // nil|false
        if (IS_NIL(li))
        {
            *out = li;
            return true;
        }

        const List op = *li.list; 
        // TRY(op.tag == tag_symbole, error_log("evaluating a list that doesn't start with a symbole"));
        if (op.tag != tag_symbole)
        { // can be an inline function
            TRY(eval_function(ctx, li, NULL, out), error_log("failed to call inline function"));
            return true;
        }

        // TODO transform into an prefect hash table
        // uint16_t a = *(uint16_t)&op.str.arr;
        
        // Rc_List_array *rc = Rc_get_list(&li);

        if (List_str_equal(op, "local"))
        {
            TRY(li.size == 3, error_log("expected 3 element list for local got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to local to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1].str };
            TRY(eval(ctx, li.list[2], &var.value));

            local_Variable(ctx, var);

            return true;
        }
        if (List_str_equal(op, "set"))
        {
            TRY(li.size == 3, error_log("expected 3 element list for set got %d", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to set to got %s", tag_to_string(li.list[1].tag)));

            Variable var = { .name = li.list[1].str };
            TRY(eval(ctx, li.list[2], &var.value));
            
            set_reset_Variable(ctx, var);

            return true;
        }
        if (List_str_equal(op, "copy"))
        {
            TRY(li.size == 2, error_log("expected only 1 argument to be copyed got %d elements", li.size));

            List to_copy = {0};
            TRY(eval(ctx, li.list[1], &to_copy));
            *out = List_copy(&ctx->arena, to_copy);
            return true;
        }
        if (List_str_equal(op, "defun"))
        {
            TRY(li.size == 3, error_log("expected 3 element list for defun got %d elements", li.size));
            TRY(li.list[1].tag == tag_symbole, error_log("expected a symbole to defun to got %s", tag_to_string(li.list[1].tag)));

            Variable var = {
                .name = li.list[1].str,
                .value = li.list[2]
            };

            // set or replace function var.name
            Variable *old = set_Variable_emplace(&ctx->functions, var);
            if (!VAR_IS_NULL(*old))
                Rc_dec_List(old->value);
            
            Rc_inc_List(var.value);
            *old = var;
            return true;
        }
        if (List_str_equal(op, "?"))
        {
            TRY(li.size == 4, error_log("expected 4 element for '?' got %d", li.size));
            List cond = {0};
            TRY(eval(ctx, li.list[1], &cond));
            return eval(ctx, li.list[(!IS_NIL(cond)) ? 2 : 3], out);
        }
        if (List_str_equal(op, "if"))
        {
            TRY(li.size == 3, error_log("expected 3 element for 'if' got %d", li.size));
            List cond = {0};
            TRY(eval(ctx, li.list[1], &cond));
            if (!IS_NIL(cond))
                return eval(ctx, li.list[2], out);
            return true;
        }
        if (List_str_equal(op, "print"))
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
        if (List_str_equal(op, "while"))
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
        if (List_str_equal(op, "return"))
        {
            TRY(li.size == 1 || li.size == 2);
            if (li.size == 2)
                TRY(eval(ctx, li.list[1], out));
            
            // REFMAYBE
            ctx->in_return = true;
            return false;
        }
        if (List_str_equal(op, "+"))
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
        if (List_str_equal(op, "-"))
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
        if (List_str_equal(op, "*"))
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
        if (List_str_equal(op, "/"))
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
        if (List_str_equal(op, "//"))
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
        if (List_str_equal(op, "=="))
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
        if (List_str_equal(op, "!="))
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
        if (List_str_equal(op, "!"))
        {
            TRY(li.size >= 2, error_log("expected at least 2 elements for '==' got %d", li.size));
            
            List res = {0};
            TRY(eval(ctx, li.list[1], &res));
            if (IS_NIL(res))
                *out = TRUE_LIST;
            return true;
        }
        if (List_str_equal(op, "&&"))
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
        if (List_str_equal(op, "||"))
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
        if (List_str_equal(op, "first"))
        {
            TRY(li.size == 2);
            TRY(eval(ctx, li.list[1], out),  *out = NIL_LIST);
            TRY(out->tag == tag_list,        *out = NIL_LIST);
            TRY(out->size > 0,              *out = NIL_LIST; error_log("can't take first element of an empty list"));
            
            *out = out->list[0];
            return true;
        }
        if (List_str_equal(op, "next"))
        {
            TRY(li.size == 2);
            TRY(eval(ctx, li.list[1], out));
            TRY(out->tag == tag_list, *out = NIL_LIST);
            
            if (out->quote_count > 0)
            { // (next (quote (a b))) -> (a b)
                // Rc_inc_List(*out); // only inc in "set"
                out->quote_count--;
            }
            else if (out->size > 1)
            {
                // Rc_inc_List(*out); // only inc in "set"
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
        if (List_str_equal(op, "for"))
        {
            TRY(li.size >= 4);
            TRY(li.list[1].tag == tag_symbole);
            
            List iterable = {0};
            TRY(eval(ctx, li.list[2], &iterable));
            TRY(iterable.tag == tag_list);

            Variable *it = set_stack_Variable(ctx, (Variable){ .name = li.list[1].str });
            for (int i = 0; i < iterable.size; i++)
            {
                it->value = iterable.list[i];

                for (int j = 3; j < li.size; j++)
                {
                    TRY(eval(ctx, li.list[j], out));
                }
            }
            
            return true;
        }
        if (List_str_equal(op, "format"))
        { // catstr
            TRY(li.size >= 2);

            Strb acc = {0};

            for (int i = 1; i < li.size; i++)
            {
                List tmp = {0};
                TRY(eval(ctx, li.list[i], &tmp));
                TRY(dump(&acc, tmp));
            }

            // Strb_fit(&acc);
            *out = (List){
                .tag = tag_string,
                .size = acc.size,
                .str = memcpy(Rc_container_make(acc.size, sizeof(char)), acc.arr, acc.size)
            };
            Rc_get_str(out->str)->ref_count = -1; // orphan

            return true;
        }

        { // variable or function
            Variable *var_fun;
            Variable key = { .name = op.str };
            var_fun = get_variable_in_stack(ctx, key);
            if (var_fun)
            { // got a local variable
                TRY(eval(ctx, var_fun->value, out));
                return true;
            }
            var_fun = set_Variable_get(&ctx->variables, key);
            if (var_fun)
            { // got global variable
                TRY(eval(ctx, var_fun->value, out));
                return true;
            }
            var_fun = set_Variable_get(&ctx->functions, key);
            if (var_fun)
            { // got function
                // li.list.arr[0] = var_fun->value;
                TRY(eval_function(ctx, li, &var_fun->value, out));
                return true;
            }
        }
        
        error_log("no primitive '%sv' found to evaluate a list", &op.str);
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
    case tag_number: return fabs(li1.number - li2.number) < 1.0E-14;
    case tag_list: {
        TRY(li1.size == li2.size);
        for (int i = 0; i < li1.size; i++)
            TRY(List_equal(li1.list[i], li2.list[i]));
        return true;
    } break;
    case tag_string:    return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_symbole:   return Strv_equal(List_to_Strv(li1), List_to_Strv(li2));
    case tag_true:      return li2.tag == tag_true;
    default: UNREACHABLE("List equal");
    }
    return false;
}


List List_copy(Ar *arena, const List li)
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
            .list = Rc_container_make(li.size, sizeof(List))
        };

        for (size_t i = 0; i < li.size; i++)
            res.list[i] = List_copy(arena, li.list[i]);
        
        return res;
    }
    if (li.tag == tag_string)
    {
        List res = {
            .tag = tag_string,
            .quote_count = li.quote_count,
            .offset = 0,
            .size = li.size,
            .str = memcpy(
                Rc_container_make(li.size, sizeof(char)), 
                li.str, 
                li.size
            )
        };
        return res;
    }
    // tag_symbole have a static lifetime for now. To see for meta programming 

    return li;
}


// dec Rc
void List_free(List *li)
{
    if (!li) return ;
    
    if (li->tag == tag_list)
    {
        if (IS_NIL(*li)) return ;

        for (size_t i = 0; i < li->size; i++)
            List_free(&li->list[i]);
    }
    
    Rc_dec_List(*li);
    
    // tag_symbole have a static lifetime for now. To see for meta programming 
}

Lisp_context Lisp_context_init(void)
{
    Lisp_context res = {0};
    da_push_zero(&res.args_stack);

    return res;
}

void Lisp_context_free(Lisp_context *ctx)
{
    da_free(&ctx->args_stack);
    
    set_for (Variable, it, &ctx->variables)
    {
        // Strv_Rc_dec(it->name); symbole
        List_free(it);
    }
    set_Variable_free(&ctx->variables);
    
    set_for (Variable, it, &ctx->functions)
    {
        // Strv_Rc_dec(it->name); symbole
        List_free(it);
    }
    set_Variable_free(&ctx->functions);
}

