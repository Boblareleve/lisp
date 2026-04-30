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

#define GC_ALLWAYS_TRIGGER
void trigger_gc(void)
{
    if (g_ctx->euristics.paused) return;

#ifdef GC_ALLWAYS_TRIGGER
    garbage_collector();
#else
    if (clock() - g_ctx->euristics.last_clean_up >= g_ctx->euristics.max_clean_up_quantum
     || g_ctx->euristics.allocs_count            >= g_ctx->euristics.max_clean_up_allocs_count)
        garbage_collector();
#endif
}

void *List_alloc(size_t count)
{
    void *mem = calloc(count, 1);
    assert(mem && ((uintptr_t)mem & 0b1) == 0); // make sure it can be taged
    if (!g_ctx) // if no context -> allocation in the parsing phase the allocation will be register only on program startup
        return mem;
    
    trigger_gc();

    set_void_ptr_insert(&g_ctx->gc, mem);
    g_ctx->euristics.allocs_count++;
    return mem;
}

void *List_delc_alloc(void *ptr, size_t count)
{
    UNUSED(count);
    assert(((uintptr_t)ptr & 0b1) == 0);
    
    if (!g_ctx)
        return ptr;
    
    trigger_gc();

    set_void_ptr_insert(&g_ctx->gc, ptr);
    g_ctx->euristics.allocs_count++;
    return ptr;
}

void *List_duplicate(const void *src, size_t count)
{
    void *new = List_alloc(count);
    if (!new)
        return NULL;
    return memcpy(new, src, count);
}




#define GC_TAG 0x1
bool is_gc_tag(void *ptr)   { return ((GC_TAG & (uintptr_t)ptr) == GC_TAG);  }
void *gc_tag(void *ptr)     { return    (void*)((uintptr_t)ptr  |  GC_TAG);  }
void *gc_untag(void *ptr)   { return    (void*)((uintptr_t)ptr  & ~GC_TAG);  }



void gc_traverse_mark(List li)
{
    {
        void *ptr = List_get_ptr(li);
        if (!ptr) return; // if not something allocated return
        
        void **f = set_void_ptr_get(&g_ctx->gc, ptr);
        if (!f)
        {
#ifdef FSAN
            if (!set_void_ptr_contains(&g_ctx->gc, gc_tag(ptr)))
            {
                // intentional double free to trigger fsan
                // char dumb = *(char*)ptr;
                // free(ptr);
                // dumb = *(char*)ptr;
                // (void)dumb;
            }
#endif
            assert(set_void_ptr_contains(&g_ctx->gc, gc_tag(ptr))); // check if the value was already poisoned if not the allocation wasn't reported as it should
            return; // if it was not found -> already poisoned
        }
        
        *f = gc_tag(*f);
    }
    
    if ((
        li.tag == ttag_any_type 
     && li.type_tag == tag_list 
     && li.size != TYPE_UNDEFINED_LIST_SIZE
    )
     || li.tag == tag_list)
        for (int i = 0; i < li.size; i++)
            gc_traverse_mark(li.list[i]);
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


void gc_tag_context(void)
{
    // traverse code
    gc_traverse_mark(g_ctx->root);

    // traverse vm stack
    da_for (List, it, &g_ctx->vm_stack)
        gc_traverse_mark(*it);
    
    // traverse stack ("named")
    da_for (Variable, it, &g_ctx->stack)
    {
        gc_traverse_mark(it->name);
        gc_traverse_mark(it->value);
    }

    // traverse globale variables
    set_for (Variable, it, &g_ctx->variables)
    {
        gc_traverse_mark(it->name);
        gc_traverse_mark(it->value);
    }
}

bool garbage_collector(void)
{
    TRY(g_ctx);

    
    gc_tag_context();
    
    
#ifdef GC_REPORT
    size_t pointers_count = g_ctx->gc.size;
#endif
    
    erase_untag(&g_ctx->gc);
    
    set_for (void_ptr, it, &g_ctx->gc)
        *it = gc_untag(*it);
    
#ifdef GC_REPORT
    printf("gc stats: %ld freed for %ld chunks (%.2lf%%) ", 
        pointers_count - g_ctx->gc.size, pointers_count, 
        (1.0 - (double)g_ctx->gc.size / pointers_count) * 100.0
    );
#endif
    
    g_ctx->euristics.allocs_count = 0;
    g_ctx->euristics.last_clean_up = clock();
    
    return true;
}
