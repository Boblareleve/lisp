/* 

#include <dlfcn.h>
// #include <ffi.h>
#include "lisp.h"




List load_dl(const List path)
{
    if (path.tag != tag_string)
    {
        error_log("try to load a dl without a string got %s", tag_to_string(path.tag));
        return NIL_LIST;
    }
    char buffer[512];
    buffer[0] = 0;
    
    void *res = dlopen(
        strncpy(buffer, path.str, MIN(sizeof(buffer), path.size)),
        RTLD_LAZY   // lazy load of symbole
      | RTLD_GLOBAL // symboles are available for later .so open
    );
    if (!res)
    {
        error_log("dl open \"%s\": %s", buffer, dlerror());
        return NIL_LIST;
    }

    return (List){
        .tag = tag_dynamic_lib,
        .ptr = res
    };
}


ffi_type *List_type_to_ffi_type(const List type)
{
    assert(type.tag == tag_type);
    switch (type.type_tag)
    {
    case ttag_any_type: return &ffi_type_void;
    case tag_integer:   return &ffi_type_sint64;
    case tag_real:      return &ffi_type_double;
    // case tag_string:    return ;
    
    default: UNREACHABLE("List_type_to_ffi_type"); return NULL;
    }
}


// const List desc = (args1_t args2_t... return_t)
bool get_fun_dl(List lib, List *out, const List name, const List desc)
{
    TRY(name.tag == tag_string, error_log("name is not a string, got %s", tag_to_string(name.tag)));
    TRY(lib.tag == tag_dynamic_lib, error_log("lib is not a dl, got %s", tag_to_string(lib.tag)));

    printf("name '%.*s'\n", name.size, name.str);

    char buffer[1024];
    buffer[0] = 0;

    (void)dlerror(); // flush previous errors
    void *fun_ptr = dlsym(lib.ptr, strncpy(buffer, name.str, MIN(sizeof(buffer), name.size)));
    const char *err = dlerror();
    TRY(fun_ptr && !err, error_log(err));
    
    Foreign_fun *ffun = List_alloc(sizeof(Foreign_fun) + sizeof(ffi_type) * (desc.size-1));
    ffun->args_size = desc.size-1;

    for (int i = 0; i < desc.size-1; i++)
    {
        TRY(desc.list[i].tag == tag_type, free(ffun); error_log("invalid get_fun_dl description"));
        ffun->args[i] = List_type_to_ffi_type(desc.list[i]);
    }

    TRY(desc.list[desc.size-1].tag == tag_type, free(ffun); error_log("invalid get_fun_dl description (return type)"));
    ffi_status status = ffi_prep_cif(&ffun->cif,
        FFI_DEFAULT_ABI,
        ffun->args_size,
        List_type_to_ffi_type(desc.list[desc.size-1]),
        ffun->args
    );
    assert(status == FFI_OK);

    *out = (List){
        .tag = tag_foreign_function,
        .ffun = ffun
    };
    return true;
    
    if (0)
    {
        void *handle = dlopen("mylib.so", RTLD_LAZY);
        void *sym    = dlsym(handle, "add");

        // 1. Describe argument types at runtime
        ffi_cif cif;
        ffi_type *arg_types[2] = { &ffi_type_sint32, &ffi_type_sint32 };
        
        ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &ffi_type_sint32, arg_types);

        // 2. Set up argument values (pointers to values)
        int a = 10, b = 20, result;
        void *args[2] = { &a, &b };

        // 3. Call!
        ffi_call(&cif, FFI_FN(sym), &result, args);

        printf("result = %d\n", result); // 30
    }

    return true;
    // (List){
        // .tag = tag_foreign_function,
        // .fun = fun_ptr
    // }; 
}

bool unload_dl(const List dl)
{
    TRY(dl.tag == tag_dynamic_lib, error_log("try to unload not a dl got %s", tag_to_string(dl.tag)));

    TRY(!dlclose(dl.ptr), error_log(dlerror()));

    return true;
}
 */