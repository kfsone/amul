#ifndef AMUL_MODULES_H
#define AMUL_MODULES_H

#include "h/amul.list.h"
#include "h/amul.type.h"

struct Module;
using moduleinit_fn = error_t (*)(struct Module *);
using modulestart_fn = error_t (*)(struct Module *);
using moduleclose_fn = error_t (*)(struct Module *, error_t);

enum ModuleID {
    MOD_LOGGING = 1,
    MOD_CMDLINE,
    MOD_STRINGS,
    MOD_COMPILER,

    MAX_MODULE_ID,
};

struct Module {
    struct DoubleLinkedNode links;

    enum ModuleID id;

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
        enum ModuleID id, moduleinit_fn init, modulestart_fn start, moduleclose_fn close,
        void *context /*opt*/, struct Module **ptr /*opt*/);
struct Module *GetModule(enum ModuleID id);
error_t        CloseModule(struct Module *module, error_t err);

extern error_t RegisterContextModule(enum ModuleID id, void *context);
#endif
