#include "lisp.h"

// 00 00  00 00   00 00  00 00

void *List_alloc(Lisp_context *ctx, size_t count)
{
    if (!ctx)
        return calloc(count, 1); // if no context -> allocation in the parsing phase the allocation will be register only on program startup
    
    void *mem = calloc(count, 1);
    da_push(&ctx->gc, mem);

    return mem;
}

void *List_delc_alloc(Lisp_context *ctx, void *ptr, size_t count)
{
    UNUSED(count);
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
    return *(const void**)a - *(const void**)b;
}

void gc_traverse_mark(Lisp_context *ctx, List li)
{
    {
        void *ptr = List_get_ptr(&li);
        if (!ptr) return; // if not st allocated return
        
        void **f = da_bsearch(&ctx->gc, ptr, void_ptr_cmp);
        if (!f)
        {
            assert(da_bsearch(&ctx->gc, gc_tag(ptr), void_ptr_cmp)); // check if the value was already poisoned if not the allocation wasn't reported as it should
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

    gc_traverse_mark(ctx, ctx->root);


    int shift = 0;
    for (int i = 0; i < ctx->gc.size; i++)
    {
        if (is_gc_tag(ctx->gc.arr[i]))
        {
            ctx->gc.arr[i] = gc_untag(ctx->gc.arr[i]);
        }
        else if (i + 1 != ctx->gc.size)
        {
            shift++;
            ctx->gc.arr[i] = ctx->gc.arr[i + shift];
            ctx->gc.size--;
        }
    }
    printf("gc stats: %d freed for %d lists (%f%%)\n", 
        shift, (shift + ctx->gc.size), 
        (float)shift / (shift + ctx->gc.size)
    );

    return true;
}
