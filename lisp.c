

#define STRING_IMPLEMENTATION
#define AR_IMPLEMENTATION
#include "lisp.h"



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
        // [tag_nil]       = "tag_nil",
        [tag_true]      = "tag_true",
        [tag_symbole]   = "tag_symbole",
        [tag_string]    = "tag_string",
        [tag_number]    = "tag_number",
        // [tag_reference] = "tag_reference",
        [tag_list]      = "tag_list",
        // [tag_function]  = "tag_function"
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


int is_end(int c)     { return c == ')' || isspace(c); }
int not_is_end(int c) { return !is_end(c);             }


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
            count++;
            continue;
        }
        if (Strv_first(str) == '\'') // reference
        {
            do GOTRY_consume(&str); while (Strv_first(str) == '\'');
            skip_comment(&str);
            GOTRY(str.size > 0, error_log("expected atom after reference (') got EOF"));
        }


        GOTRY(skip_atom(&str));
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

bool list(Strv *str, List *li)
{
    // test_list_count(); // TODO rm

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
        li->list.arr = malloc(count * sizeof(List));
        li->list.size = 0;
        do {
            li->list.arr[li->list.size] = NIL_LIST;
            TRY(list(str, &li->list.arr[li->list.size]));
            li->list.size++;
            skip_space(str);
        } while (li->list.size < count && str->size > 0 && Strv_first(*str) != ')');
        TRY(li->list.size == count, error_log("invalid list element count, expected %d got %d with: %sv", count, li->list.size, &save));
        TRY(Strv_first(*str) == ')', error_log("unexpected EOF or underestimate list_count()"));
        Strv_inc(str);
        return true;
    }
    if (isdigit(Strv_first(*str)) || (is_unary_sign(Strv_first(*str)) && isdigit(Strv_char_at(*str, 1))))
    { // numbers
        char *it = str->arr;
        char *end = &str->arr[str->size];

        *li = (List){
            .tag = tag_number,
            .number = strtod(str->arr, &it)
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
        Strv_inc(str);
        *li = (List){
            .tag = tag_string,
            .str = Strv_range(begin, str->arr)
        };
        return true;
    }
    if (Strv_first(*str) == '\'')
    {
        // count ref depth
        uint32_t count = 1;

        do TRY(consume(str)); while (Strv_first(*str) == '\'');

        TRY(list(str, li));
        li->quote_count = count;
        
        return true;
    }
    
    // any symbole (yes can be any character)
    *li = (List){
        .tag = tag_symbole,
        .str = Strv_fun_substr(str, not_is_end)
    };

    // special case true
    if (Strv_equal_lit(li->str, "t"))
        *li = (List){ .tag = tag_true };
    
    return true;
}



bool _dump_indent(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_symbole: {
        Strb_catf(out, "symbole: '"STRV_FMT"'", STRV_UNPACK(li.str));
    } break;
    case tag_list: {
        if (li.list.size == 0)
        {
            Strb_cat(out, "()");
            break;
        }
        Strb_catf(out, "(list {%d}:\n", li.list.size);
        da_for (const List, it, &li.list)
        {
            TRY(_dump_indent(out, *it, indent + 2));
            Strb_cat(out, "\n");
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");
    } break;
    case tag_string: {
        Strb_catf(out, "string: \""STRV_FMT"\"", STRV_UNPACK(li.str));
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

bool _dump(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_symbole: {
        Strb_catf(out, "'"STRV_FMT"'", STRV_UNPACK(li.str));
    } break;
    case tag_list: {    
        Strb_catf(out, "({%d}\n", li.list.size);
        da_for (const List, it, &li.list)
        {
            TRY(_dump(out, *it, indent + 2));
            Strb_cat(out, "\n");
            // if (it != &da_top(&li.list))   (a a a?)
            //     Strb_cat_char(out, ' ');
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");
    } break;
    case tag_string: {
        Strb_catf(out, "\""STRV_FMT"\"", STRV_UNPACK(li.str));
    } break;
    case tag_number: {
        if (fmod(li.number, 1.0) == 0.0)
            Strb_catf(out, "%.0f64", li.number);
        else
            Strb_catf(out, "%f64", li.number);
    } break;
    case tag_true: {
        Strb_cat(out, "true");
    } break;
    /* case tag_function: {
        Strb_catf(out, "(fun{%d}\n", li.list.size);
        da_for (const List, it, &li.list)
        {
            TRY(_dump(out, *it, indent + 2));
            Strb_cat(out, "\n");
            // if (it != &da_top(&li.list))
            //     Strb_cat_char(out, ' ');
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");

    } break; */
    default: Strb_cat(out, "UNKOWN"); break;
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



// li: (((args_def ...) statements ...) args_call)
// or
// function_def != NULL => li: (name args_call) and function_def: ((args_def ...) statements ...)
bool eval_function(Lisp_context *ctx, const List li, const List *function_def, List *out)
{
    bool res = false;
    
    const List func_def = function_def ? *function_def : da_first(&li.list);
    TRY(func_def.tag == tag_list && func_def.list.size >= 2, error_log("not a function definition"));

    const List args_def = da_first(&func_def.list);
    TRY(args_def.tag == tag_list);

    TRY(args_def.list.size == li.list.size - 1, error_log("expected %d arguments got %d", args_def.list.size, li.list.size-1));
    
    // push args with their names in stack
    da_push_zero(&ctx->args_stack);
    for (int i = 0; i < args_def.list.size; i++)
    {
        da_push_struct(&da_top(&ctx->args_stack), Variable, 
            .name = args_def.list.arr[i].str,
            // .value = li.list.arr[i+1]
        );
        GOTRY(eval(ctx, li.list.arr[i+1], &da_top(&da_top(&ctx->args_stack)).value));
    }

    // execute statements
    for (int i = 1; i+1 < func_def.list.size; i++)
        GOTRY(eval(ctx, func_def.list.arr[i], &NIL_LIST));
    
    // return the last one
    GOTRY(eval(ctx, da_top(&func_def.list), out));

    res = true;
fail:
    if (da_top(&ctx->args_stack).size > 0)
    {
        da_free(&da_top(&ctx->args_stack));
        da_top(&ctx->args_stack).size--;
    }
    return res;
}



bool eval(Lisp_context *ctx, const List li, List *out)
{
    TRY(out, error_log("no output"));
    *out = NIL_LIST;

    // dec ref count
    if (li.quote_count)
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

        Variable *var_fun;
        Variable key = { .name = li.str };

        var_fun = get_variable_in_stack(ctx, key);
        if (var_fun)
        {
            *out = var_fun->value;
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
        if (li.list.size == 0)
        {
            *out = li;
            return true;
        }

        const List op = li.list.arr[0]; 
        // TRY(op.tag == tag_symbole, error_log("evaluating a list that doesn't start with a symbole"));
        if (op.tag != tag_symbole)
        { // can be an inline function
            TRY(eval_function(ctx, li, NULL, out), error_log("failed to call inline function"));
            return true;
        }

        if (Strv_equal_lit(op.str, "?"))
        {
            TRY(li.list.size == 4, error_log("expected 4 element for 'if' got %d", li.list.size));
            List cond = {0};
            TRY(eval(ctx, li.list.arr[1], &cond));
            return eval(ctx, li.list.arr[(!IS_NIL(cond)) ? 2 : 3], out);
        }
        if (Strv_equal_lit(op.str, "print"))
        {
            TRY(li.list.size >= 2, error_log("expected at least 2 elements for 'print' got %d", li.list.size));

            for (int i = 1; i < li.list.size; i++)
            {
                List li_to_print = {0};
                TRY(eval(ctx, li.list.arr[i], &li_to_print), error_log("failed to eval to print"));
                TRY(List_print(li_to_print), error_log("failed to print"));
                printf("\n");
            }
            return true;
        }
        if (Strv_equal_lit(op.str, "set"))
        { // lazy evaluate
            TRY(li.list.size == 3, error_log("expected 3 element list for set got %d", li.list.size));
            TRY(li.list.arr[1].tag == tag_symbole, error_log("expected a symbole to set to got %s", tag_to_string(li.list.arr[1].tag)));

            Variable var = {
                .name = li.list.arr[1].str,
            };
            TRY(eval(ctx, li.list.arr[2], &var.value));

            // set or replace variable var.name
            *set_Variable_insert(&ctx->variables, var) = var;
            
            return true;
        }
        if (Strv_equal_lit(op.str, "defun"))
        {
            TRY(li.list.size == 3, error_log("expected 3 element list for defun got %d", li.list.size));
            TRY(li.list.arr[1].tag == tag_symbole, error_log("expected a symbole to defun to got %s", tag_to_string(li.list.arr[1].tag)));

            Variable var = {
                .name = li.list.arr[1].str,
                .value = li.list.arr[2]
            };

            // set or replace function var.name
            *set_Variable_insert(&ctx->functions, var) = var;
            
            return true;
        }
        if (Strv_equal_lit(op.str, "+"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 3 elements for '+' got %d", li.list.size));
            List res = {
                .tag = tag_number,
                .number = 0
            };
            for (int i = 1; i < li.list.size; i++)
            {
                List operand = {0};
    
                TRY(eval(ctx, li.list.arr[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to add got %s", tag_to_string(operand.tag)));
    
                res.number += operand.number;
            }
            *out = res;
            return true;
        }
        if (Strv_equal_lit(op.str, "-"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 3 elements for '-' got %d", li.list.size));
            List res = {0};
            TRY(eval(ctx, li.list.arr[1], &res));
            TRY(res.tag == tag_number);

            for (int i = 2; i < li.list.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list.arr[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to subtruct got %s", tag_to_string(operand.tag)));
                
                res.number -= operand.number;
            }
            *out = res;
            return true;
        }
        if (Strv_equal_lit(op.str, "*"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 2 elements for '*' got %d", li.list.size));
            List res = { 
                .tag = tag_number,
                .number = 1
            };
            for (int i = 1; i < li.list.size; i++)
            {
                List operand = {0};
    
                TRY(eval(ctx, li.list.arr[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to multiply got %s", tag_to_string(operand.tag)));
                
                res.number *= operand.number;
            }
            *out = res;
            return true;
        }
        if (Strv_equal_lit(op.str, "/"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 2 elements for '/' got %d", li.list.size));
            List res = {0};
            TRY(eval(ctx, li.list.arr[1], &res));
            TRY(res.tag == tag_number);

            for (int i = 2; i < li.list.size; i++)
            {
                List operand = {0};
                
                TRY(eval(ctx, li.list.arr[i], &operand));
                TRY(operand.tag == tag_number, error_log("expected a number to divide got %s", tag_to_string(operand.tag)));
                
                res.number /= operand.number;
            }
            *out = res;
            return true;
        }
        if (Strv_equal_lit(op.str, "=="))
        {
            TRY(li.list.size >= 3, error_log("expected at least 2 elements for '==' got %d", li.list.size));
            
            List acc = {0};
            TRY(eval(ctx, li.list.arr[1], &acc));
            
            for (int i = 2; i < li.list.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list.arr[i], &operand));
                
                if (!List_equal(acc, operand))
                    return true; // out is already set to nil 
                
            }
            *out = TRUE_LIST;
            return true;
        }
        if (Strv_equal_lit(op.str, "!"))
        {
            TRY(li.list.size >= 2, error_log("expected at least 2 elements for '==' got %d", li.list.size));
            
            List res = {0};
            TRY(eval(ctx, li.list.arr[1], &res));
            if (res.tag == tag_true)
                *out = NIL_LIST;
            else if (res.tag == tag_list && res.list.size == 0)
                *out = TRUE_LIST;
            else
                return false;
            return true;
        }
        if (Strv_equal_lit(op.str, "&&"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 3 elements for '&&' got %d", li.list.size));
            
            List acc = {0};
            TRY(eval(ctx, li.list.arr[1], &acc));
            
            for (int i = 2; i < li.list.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list.arr[i], &operand));
                
                if (acc.tag != tag_true || operand.tag != tag_true)
                    return true; // out is already set to nil (false)
            }
            *out = TRUE_LIST;
            return true;
        }
        if (Strv_equal_lit(op.str, "||"))
        {
            TRY(li.list.size >= 3, error_log("expected at least 3 elements for '||' got %d", li.list.size));
            
            List acc = {0};
            TRY(eval(ctx, li.list.arr[1], &acc));
            
            for (int i = 2; i < li.list.size; i++)
            {
                List operand = {0};
                TRY(eval(ctx, li.list.arr[i], &operand));
                
                if (acc.tag == tag_true && operand.tag == tag_true)
                {
                    *out = TRUE_LIST;
                    return true;
                }
            }
            return true; // out is already set to nil (false)
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
    if (li1.tag != li2.tag)
        return false;
    
    switch (li1.tag)
    {
    case tag_number: return fabs(li1.number - li2.number) < 0.00001;
    case tag_list: {
        TRY(li1.list.size == li2.list.size);
        for (int i = 0; i < li1.list.size; i++)
            TRY(List_equal(li1.list.arr[i], li2.list.arr[i]));
        return true;
    } break;
    // case tag_function:  TODO("fun equal"); break;
    // case tag_reference: TODO("ref equal"); break;
    case tag_string:    return Strv_equal(li1.str, li2.str);
    case tag_symbole:   return Strv_equal(li1.str, li2.str);
    case tag_true:      return li2.tag == tag_true;
    default: UNREACHABLE("List equal");
    }
    return false;
}


List List_copy(const List li)
{
    if (li.tag == tag_list)
    {
        List res = li;
        res.list.arr = malloc(sizeof(List) * res.list.size); 

        for (size_t i = 0; i < li.list.size; i++)
            res.list.arr[i] = List_copy(li.list.arr[i]);
        return res;
    }

    return li;
}

void List_free(List *li)
{
    if (li && li->tag == tag_list)
    {
        da_for (List, it, &li->list)
            List_free(it);
        free(li->list.arr);
    }
}


void Lisp_context_free(Lisp_context *ctx)
{
    // da_for (da_Variable, it, &ctx->args_stack)
    //     da_free(it);
    da_free(&ctx->args_stack);
    
    set_Variable_free(&ctx->variables);
    set_Variable_free(&ctx->functions);
}

