#ifndef LISP_H
#define LISP_H

// #define LISP_DEBUG_INFO

#include "Str.h"
#include "da.h"
#include "utils.h"
#include "sets.h"
#include <stdlib.h>
#include <ffi.h>

#define AR_MAX_ALIGN 8
#include "ar.h"


typedef enum List_tag : uint8_t
{
    tag_list = 0,  // (a a a)|()
    tag_symbole,   // 
    tag_true,      // t
    tag_string,    // "dslmjkfdsqml"
    tag_integer,   // 4326324
    tag_real,      // 3.3
    tag_type,      // int...
    tag_reference,
    tag_object,
    tag_void_ptr,  // C struct handel
    tag_dynamic_lib,
    tag_foreign_function, // Foreign_fun
    ttag_any_type, // can only be use in type_tag fild of List 
    ttag_frame, // to indicate the in a variable the value associated is a the index to the start of the previous frame
    ttag_macro, // to indicate start of a macro stack frame  
} List_tag;
static inline bool is_list_self_evaluating(List_tag tag)
{
    assert((tag == tag_real     || tag == tag_type
         || tag == tag_string   || tag == tag_true
         || tag == tag_integer)
        == (tag_true <= tag && tag <= tag_type)
    );
    return tag_true <= tag && tag <= tag_type;
}

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

#define TYPE_UNDEFINED_LIST_SIZE UINT16_MAX
#define TYPE_TYPE ((List){ .tag = tag_type, .type_tag = tag_type })
#define ANY_TYPE ((List){ .tag = tag_type, .type_tag = ttag_any_type })
#define LIST_TYPE (List){ .tag = tag_type, .type_tag = tag_list, .size = TYPE_UNDEFINED_LIST_SIZE }


typedef struct List List;
DA_TYPEDEF_ARRAY(List);

typedef struct Variable Variable;
DA_TYPEDEF_ARRAY(Variable);
DA_TYPEDEF_ARRAY(da_Variable);
SET_TYPEDEF_HASH_SET(Variable); // global variable and functions



// 

struct List
{
    List_tag tag;
    uint8_t quote_count; // how many reference "(QUOTE self)" depth it is
    List_tag type_tag;   // for type only
    uint8_t __pad[1];    // padding

    uint16_t offset; // only to get back the start of the allocated chunk
    uint16_t size;   // count of bytes in string/symbole or List in a list
    
    union {
        double real;
        int64_t integer;
        List *list;
        set_Variable *object;
        char *str;
        void *ptr;
        Foreign_fun *ffun;
    };
};
static_assert(sizeof(List) == 16, "List size is expected to be 16 bytes");


struct Variable
{
    List name;
    List type;
    List value;
};


typedef void *void_ptr;
SET_TYPEDEF_HASH_SET(void_ptr); // gc


typedef struct Lisp_context
{
    struct {
        // globals
        set_Variable variables; 
        // stack (local)
        da_Variable stack;
        int frame_start; // macro/function
        bool in_return; // indicate that the error is only a return mechanism
                        // see with vm_stack
        int macro_start;
        bool in_break;

        da_List vm_stack;
        set_void_ptr gc;
    };
    
    struct {
        clock_t max_clean_up_quantum;
        size_t max_clean_up_allocs_count;

        clock_t last_clean_up; 
        size_t allocs_count; // since last gc

        bool paused; // turn on in copy and parsing to avoid unwandted trigger
    } euristics;
    
    List root;

    da_List paths;

    Strb error;
} Lisp_context;



// VM

// lvalue
#define VM_top1 (*(assert(g_ctx->vm_stack.size >= 1), &g_ctx->vm_stack.arr[g_ctx->vm_stack.size-1]))
#define VM_top2 (*(assert(g_ctx->vm_stack.size >= 2), &g_ctx->vm_stack.arr[g_ctx->vm_stack.size-2]))
#define VM_top3 (*(assert(g_ctx->vm_stack.size >= 3), &g_ctx->vm_stack.arr[g_ctx->vm_stack.size-3]))
#define VM_top4 (*(assert(g_ctx->vm_stack.size >= 4), &g_ctx->vm_stack.arr[g_ctx->vm_stack.size-4]))


#define VM_push(...)\
({\
    __auto_type VM_tmp = __VA_ARGS__;\
    da_push(&g_ctx->vm_stack, VM_tmp);\
})
#define VM_pop (assert(g_ctx->vm_stack.size > 0), g_ctx->vm_stack.size--)



extern Strb static_error;
#define error_log(msg, ...)\
do {\
    Strb *_error = (g_ctx) ? &g_ctx->error : &static_error;\
    if (_error->size > 0) Strb_cat(_error, "\n");\
    Strb_catf(_error, "%s:%s:%d: ", __FILE__, __func__, __LINE__);\
    Strb_catf(_error, msg __VA_OPT__(,) __VA_ARGS__);\
} while (0)
#define get_error() (g_ctx ? g_ctx->error.view : (void)0)
#define reset_error() (g_ctx ? g_ctx->error.size = 0 : (void)0)

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
        [tag_reference]        = "tag_reference",
    };
    return table[tag];
}
static inline void *List_get_ptr(const List li)
{
    assert(li.tag != tag_foreign_function || li.offset == 0); // tag_foreign_function -> .offset == 0
    if (li.tag == tag_list
     || li.tag == tag_foreign_function
     || li.tag == tag_reference)
        return (void*)(li.list - (uintptr_t)li.offset);
    if (li.tag == tag_symbole || li.tag == tag_string)
        return (void*)(li.str - (uintptr_t)li.offset);
    
    return NULL;
}

#define List_to_Strv(li) (Strv){ \
    .arr = (assert((li).tag == tag_string || (li).tag == tag_symbole), (li).str), \
    .size = (li).size \
}

#define _cstr_to_List(cstr) (List){ .tag = tag_string, .size = STRING_LEN(cstr), .str = cstr }
#define _cstr_to_List_symbole(cstr) (List){ .tag = tag_symbole, .size = STRING_LEN(cstr), .str = cstr }
#define List_str_equal(li1, li2) Strv_equal(List_to_Strv(li1), List_to_Strv(li2))
#define List_equal_lit(li, lit) Strv_equal_lit(List_to_Strv(li), lit)
#define List_start_with_lit(li, lit) Strv_start_with_lit(List_to_Strv(li), lit)

extern Lisp_context *g_ctx;


// parse.c
bool list(Strv *str, List *li);
bool lists(Strv str, List *li);

// dump.c
bool dump(Strb *out, const List li);
bool List_print(const List li);

// lisp.c
bool eval(void); //const List li, List *out);
bool List_equal(const List li1, const List li2);
void List_free(List li);
List List_copy(const List li);
Lisp_context *Lisp_context_init(List root);
void set_Lisp_context(Lisp_context *ctx);
void Lisp_context_free(void);

// memory.c
void *List_alloc(size_t count);
void *List_delc_alloc(void *ptr, size_t count);
void *List_duplicate(const void *src, size_t count);
void trigger_gc(void);
bool garbage_collector(void);

// dl.c
List load_dl(const List path);
bool unload_dl(const List dl);
bool get_fun_dl(List lib, List *out, const List name, const List desc);

// type.c
bool have_function_arguments_shape(const List li);
bool is_of_type(const List li, const List type);
bool type_equal(const List a, const List b);
void add_primitive_type(const char *name, List type);
bool type_compatible(const List a, const List b);



// eval_list.c
typedef bool (*primitive_t)(void);
void init_primitive_map(void);
bool test_get_Primitive(void);
primitive_t get_Primitive(const List op);


#endif /* LISP_H */
