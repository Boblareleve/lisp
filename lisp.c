

#define STRING_IMPLEMENTATION
#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
#define AR_IMPLEMENTATION
#include "ar.h"



struct List;

typedef struct List List;
DA_TYPEDEF_ARRAY(List);

#define NIL_LIST (List){0}
struct List
{
    enum {
        tag_nil = 0,
        tag_true,
        tag_symbole,
        tag_string,
        tag_number,
        tag_reference,
        tag_list,
        tag_function
    } tag;
    union {
        da_List list;
        Strv str;
        double number;
    };
};

const char *tag_to_string(int tag)
{
    static const char *table[] = {
        [tag_nil]       = "tag_nil",
        [tag_true]      = "tag_true",
        [tag_symbole]   = "tag_symbole",
        [tag_string]    = "tag_string",
        [tag_number]    = "tag_number",
        [tag_reference] = "tag_reference",
        [tag_list]      = "tag_list",
        [tag_function]  = "tag_function"
    };
    return table[tag];
}

typedef struct Variable
{
    List value;
    Strv name;
} Variable;
DA_TYPEDEF_ARRAY(Variable);

SET_TYPEDEF_HASH_SET(Variable);
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



DA_TYPEDEF_ARRAY(da_Variable);
typedef struct Lisp_context
{
    set_Variable variables;
    da_da_Variable args_stack;
    // Strb error;
} Lisp_context;


Strb error = {0};
#define error_log(msg, ...)\
do {\
    if (error.size > 0) Strb_cat(&error, "\n");\
    Strb_catf(&error, "%s:%s:%d: ", __FILE__, __func__, __LINE__);\
    Strb_catf(&error, msg __VA_OPT__(,) __VA_ARGS__);\
} while (0)


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


int is_end(int c) { return c == ')' || isspace(c); }
int not_is_end(int c) { return !is_end(c); }

bool list(Strv *str, List *li)
{
    TRY(li, error_log("no output list to parse"));
    TRY(Strv_first(*str) != ')', error_log("closing parent at root"));
    TRY(str->size > 0, error_log("empty input"));
   
    skip_space(str);
    skip_comment(str);
    


    if (Strv_first(*str) == '(')
    {
        li->tag = tag_list;
        TRY(consume(str), error_log("EOF"));
        skip_space(str);
        TRY(str->size > 0, error_log("EOF"));
        if (Strv_first(*str) == ')')
        {
            *li = NIL_LIST;
            Strv_inc(str);
            return true;
        }

        do {
            da_push_zero(&li->list);
            TRY(list(str, &da_top(&li->list)));
            skip_space(str);
        } while (str->size > 0 && Strv_first(*str) != ')');

        TRY(Strv_first(*str) == ')');
        Strv_inc(str);
        return true;
    }

    
    if (isdigit(Strv_first(*str)) 
     || Strv_first(*str) == '.')
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
    if (Strv_first(*str) == '"') // string TODO escape
    {
        Strv_inc(str);
        *li = (List){
            .tag = tag_string,
            .str = Strv_c_substr(str, '"')
        };
        Strv_inc(str);
        return true;
    }
    if (Strv_first(*str) == '\'')
    {
        TODO("refs");
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


bool __dump(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_symbole: {
        Strb_catf(out, "symbole: '"STRV_FMT"'", STRV_UNPACK(li.str));
    } break;
    case tag_list: {
        Strb_catf(out, "(list {%d}:\n", li.list.size);
        da_for (const List, it, &li.list)
        {
            TRY(__dump(out, *it, indent + 2));
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
    case tag_nil: {
        Strb_cat(out, "nil");
    } break;
    case tag_function: {
        Strb_catf(out, "(fun {%d}:\n", li.list.size);
        da_for (const List, it, &li.list)
        {
            TRY(__dump(out, *it, indent + 2));
            Strb_cat(out, "\n");
            // if (it != &da_top(&li.list))
            //     Strb_cat_char(out, ' ');
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");

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
            // if (it != &da_top(&li.list))
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
    case tag_nil: {
        Strb_cat(out, "nil");
    } break;
    case tag_function: {
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

    } break;
    default: Strb_cat(out, "UNKOWN"); break;
    }
    return true;
}
bool dump(Strb *out, const List li)
{
    return __dump(out, li, 0);
}
bool print(const List li)
{
    static Strb to_print = {0};
    to_print.size = 0;

    TRY(dump(&to_print, li));
    printf(STRV_FMT, STRV_UNPACK(to_print.view));
    return true;
}




bool eval(Lisp_context *ctx, const List li, List *out);
bool List_equal(const List li1, const List li2);


// set in global state
bool eval_set(Lisp_context *ctx, const da_List arr)
{
    TRY(arr.size == 3, error_log("expected 3 element list for set got %d", arr.size));
    TRY(arr.arr[1].tag == tag_symbole, error_log("expected a symbole to set to got %s", tag_to_string(arr.arr[1].tag)));

    Variable var = {
        .name = arr.arr[1].str,
        .value = arr.arr[2]
    };

    *set_Variable_insert(&ctx->variables, var) = var;
    return true;
}

bool eval_defun(Lisp_context *ctx, const da_List arr)
{
    TRY(arr.size >= 4, error_log("expected 4 element list for defun got %d", arr.size)); // defun name params ...
    TRY(arr.arr[1].tag == tag_symbole, error_log("expected symbole as function name got %s", tag_to_string(arr.arr[1].tag)));
    TRY(arr.arr[2].tag == tag_list, error_log("expected a list of arguments got %s", tag_to_string(arr.arr[2].tag)));

    // args are only symboles
    da_List args = arr.arr[2].list;
    da_for (List, it, &args)
        TRY(it->tag == tag_symbole, error_log("all arguments should be symboles got %s", tag_to_string(it->tag)));

    Variable fun = {
        .name = arr.arr[1].str,
        .value = {
            .tag = tag_function,
            .list = arr // the entire definition (with defun)
        }
    };
    *set_Variable_insert(&ctx->variables, fun) = fun;
    return true;
}


bool eval_funcall(Lisp_context *ctx, const da_List call, List *out)
{
    TRY(da_first(&call).tag == tag_function);

    bool res = false;

    const da_List def_args = da_first(&call).list.arr[2].list;
    TRY(def_args.size == call.size - 1, error_log("expected %d arguments got %d", def_args.size, call.size-1));
    
    // push args with their names in stack
    da_push_zero(&ctx->args_stack);
    for (int i = 0; i < def_args.size; i++)
    {
        da_push_struct(&da_top(&ctx->args_stack), Variable, 
            .name = def_args.arr[i].str,
            .value = call.arr[i+1]
        );
    }

    // execute statements
    const da_List statements = da_first(&call).list;
    for (int i = 3; i+1 < statements.size; i++)
        GOTRY(eval(ctx, statements.arr[i], &(List){0}));
    GOTRY(eval(ctx, da_top(&statements), out));

    res = true;
fail:
    if (da_top(&ctx->args_stack).size > 0)
    {
        da_free(&da_top(&ctx->args_stack));
        da_top(&ctx->args_stack).size--;
    }
    return res;
}


// call: function name followed by args 
// function: body of the function
/* bool eval_funcall(Lisp_context *ctx, const List function, const da_List call, List *out)
{
    bool res = false;

    const da_List def_args = function.list.arr[2].list;
    TRY(def_args.size == call.size - 1, error_log("expected %d arguments got %d", def_args.size, call.size-1));
    
    // push args with their names in stack
    da_push_zero(&ctx->args_stack);
    for (int i = 0; i < def_args.size; i++)
    {
        da_push_struct(&da_top(&ctx->args_stack), Variable, 
            .name = def_args.arr[i].str,
            .value = call.arr[i+1]
        );
    }

    // execute statements
    const da_List statements = function.list;
    for (int i = 3; i+1 < statements.size; i++)
        GOTRY(eval(ctx, statements.arr[i], &(List){0}));
    GOTRY(eval(ctx, da_top(&statements), out));

    res = true;
fail:
    if (da_top(&ctx->args_stack).size > 0)
    {
        da_free(&da_top(&ctx->args_stack));
        da_top(&ctx->args_stack).size--;
    }
    return res;
}
 */


// substitue the variable with his value
bool eval_symbole(Lisp_context *ctx, const List symbole, List *out)
{
    TRY(symbole.tag == tag_symbole, error_log("expected symbole got %s", tag_to_string(symbole.tag)));

    Variable *res = set_Variable_get(&ctx->variables, (Variable){ .name = symbole.str });
    if (!res)
    {
        if (ctx->args_stack.size > 0)
            da_for (Variable, it, &da_top(&ctx->args_stack))
                if (Strv_equal(it->name, symbole.str))
                    res = it;
        
        TRY(res, error_log("no variable named '"STRV_FMT"'", STRV_UNPACK(symbole.str)));
    }

    TRY(eval(ctx, res->value, out));

    return true;
}

bool eval_list(Lisp_context *ctx, const List li, List *out)
{
    TRY(li.tag == tag_list, error_log("expected list got %s", tag_to_string(li.tag)));
    const da_List arr = li.list;

    if (arr.size == 0)
    {
        *out = NIL_LIST;
        return true;
    }

    if (da_first(&arr).tag == tag_symbole)
    {
        Strv head_element = da_first(&arr).str;
        
        if (Strv_equal_lit(head_element, "?"))
        {
            TRY(arr.size == 4, error_log("expected 4 element for 'if' got %d", arr.size));
            List cond = {0};
            TRY(eval(ctx, arr.arr[1], &cond));
            return eval(ctx, arr.arr[(cond.tag != tag_nil) ? 2 : 3], out);
        }
        if (Strv_equal_lit(head_element, "print"))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for 'print' got %d", arr.size));
            out->tag = tag_nil;
            for (int i = 1; i < arr.size; i++)
            {
                List li_to_print = {0};
                TRY(eval(ctx, arr.arr[i], &li_to_print), error_log("failed to eval to print"));
                TRY(print(li_to_print), error_log("failed to print"));
                printf("\n");
            }
            return true;
        }
        if (Strv_equal_lit(head_element, "set"))
        {
            *out = NIL_LIST;
            return eval_set(ctx, arr);
        
        }
        if (Strv_equal_lit(head_element, "defun"))
        {
            *out = NIL_LIST;
            return eval_defun(ctx, arr);
        }
        if (Strv_equal_lit(head_element, "+"))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '+' got %d", arr.size));
            out->tag = tag_number;
            out->number = 0.0;
            for (int i = 1; i < arr.size; i++)
            {
                List res = {0};
    
                TRY(eval(ctx, arr.arr[i], &res));
                TRY(res.tag == tag_number, error_log("expected a number to add got %s", tag_to_string(res.tag)));
    
                out->number += res.number;
            }
            return true;
        }
        if (Strv_equal_lit(head_element, "-"))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '-' got %d", arr.size));
            
            { // first set res to arr[1]
                List res = {0};
        
                TRY(eval(ctx, arr.arr[1], &res));
                TRY(res.tag == tag_number, error_log("expected a number to substract got %s", tag_to_string(res.tag)));
                
                out->tag = tag_number;
                out->number = res.number;
            }
            
            for (int i = 2; i < arr.size; i++)
            { // then -=
                List res = {0};
    
                TRY(eval(ctx, arr.arr[i], &res));
                TRY(res.tag == tag_number, error_log("expected a number to substract got %s", tag_to_string(res.tag)));
    
                out->number -= res.number;
            }
            return true;
        }
        if (Strv_equal_lit(head_element, "*"))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '*' got %d", arr.size));
            out->tag = tag_number;
            out->number = 1.0;
            for (int i = 1; i < arr.size; i++)
            {
                List res = {0};
    
                TRY(eval(ctx, arr.arr[i], &res));
                TRY(res.tag == tag_number, error_log("expected a number to multiply got %s", tag_to_string(res.tag)));
                
                out->number *= res.number;
            }
            return true;
        }
        if (Strv_equal_lit(head_element, "/"))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '/' got %d", arr.size));
            
            { // first set res to arr[1]
                List res = {0};
        
                TRY(eval(ctx, arr.arr[1], &res));
                TRY(res.tag == tag_number, error_log("expected a number to divide got %s", tag_to_string(res.tag)));
                
                out->tag = tag_number;
                out->number = res.number;
            }
            
            for (int i = 2; i < arr.size; i++)
            { // then /=
                List res = {0};
    
                TRY(eval(ctx, arr.arr[i], &res));
                TRY(res.tag == tag_number, error_log("expected a number to divide got %s", tag_to_string(res.tag)));
    
                out->number /= res.number;
            }
            return true;
        }
        if (Strv_equal_lit(head_element, "=="))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '==' got %d", arr.size));
            
            List acc = {0};
            TRY(eval(ctx, arr.arr[1], &acc));
            
            out->tag = tag_true;
            for (int i = 2; i < arr.size; i++)
            {
                List res = {0};
                
                TRY(eval(ctx, arr.arr[i], &res));
                
                if (!List_equal(acc, res))
                {
                    out->tag = tag_nil;
                    break;
                }
            }
            return true;
        }
        /* if (Strv_equal_lit(head_element, ">="))
        {
            TRY(arr.size >= 2, error_log("expected at least 2 elements for '>=' got %d", arr.size));
            
            List acc = {0};
            TRY(eval(ctx, arr.arr[1], &acc));
            TRY(acc.tag == tag_number, error_log("expected a number to compare got %s", tag_to_string(acc.tag)));
            
            out->tag = tag_true;
            for (int i = 2; i < arr.size; i++)
            {
                List res = {0};
                
                TRY(eval(ctx, arr.arr[i], &res));
                TRY(res.tag == tag_number, error_log("expected a number to compare got %s", tag_to_string(res.tag)));
                
                if (!List_equal(acc, res))
                {
                    out->tag = tag_nil;
                    break;
                }
            }
            return true;
        }
         */

        Variable *var = set_Variable_get(&ctx->variables, (Variable){ .name = head_element });
        if (var) TRY(eval(ctx, var->value, &da_first(&arr)));
        if (da_first(&arr).tag == tag_function)
            return eval_funcall(ctx, arr, out);
    }
    else if (da_first(&arr).tag == tag_function)
        return eval_funcall(ctx, arr, out);
    else if (da_first(&arr).tag == tag_list 
          && da_first(&da_first(&arr).list).tag == tag_symbole
          && Strv_equal_lit(da_first(&da_first(&arr).list).str, "defun")
    )
        return eval_funcall(ctx, arr, out);
    
    
    // printf("\n");
    // print(li);
    // printf("\n");

    
    for (int i = 0; i < arr.size; i++)
    {
        if (arr.arr[i].tag == tag_symbole)
            TRY(eval_symbole(ctx, arr.arr[i], &arr.arr[i]));
        
    }

    
    
    *out = li;
    return true;
}

bool eval(Lisp_context *ctx, const List li, List *out)
{
    TRY(out, error_log("no output"));
    switch (li.tag)
    {
    case tag_list:     return eval_list(ctx, li, out);
    case tag_symbole:  return eval_symbole(ctx, li, out); // can't eval a symbole
    case tag_function: 
    case tag_number:
    case tag_nil:
    case tag_string:
    case tag_true:     
        *out = li; 
        return true;

    default: UNREACHABLE("invalid tag");
    }
    return false;
}

void List_free(List *li)
{
    if (li->tag == tag_list)
    {
        da_for (List, it, &li->list)
            List_free(it);
        da_free(&li->list);
    }
}
void Variable_free(Variable *var)
{
    // var->value;
}
void Lisp_context_free(Lisp_context *ctx)
{
    set_Variable_free_fun_ptr(&ctx->variables, Variable_free);
    da_for (da_Variable, it, &ctx->args_stack)
    {
        da_for (Variable, jt, it)
            Variable_free(jt);
        da_free(it);
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
    case tag_function: TODO("fun equal"); break;
    case tag_reference: TODO("ref equal"); break;
    case tag_string: return Strv_equal(li1.str, li2.str);
    case tag_symbole: return Strv_equal(li1.str, li2.str);
    case tag_true: return li2.tag == tag_true;
    case tag_nil:  return li2.tag == tag_nil;
    default: UNREACHABLE("List equal");
    }
    return false;
}

