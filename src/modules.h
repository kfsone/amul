#ifndef AMUL_SRC_MODULES_H
#define AMUL_SRC_MODULES_H

#include <h/amul.list.h>
#include <h/amul.type.h>

struct Module;
typedef error_t (*moduleinit_fn)(struct Module *);
typedef error_t (*modulestart_fn)(struct Module *);
typedef error_t (*moduleclose_fn)(struct Module *, error_t);

enum ModuleID {
    MOD_LOGGING = 1,
    MOD_CMDLINE,
    MOD_STRINGS,
    MOD_COMPILER,

    MAX_MODULE_ID,
};

struct Module {
    DoubleLinkedNode links;

    ModuleID 	  id;
    bool          allocd;  // false if this is a static module object

    moduleinit_fn  init;
    modulestart_fn start;
    moduleclose_fn close;

    const char *name;
    void *      context;
};

void    InitModules();
error_t StartModules();
void    CloseModules(error_t err);

error_t NewModule(
        bool useStatic, enum ModuleID id, moduleinit_fn init, modulestart_fn start,
        moduleclose_fn close, void *context /*opt*/, Module **ptr /*opt*/);
Module  *GetModule(enum ModuleID id);
error_t CloseModule(Module **module, error_t err);

extern error_t RegisterContextModule(enum ModuleID id, void *context);
#endif
