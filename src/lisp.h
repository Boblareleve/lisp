#ifndef LISP_H
#define LISP_H

// #define LISP_DEBUG_INFO

#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
// #include <setjmp.h>
#include <ffi.h>

#define AR_MAX_ALIGN 8
#include "ar.h"


typedef enum List_tag : uint8_t
{
    tag_list = 0,  // (a a a)|()
    tag_true,      // t
    tag_symbole,   // 
    tag_string,    // "dslmjkfdsqml"
    tag_integer,   // 4326324
    tag_real,      // 3.3
    tag_type,      // int...
    ttag_any_type, // can only be use in type_tag fild of List 
    tag_void_ptr,  // C struct handel
    tag_dynamic_lib,
    tag_foreign_function, // Foreign_fun
} List_tag;


typedef struct
{
    ffi_cif cif;
    size_t args_size;
    ffi_type *args[0];
} Foreign_fun;

// typedef struct Foreign_fun
// {
//     ffi_cif ffi;
// } Foreign_fun;


#define NIL_LIST (List){0}
#define TRUE_LIST (List){ .tag = tag_true }
#define IS_NIL(li) ((li).tag == tag_list && (li).size == 0)
#define ANY_TYPE ((List){ .tag = tag_type, .type_tag = ttag_any_type })
#define TYPE_TYPE ((List){ .tag = tag_type, .type_tag = tag_type })

#define TYPE_UNDEFINED_LIST_SIZE UINT16_MAX


typedef struct List List;


// maybe get down to 8 bytes using uint32_t for indexing into a pool
struct List
{
    List_tag tag;
    uint8_t quote_count; // how many reference "(QUOTE self)" depth it is
    List_tag type_tag;   // for type only

    uint16_t offset; // only to get back the start of the allocated chunk
    uint16_t size;

    union {
        List *list;
        char *str;
        double real;
        int64_t integer;
        void *ptr;
        Foreign_fun *ffun;
    };
};
static_assert(sizeof(List) == 16);



typedef struct Variable
{
    List name;
    List type;
    List value;
} Variable;
DA_TYPEDEF_ARRAY(Variable);
DA_TYPEDEF_ARRAY(da_Variable);
SET_TYPEDEF_HASH_SET(Variable); // global variable and functions


typedef void *void_ptr;
SET_TYPEDEF_HASH_SET(void_ptr); // gc

// DA_TYPEDEF_ARRAY_PTR(void);
typedef struct Lisp_context
{
    // globals
    set_Variable variables; 
    // set_Variable functions;
    // set_Variable types;

    // stack (local)
    da_da_Variable args_stack;
    bool in_return; // indicate that the error is only a return mechanism

    List root;

    set_void_ptr gc;
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
        [tag_true]             = "tag_true",
        [tag_symbole]          = "tag_symbole",
        [tag_string]           = "tag_string",
        [tag_integer]          = "tag_integer",
        [tag_real]             = "tag_real",
        [tag_list]             = "tag_list",
        [tag_dynamic_lib]      = "tag_dynamic_lib",
        [tag_void_ptr]         = "tag_void_ptr",
        [tag_foreign_function] = "tag_foreign_function",
        [tag_type]             = "tag_type",
    };
    return table[tag];
}
static inline void *List_get_ptr(const List *li)
{
    assert(li->tag != tag_foreign_function || li->offset == 0); // tag_foreign_function -> .offset == 0
    if (li->tag == tag_list || li->tag == tag_foreign_function)
        return (void*)(li->list - (uintptr_t)li->offset);
    if (li->tag == tag_symbole || li->tag == tag_string)
        return (void*)(li->str - (uintptr_t)li->offset);
    
    return NULL;
}

#define List_to_Strv(li) (assert((li).tag == tag_string || (li).tag == tag_symbole), (Strv){ .arr = (li).str, .size = (li).size })
#define _cstr_to_List(cstr) (List){ .tag = tag_string, .size = STRING_LEN(cstr), .str = cstr }
#define List_str_equal(li1, li2) Strv_equal(List_to_Strv(li1), List_to_Strv(li2))
#define List_equal_lit(li, lit) Strv_equal_lit(List_to_Strv(li), lit)

extern Lisp_context *g_ctx;


// parse.c
bool list(Strv *str, List *li);
bool lists(Strv str, List *li);

// dump.c
bool dump(Strb *out, const List li);
bool List_print(const List li);

// lisp.c
bool eval(const List li, List *out);
bool List_equal(const List li1, const List li2);
void List_free(List *li);
List List_copy(const List li);
Lisp_context *Lisp_context_init(List root);
void set_Lisp_context(Lisp_context *ctx);
void Lisp_context_free(void);

// memory.c
void *List_alloc(size_t count);
void *List_delc_alloc(void *ptr, size_t count);
void *List_duplicate(const void *src, size_t count);
bool garbage_collector(void);

// dl.c
List load_dl(const List path);
bool unload_dl(const List dl);
bool get_fun_dl(List lib, List *out, const List name, const List desc);

// type.c
bool have_function_shape(const List li);
bool is_of_type(const List li, const List type);
bool type_equal(const List a, const List b);
void add_simple_type(const char *name, List type);
bool type_compatible(const List a, const List b);



// eval_list.c
typedef bool (*primitive_t)(const List li, List *out);
void init_primitive_map(void);
bool test_get_Primitive(void);
primitive_t get_Primitive(const List op);


#endif /* LISP_H */
