#include "lisp.h"

// 00 00  00 00   00 00  00 00

void *List_alloc(Lisp_context *ctx, size_t count)
{
    void *mem = calloc(count, 1);
    assert(((uintptr_t)mem & 0b1) == 0);
    if (!ctx) // if no context -> allocation in the parsing phase the allocation will be register only on program startup
        return mem;
    
    
    da_push(&ctx->gc, mem);

    return mem;
}

void *List_delc_alloc(Lisp_context *ctx, void *ptr, size_t count)
{
    UNUSED(count);
    
    assert(((uintptr_t)ptr & 0b1) == 0);

    if (!ctx)
        return ptr;

    da_push(&ctx->gc, ptr);
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
        
        void **f = da_bsearch(&ctx->gc, &ptr, void_ptr_cmp);
        if (!f)
        {
            assert(da_bsearch(&ctx->gc, &(void*){gc_tag(ptr)}, void_ptr_cmp)); // check if the value was already poisoned if not the allocation wasn't reported as it should
            return; // if it was not found -> already poisoned
        }
        
        *f = gc_tag(*f);
    }

    if (li.tag == tag_list)
        for (int i = 0; i < li.size; i++)
            gc_traverse_mark(ctx, li.list[i]);
}




bool garbage_collector(Lisp_context *ctx)
{
    TRY(ctx);
    
    da_qsort(&ctx->gc, void_ptr_cmp);

    da_unique(&ctx->gc);

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

    // // free untaged pointers
    // for (int i = 0; i < ctx->gc.size; i++)
    // {
    //     if (!is_gc_tag(ctx->gc.arr[i]))
    //         free(ctx->gc.arr[i]);
    // }

    // // delete untaged pointers
    // int shift = da_filter(&ctx->gc, is_gc_tag);
    
    // // untag remaining pointers
    // da_for (void*, it, &ctx->gc)
    //     *it = gc_untag(*it);

    // free and delete untaged pointers and unmark the remaining
    int shift = 0;
    for (int i = 0; i+1 < ctx->gc.size; i++)
    {
        if (!is_gc_tag(ctx->gc.arr[i])) // && i + 1 != ctx->gc.size)
        {
            // printf("i: %d\n", i);
            // printf("-wow> %.*s\n", 3, (char*)ctx->gc.arr[i]);
            free(ctx->gc.arr[i]);
            shift++;
        }
        else
            ctx->gc.arr[i - shift] = gc_untag(ctx->gc.arr[i]);
    }

    if (ctx->gc.size > 0)
    {
        if (!is_gc_tag(da_top(&ctx->gc)))
            shift++;
        ctx->gc.arr[ctx->gc.size - 1 - shift] = gc_untag(da_top(&ctx->gc));
    }
    ctx->gc.size -= shift;
    

    printf("gc stats: %d freed for %d chunks (%lf%%)\n", 
        shift, (shift + ctx->gc.size), 
        (double)shift / (shift + ctx->gc.size) * 100.0
    );

    return true;
}
