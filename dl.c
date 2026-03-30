#include <dlfcn.h>
#include <ffi.h>
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

List get_fun_dl(List lib, const List name, const List desc)
{
    UNUSED(desc);
    if (name.tag != tag_string)
    {
        error_log("name is not a string, got %s", tag_to_string(name.tag));
        return NIL_LIST;
    }
    if (lib.tag != tag_dynamic_lib)
    {
        error_log("lib is not a dl, got %s", tag_to_string(lib.tag));
        return NIL_LIST;
    }

    printf("name '%.*s'\n", name.size, name.str);

    char buffer[512];
    buffer[0] = 0;

    (void)dlerror(); // flush potencial previous errors
    void *fun_ptr = dlsym(lib.ptr, strncpy(buffer, name.str, MIN(sizeof(buffer), name.size)));
    const char *err = dlerror();
    if (!fun_ptr || err)
    {
        error_log(err);
        return NIL_LIST;
    }
    
    
    
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

    return NIL_LIST;/* (List){
        .tag = tag_foreign_function,
        .fun = fun_ptr
    }; */
}

bool unload_dl(const List dl)
{
    TRY(dl.tag == tag_dynamic_lib, error_log("try to unload not a dl got %s", tag_to_string(dl.tag)));

    TRY(!dlclose(dl.ptr), error_log(dlerror()));

    return true;
}
