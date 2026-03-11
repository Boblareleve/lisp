#ifndef LISP_H
#define LISP_H

#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
#include "ar.h"



#define NIL_LIST (List){0}
#define TRUE_LIST (List){ .tag = tag_true }
#define IS_NIL(li) ((li).tag == tag_list && (li).list.size == 0)
typedef struct List
{
    enum {
        tag_list = 0,  // (a a a)|()
        tag_true,      // t
        tag_symbole,   // 
        tag_string,    // "dslmjkfdsqml"
        tag_number,    // 4326324
        // tag_reference, // ?
        // tag_function   // (foo )
    } tag;
    uint32_t reference_count; // how many reference depth it is
    union {
        struct {
            struct List *arr;
            size_t size;
        } list;
        // struct List *ref;
        Strv str;
        double number;
    };
} List;


typedef struct Variable
{
    List value;
    Strv name;
} Variable;
DA_TYPEDEF_ARRAY(Variable);

SET_TYPEDEF_HASH_SET(Variable);



DA_TYPEDEF_ARRAY(da_Variable);
typedef struct Lisp_context
{
    // globals
    set_Variable variables; 
    set_Variable functions;

    // stack (local)
    da_da_Variable args_stack;


    // Strb error;
} Lisp_context;



extern Strb error;
#define error_log(msg, ...)\
do {\
    if (error.size > 0) Strb_cat(&error, "\n");\
    Strb_catf(&error, "%s:%s:%d: ", __FILE__, __func__, __LINE__);\
    Strb_catf(&error, msg __VA_OPT__(,) __VA_ARGS__);\
} while (0)
#define get_error() (error.view)
#define reset_error() (error.size)



bool list(Strv *str, List *li);
bool dump(Strb *out, const List li);
bool print(const List li);
bool eval(Lisp_context *ctx, const List li, List *out);
bool List_equal(const List li1, const List li2);
void List_free(List *li);
void Lisp_context_free(Lisp_context *ctx);

void skip_space(Strv *str);
void skip_comment(Strv *str);


#endif /* LISP_H */
