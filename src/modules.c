#include "modules.h"
#include "system.h"

#include <h/amul.alog.h>
#include <h/amul.test.h>
#include <h/amul.type.h>

#include <h/amigastubs.h>

extern error_t InitCommandLine(const struct CommandLine *);

static struct Module *s_modulesHead;
static struct Module *s_modulesTail;

// If there's ever a memory management module, it won't want to have the
// module system first try and allocate memory for a module... So there
// is a pool of static modules.

enum { NUM_STATIC_MODULES = 4 };
static struct Module s_staticModules[4];

static struct Module *s_freeModules;

static const char *moduleNames[MAX_MODULE_ID] = {
        // hard-coded names for modules
        "INVALID",
        "cmdline",
        "runtime",
        "compiler",
};

void
InitModules()
{
    s_freeModules = &s_staticModules[0];
    for (size_t i = 1; i < NUM_STATIC_MODULES; ++i) {
        s_freeModules[i - 1].links.next = (struct DoubleLinkedNode *)&s_freeModules[i];
    }
}

error_t
StartModules()
{
    for (struct Module *cur = s_modulesHead; cur; cur = (struct Module *)cur->links.next) {
        if (cur->start) {
            alog(AL_DEBUG, "Starting Module #%d: %s", cur->id, cur->name);
            error_t err = cur->start(cur);
            if (err != 0)
                return err;
        }
    }
    return 0;
}

void
CloseModules(error_t err)
{
    struct Module *prev = NULL;
    for (struct Module *cur = s_modulesTail; cur; cur = prev) {
        alog(AL_DEBUG, "Closing Module #%d: %s", cur->id, cur->name);
        prev = (struct Module *)cur->links.prev;
        error_t reterr = CloseModule(cur, err);
        if (reterr != 0) {
            fprintf(stderr, "*** INTERNAL ERROR: Module %s failed to terminate with %d\n",
                    cur->name, reterr);
        }
    }
}

error_t
NewModule(
        bool useStatic, enum ModuleID id, moduleinit_fn init /*opt*/, modulestart_fn start /*opt*/,
        moduleclose_fn close /*opt*/, void *context /*opt*/, struct Module **ptr /*opt*/)
{
    REQUIRE(id && (context || (init || start || close)));
    REQUIRE(id < MAX_MODULE_ID);

    if (GetModule(id) != NULL) {
        alog(AL_DEBUG, "Tried to register duplicate module#%d: %s", id, moduleNames[id]);
        return EEXIST;
    }

    struct Module *instance = NULL;
    struct Module *cur = s_freeModules;
    for (; cur; cur = (struct Module *)cur->links.next) {
        if (cur->allocd == !useStatic)
            break;
        instance = cur;
    }

    if (cur) {
        // cut me out of the list
        if (!instance) {  // first entry on the list
            assert(cur == s_freeModules);
            s_freeModules = (struct Module *)cur->links.next;
        } else {
            instance->links.next = cur->links.next;
        }
    } else if (useStatic) {
        alog(AL_FATAL, "Cannot provision static module #%d: %s", id, moduleNames[id]);
    } else {
        cur = (struct Module *)AllocateMem(sizeof(struct Module));
    }

    if (cur == NULL) {
        alog(AL_FATAL, "Out of memory");
        return ENOMEM;
    }

    // populate values
    cur->id = id;
    cur->name = moduleNames[id];
    cur->links.next = NULL;
    cur->links.prev = (struct DoubleLinkedNode *)s_modulesTail;
    cur->init = init;
    cur->start = start;
    cur->close = close;
    cur->context = context;
    cur->allocd = !useStatic;

    if (!s_modulesHead) {
        s_modulesHead = cur;
        s_modulesTail = cur;
    } else {
        s_modulesTail->links.next = (struct DoubleLinkedNode *)cur;
    }

    error_t err = 0;
    if (cur->init) {
        err = cur->init(cur);
        if (err != 0) {
            alog(AL_FATAL, "Module #%d: %s: initialization failed: %d", id, cur->name, err);
        }
    }
    if (ptr)
        *ptr = cur;

    return 0;
}

struct Module *
GetModule(enum ModuleID id)
{
    assert(id);
    for (struct Module *cur = s_modulesHead; cur; cur = (struct Module *)cur->links.next) {
        if (id == cur->id)
            return cur;
    }
    return NULL;
}

error_t
CloseModule(struct Module *module, error_t err)
{
    REQUIRE(module);
    error_t reterr = 0;
    if (module->close)
        reterr = module->close(module, err);

    if (module->links.prev)
        module->links.prev->next = &module->links;
    if (module->links.next)
        module->links.next->prev = &module->links;
    if (s_modulesHead == module)
        s_modulesHead = (struct Module *)module->links.next;
    if (s_modulesTail == module)
        s_modulesTail = (struct Module *)module->links.prev;

    memset(&module, 0, sizeof(module));

    if (module->allocd)
        ReleaseMem((void**)&module);
    else {
        // Put me at the front of the free list
        module->links.next = s_freeModules->links.next;
        s_freeModules = module;
    }

    return reterr;
}

error_t
RegisterContextModule(enum ModuleID id, void *context)
{
    return NewModule(false, id, NULL, NULL, NULL, context, NULL);
}
