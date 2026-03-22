#ifndef LISP_H
#define LISP_H

// #define LISP_DEBUG_INFO

#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
#include <setjmp.h>

#define AR_MAX_ALIGN 8
#include "ar_virt.h"

typedef enum List_tag : uint8_t
{
    tag_list = 0,  // (a a a)|()
    tag_true,      // t
    tag_symbole,   // 
    tag_string,    // "dslmjkfdsqml"
    tag_number,    // 4326324 3.3
    // tag_slice,     
    // tag_integer
    // tag_...
} List_tag;


#define NIL_LIST (List){0}
#define TRUE_LIST (List){ .tag = tag_true }
#define IS_NIL(li) ((li).tag == tag_list && (li).size == 0)



typedef struct List List;

// maybe get down to 8 bytes using uint32_t for indexing into a pool
struct List
{
    List_tag tag;
    uint8_t quote_count; // how many reference "(QUOTE self)" depth it is
    
    uint16_t offset; // only to get back the start of the allocated chunk
    uint16_t size;

    
    union {
        List *list;
        char *str;
        // int64_t number; // double ?
        double number;
    };
};
static_assert(sizeof(List) == 16);



typedef struct Variable
{
    List value;
    List name;
} Variable;
DA_TYPEDEF_ARRAY(Variable);

SET_TYPEDEF_HASH_SET(Variable);


DA_TYPEDEF_ARRAY_PTR(void);
DA_TYPEDEF_ARRAY(da_Variable);
typedef struct Lisp_context
{
    // globals
    set_Variable variables; 
    set_Variable functions;

    // stack (local)
    da_da_Variable args_stack;
    bool in_return; // indicate that the error is only a return mechanism

    List root;

    da_ptr_void gc;
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

static inline const char *tag_to_string(int tag)
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
static inline void *List_get_ptr(const List *li)
{
    if (li->tag == tag_list)
        return li->list - li->offset;
    if (li->tag == tag_symbole || li->tag == tag_string)
        return li->str - li->offset;

    return NULL;
}

#define List_to_Strv(li) (assert((li).tag == tag_string || (li).tag == tag_symbole), (Strv){ .arr = (li).str, .size = (li).size })
#define List_str_equal(li1, li2) Strv_equal(List_to_Strv(li1), List_to_Strv(li2))
#define List_equal_lit(li, lit) Strv_equal_lit(List_to_Strv(li), lit)


bool list(Strv *str, List *li);
bool lists(Strv str, List *li);
bool dump(Strb *out, const List li);
bool List_print(const List li);
bool eval(Lisp_context *ctx, const List li, List *out);
bool List_equal(const List li1, const List li2);
void List_free(List *li);
List List_copy(Lisp_context *ctx, const List li);
Lisp_context Lisp_context_init(List root);
void Lisp_context_free(Lisp_context *ctx);

void *List_alloc(Lisp_context *ctx, size_t count);
void *List_delc_alloc(Lisp_context *ctx, void *ptr, size_t count);
void *List_duplicate(Lisp_context *ctx, void *src, size_t count);
bool garbage_collector(Lisp_context *ctx);


#include "memory.h"

#endif /* LISP_H */
