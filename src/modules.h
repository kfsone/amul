#ifndef AMUL_SRC_MODULES_H
#define AMUL_SRC_MODULES_H

#include <h/amul.list.h>
#include <h/amul.type.h>

typedef error_t (*moduleinit_fn)(struct Module *);
typedef error_t (*modulestart_fn)(struct Module *);
typedef error_t (*moduleclose_fn)(struct Module *, error_t);

enum ModuleID {
    MOD_LOGGING = 1,
    MOD_CMDLINE,
    MOD_RUNTIME,
    MOD_COMPILER,

    MAX_MODULE_ID,
};

struct Module {
    struct DoubleLinkedNode links;

    enum ModuleIDs id;
    const char *   name;

    moduleinit_fn  init;
    modulestart_fn start;
    moduleclose_fn close;

    void *context;

    bool allocd;  // false if this is a static module object
};

struct CommandLine {
    int          argc;
    const char **argv;
    const char **envp;
};

void           InitModules(const struct CommandLine *cmdline);
error_t        StartModules();
void           CloseModules(error_t err);

error_t NewModule(
        bool useStatic, enum ModuleID id, moduleinit_fn init, modulestart_fn start,
        moduleclose_fn close, void *context /*opt*/, struct Module **ptr /*opt*/);
struct Module *GetModule(enum ModuleID id);
error_t        CloseModule(struct Module *module, error_t err);

extern error_t RegisterContextModule(enum ModuleID id, void *context);
#endif