#include "lisp.h"


bool _dump_indent(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_list: {
        if (li.size == 0)
        {
            Strb_cat(out, "()");
            break;
        }
        Strb_catf(out, "(\n", li.size);
        for (size_t i = 0; i < li.size; i++)
        {
            TRY(_dump_indent(out, li.list[i], indent + 2));
            Strb_cat(out, "\n");
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")\n");
    } break;
    case tag_reference: {
        Strb_cat(out, "*");
        TRY(_dump_indent(out, *li.list, indent));
    } break;
    case tag_real:      Strb_catf(out, "%.0f64", li.real);         break;
    case tag_integer:   Strb_catf(out, "%d64",   li.integer);      break;
    case tag_symbole:   Strb_catf(out, "%.*s",   li.size, li.str); break;
    case tag_string:    Strb_catf(out, "\"%.*s\"",   li.size, li.str); break;
    case tag_true:      Strb_cat(out, "true");                     break;
    case tag_dynamic_lib:
    case tag_void_ptr:
                        Strb_cat(out, li.ptr ? "<HANDLE>" : "(nil)"); break;
    default:            Strb_cat(out, "UNKOWN");                   break;
    }
    return true;
}
bool _dump_type_indent(Strb *out, const List li, int indent)
{
    TRY(out, error_log("no output Strb"));

    Strb_cat_nchar(out, indent, ' ');
    switch (li.tag)
    {
    case tag_symbole: {
        Strb_catf(out, "symbole: '%.*s'", li.size, li.str);
    } break;
    case tag_list: {
        if (li.size == 0)
        {
            Strb_cat(out, "()");
            break;
        }
        Strb_catf(out, "(list {%d}:\n", li.size);
        for (size_t i = 0; i < li.size; i++) // const List, it, &li.list)
        {
            TRY(_dump_type_indent(out, li.list[i], indent + 2));
            Strb_cat(out, "\n");
        }
        Strb_cat_nchar(out, indent, ' ');
        Strb_cat(out, ")");
    } break;
    case tag_string:    Strb_catf(out, "string: \"%.*s\"", li.size, li.str); break;
    case tag_real:      Strb_catf(out, "real: %.0f64",   li.real);      break;
    case tag_integer:   Strb_catf(out, "interger: %d64", li.integer);   break;
    case tag_true:      Strb_cat(out, "true");                          break;
    case tag_dynamic_lib:
    case tag_void_ptr:
                        Strb_cat(out, li.ptr ? "<HANDLE>" : "handle: (nil)"); break;
    default: Strb_cat(out, "UNKOWN"); break;
    }
    return true;
}

bool dump(Strb *out, const List li)
{
    TRY(out, error_log("no output Strb"));

    switch (li.tag)
    {
    case tag_list: {    
        Strb_cat(out, "(");
        for (size_t i = 0; i < li.size; i++)
        {
            TRY(dump(out, li.list[i]));
            if (i+1 == li.size)
                Strb_cat_char(out, ' ');
        }
        Strb_cat(out, ")");
    } break;
    case tag_real:        Strb_catf(out, "%.0f64",   li.real);          break;
    case tag_integer:     Strb_catf(out, "%d64", li.integer);           break;
    case tag_symbole:     Strb_catf(out, "%.*s", li.size, li.str);      break;
    case tag_string:      Strb_catf(out, "%.*s", li.size, li.str);      break;
    case tag_true:        Strb_cat(out, "true");                        break;
    case tag_dynamic_lib: 
    case tag_void_ptr:
                          Strb_cat(out, li.ptr ? "<HANDLE>" : "(nil)"); break;
    default:              Strb_cat(out, "UNKOWN");                      break;
    }
    return true;
}
bool dump_indent(Strb *out, const List li)
{
    return _dump_indent(out, li, 0);
}
bool List_print(const List li)
{
    static Strb to_print = {0};
    to_print.size = 0;

    TRY(dump_indent(&to_print, li));
    fprintf(stdout, STRV_FMT, STRV_UNPACK(to_print.view));
#ifdef DEBUG
    fflush(stdout);
#endif
    return true;
}

