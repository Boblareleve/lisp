#ifndef LISP_H
#define LISP_H

// #define LISP_DEBUG_INFO

#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
#include <setjmp.h>
#include "ar_virt.h"


typedef enum List_tag {
    tag_list = 0,  // (a a a)|()
    tag_true,      // t
    tag_symbole,   // 
    tag_string,    // "dslmjkfdsqml"
    tag_number,    // 4326324 3.3
} List_tag;
// typedef uint16_t List_tag;


#define NIL_LIST (List){0}
#define TRUE_LIST (List){ .tag = tag_true }
#define IS_NIL(li) ((li).tag == tag_list && (li).list == NULL)



// while >= 0 object valid -> rc == 0 <=> one reference for (Rc_container){0} convinence
#define RC_CONTAINER_HEADER struct { size_t ref_count; size_t size; }

typedef struct Rc_container
{
    RC_CONTAINER_HEADER;
    uint8_t *arr[0];
} Rc_container;

typedef struct Rc_List_array
{
    RC_CONTAINER_HEADER;
    List arr[0];
} Rc_List_array;
typedef struct Rc_String
{
    RC_CONTAINER_HEADER;
    char arr[0];
} Rc_String;





// maybe get down to 8 bytes using uint32_t for indexing into a pool
typedef struct List
{
    List_tag tag;
    uint32_t quote_count; // how many reference "(QUOTE self)" depth it is
    union {
        List *list;
        char *str;
        // int64_t number; // double ?
        double number;
    };
} List;

static_assert(sizeof(Rc_container) == 16);
static_assert(sizeof(List) == 16);
static_assert(sizeof(List) == sizeof(Rc_container));


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
    bool in_return; // indicate that the error is only a return mechanism

    Ar arena;
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



bool list(Ar *arena, Strv *str, List *li);
bool dump(Strb *out, const List li);
bool List_print(const List li);
bool eval(Lisp_context *ctx, const List li, List *out);
bool List_equal(const List li1, const List li2);
void List_free(List *li);
List List_copy(Ar *arena, const List li);
Lisp_context Lisp_context_init(void);
void Lisp_context_free(Lisp_context *ctx);

void skip_space(Strv *str);
void skip_comment(Strv *str);


#endif /* LISP_H */
