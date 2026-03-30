#include "lisp.h"

// 00 00  00 00   00 00  00 00


int set_void_ptr_equal(const void_ptr a, const void_ptr b)
{
    return a == b;
}
uint64_t set_void_ptr_hash(const void_ptr key, uint64_t seed)
{ // 0b1000
    return ((uint64_t)key >> 3) ^ seed;
}

// 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
// 0 1 2 3 4 5 6 7 8 9 a b c d e f
// 0 1 10 11 100 101 110 111 1000

#define ISNULL_VPTR(ptr) ((ptr) == NULL)
#define SETNULL_VPTR(ptr) ((ptr) = NULL)


SET_IMPLEMENT_HASH_SET(void_ptr, ISNULL_VPTR, SETNULL_VPTR, 4, 0.8, 64);



void *List_alloc(Lisp_context *ctx, size_t count)
{
    void *mem = calloc(count, 1);
    assert(((uintptr_t)mem & 0b1) == 0);
    if (!ctx) // if no context -> allocation in the parsing phase the allocation will be register only on program startup
        return mem;
    
    
    set_void_ptr_insert(&ctx->gc, mem);

    return mem;
}

void *List_delc_alloc(Lisp_context *ctx, void *ptr, size_t count)
{
    UNUSED(count);
    
    assert(((uintptr_t)ptr & 0b1) == 0);

    if (!ctx)
        return ptr;

    set_void_ptr_insert(&ctx->gc, ptr);
    return ptr;
}

void *List_duplicate(Lisp_context *ctx, void *src, size_t count)
{
    void *new = List_alloc(ctx, count);
    return memcpy(new, src, count);
}




#define GC_TAG 0x1
bool is_gc_tag(void *ptr)
{
    return ((uintptr_t)ptr & GC_TAG) == GC_TAG;
}
void *gc_tag(void *ptr)
{
    return (void*)((uintptr_t)ptr | GC_TAG);
}
void *gc_untag(void *ptr)
{
    return (void*)((uintptr_t)ptr & ~GC_TAG);
}




int void_ptr_cmp(const void *a, const void *b)
{
    const void * const *pa = a;
    const void * const *pb = b;

    return *pa - *pb;
}

void gc_traverse_mark(Lisp_context *ctx, List li)
{
    {
        void *ptr = List_get_ptr(&li);
        if (!ptr) return; // if not something allocated return
        
        void **f = set_void_ptr_get(&ctx->gc, ptr);
        if (!f)
        {
            assert(set_void_ptr_contains(&ctx->gc, gc_tag(ptr))); // check if the value was already poisoned if not the allocation wasn't reported as it should
            return; // if it was not found -> already poisoned
        }
        
        *f = gc_tag(*f);
    }

    if (li.tag == tag_list)
        for (int i = 0; i < li.size; i++)
            gc_traverse_mark(ctx, li.list[i]);
}



// bool set_##TK##_erase(set_##TK *obj, TK val)
// {
//     const int index_erase = _set_##TK##_where(obj, val);
//     if (IS_NULL(obj->arr[index_erase]))
//         return false;
//     SET_NULL(obj->arr[index_erase]);
//     obj->size--;
//     
//     int it = (index_erase + 1) % obj->capacity;
//     while (!IS_NULL(obj->arr[it]))
//     { /* if it is already at the good place there still could be miss place element after and reinsert it is trivial */
//         int hash = _set_##TK##_where(obj, obj->arr[it]);
//         if (it != hash)
//         {
//             obj->arr[hash] = obj->arr[it];
//             SET_NULL(obj->arr[it]);
//         }
//         it = (it + 1) % obj->capacity;
//     }
//     return true;
// }
void erase_untag(set_void_ptr *gc)
{
    for (int i = 0; i < gc->capacity; i++)
        if (gc->arr[i] && !is_gc_tag(gc->arr[i]))
        {
            free(gc->arr[i]);
            gc->arr[i] = NULL;
            gc->size--;
            
            i = (i + 1) % gc->capacity;
            while (gc->arr[i])
            {
                if (!is_gc_tag(gc->arr[i]))
                {
                    free(gc->arr[i]);
                    gc->arr[i] = NULL;
                    i = (i + 1) % gc->capacity;
                    gc->size--;
                    continue;
                }
                //
                int hash = _set_void_ptr_where(gc, gc->arr[i]);
                if (i != hash)
                {
                    gc->arr[hash] = gc->arr[i];
                    gc->arr[i] = NULL;
                }
                i = (i + 1) % gc->capacity;
            }
        }
}
// void set_void_ptr_filter(set_void_ptr *gc, bool (*filter)(void_ptr), void *ctx, bool (*callback)(void *ctx, void_ptr))
// {
//     for (int i = 0; i < gc->capacity; i++)
//         if (IS_NULL(gc->arr[i]) && !filter(gc->arr[i]))
//         {
//             callback(ctx, gc->arr[i]);
//             SET_NULL(gc->arr[i]);
//             gc->size--;
//             i = (i + 1) % gc->capacity;
//             while (IS_NULL(gc->arr[i]))
//             {
//                 if (!filter(gc->arr[i]))
//                 {
//                     callback(ctx, gc->arr[i]);
//                     SET_NULL(gc->arr[i]);
//                     i = (i + 1) % gc->capacity;
//                     gc->size--;
//                     continue;
//                 }
//                 int hash = _set_void_ptr_where(gc, gc->arr[i]);
//                 if (i != hash)
//                 {
//                     gc->arr[hash] = gc->arr[i];
//                     SET_NULL(gc->arr[i]);
//                 }
//                 i = (i + 1) % gc->capacity;
//             }
//         }
// }


void gc_tag_context(Lisp_context *ctx)
{
    // traverse code
    gc_traverse_mark(ctx, ctx->root);

    // traverse stack
    da_for (da_Variable, it, &ctx->args_stack)
    {
        da_for (Variable, jt, it)
        {
            gc_traverse_mark(ctx, jt->name);
            gc_traverse_mark(ctx, jt->value);
        }
    }

    // traverse globale variables
    set_for (Variable, it, &ctx->variables)
    {
        gc_traverse_mark(ctx, it->name);
        gc_traverse_mark(ctx, it->value);
    }

    // traverse functions
    set_for (Variable, it, &ctx->functions)
    {
        gc_traverse_mark(ctx, it->name);
        gc_traverse_mark(ctx, it->value);
    }
}

bool garbage_collector(Lisp_context *ctx)
{
    TRY(ctx);
    
    
    gc_tag_context(ctx);

    size_t pointers_count = ctx->gc.size;
    
    erase_untag(&ctx->gc);

    set_for (void_ptr, it, &ctx->gc)
        *it = gc_untag(*it);

#ifdef GC_REPORT
    printf("gc stats: %ld freed for %ld chunks (%lf%%) ", 
        pointers_count - ctx->gc.size, pointers_count, 
        (1.0 - (double)ctx->gc.size / pointers_count) * 100.0
    );
#else
    (void)pointers_count;
#endif

    return true;
}
