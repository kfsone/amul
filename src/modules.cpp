#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "modules.h"
#include "system.h"

#include <h/amul.alog.h>
#include <h/amul.test.h>
#include <h/amul.type.h>

#include <h/amigastubs.h>

Module *s_modulesHead;
Module *s_modulesTail;

bool s_modulesInitialized;
bool s_modulesClosed;

static const char *moduleNames[MAX_MODULE_ID] = {
        // hard-coded names for modules
        "INVALID", "logging", "cmdline", "strings", "compiler",
};

void
Terminate(error_t err)
{
    CloseModules(err);
    exit(err);
}

void
InitModules()
{
    if (s_modulesInitialized == true)
        abort();
    s_modulesInitialized = true;
}

error_t
StartModules()
{
    for (Module *cur = s_modulesHead; cur; cur = (Module *)cur->links.next) {
        if (cur->start) {
            alog(AL_DEBUG, "Starting Module #%d: %s", cur->id, cur->name);
            error_t err = cur->start(cur);
            if (err != 0) {
                alog(AL_ERROR, "Module initialization failed: aborting.");
                DEBUG_BREAK;
                Terminate(err);
            }
        }
    }
    return 0;
}

void
CloseModules(error_t err)
{
    Module *prev = nullptr;
    for (Module *cur = s_modulesTail; cur; cur = prev) {
        alog(AL_DEBUG, "Closing Module #%d: %s", cur->id, cur->name);
        prev = (Module *)cur->links.prev;
        error_t reterr = CloseModule(cur, err);
        if (reterr != 0) {
            fprintf(stderr, "*** INTERNAL ERROR: Module %s failed to terminate with %d\n",
                    cur->name, reterr);
        }
    }

    s_modulesClosed = true;
}

error_t
NewModule(
        ModuleID id, moduleinit_fn init /*opt*/, modulestart_fn start /*opt*/,
        moduleclose_fn close /*opt*/, void *context /*opt*/, Module **ptr /*opt*/)
{
    REQUIRE(id && id < MAX_MODULE_ID);
    REQUIRE(context || (init || start || close));

    if (GetModule(id) != NULL) {
        alog(AL_DEBUG, "Tried to register duplicate module#%d: %s", id, moduleNames[id]);
        return EEXIST;
    }

    Module *cur = (Module *)AllocateMem(sizeof(Module));
    if (cur == NULL) {
		afatal("Out of memory");
        return ENOMEM;
    }

    // populate values
    cur->id = id;
    cur->name = moduleNames[id];
    cur->links.next = NULL;
    cur->links.prev = (DoubleLinkedNode *)s_modulesTail;
    cur->init = init;
    cur->start = start;
    cur->close = close;
    cur->context = context;

    if (!s_modulesHead) {
        s_modulesHead = cur;
        s_modulesTail = cur;
    } else {
        s_modulesTail->links.next = (DoubleLinkedNode *)cur;
        cur->links.prev = (DoubleLinkedNode *)s_modulesTail;
        s_modulesTail = cur;
    }

    if (cur->init) {
        error_t err = cur->init(cur);
        if (err != 0) {
			afatal("Module #%d: %s: initialization failed: %d", id, cur->name, err);
            return err;
        }
    }
    if (ptr)
        *ptr = cur;

    return 0;
}

Module *
GetModule(ModuleID id)
{
    for (Module *cur = s_modulesHead; cur; cur = (Module *)cur->links.next) {
        if (id == cur->id)
            return cur;
    }
    return NULL;
}

error_t
CloseModule(Module *module, error_t err)
{
    REQUIRE(module);

    // Make sure this is a registered module
    Module *cur = s_modulesHead;
    while (cur && cur != module)
        cur = (Module *)cur->links.next;
    if (cur != module)
        return EFAULT;

    error_t reterr = 0;
    if (module->close)
        reterr = module->close(module, err);

    if (module->links.prev)
        module->links.prev->next = module->links.next;
    if (module->links.next)
        module->links.next->prev = module->links.prev;
    if (s_modulesHead == module)
        s_modulesHead = (Module *)module->links.next;
    if (s_modulesTail == module)
        s_modulesTail = (Module *)module->links.prev;

    memset(module, 0, sizeof(*module));

    ReleaseMem((void **)&module);

    return reterr;
}

error_t
RegisterContextModule(ModuleID id, void *context)
{
    return NewModule(id, NULL, NULL, NULL, context, NULL);
}
