#include <src/filesystem.h>
#include <src/modules.h>
#include <h/amul.test.h>

extern struct Module *s_modulesHead;
extern struct Module *s_modulesTail;
extern struct Module  s_staticModules[4];
extern struct Module *s_freeModules;
extern bool           s_modulesInitialized;
extern bool           s_modulesClosed;

struct ModuleState {
    bool    init;
    bool    start;
    bool    close;
    error_t closeErr;
};

error_t
modInit(struct Module *module)
{
    ((struct ModuleState *)(module->context))->init = true;
    return 0;
}

error_t
modStart(struct Module *module)
{
    ((struct ModuleState *)(module->context))->start = true;
    return 0;
}

error_t
modClose(struct Module *module, error_t err)
{
    ((struct ModuleState *)(module->context))->close = true;
    ((struct ModuleState *)(module->context))->closeErr = err;
    return 0;
}

void
modules_test_tearUp(struct TestContext *t)
{
    *(struct ModuleState *)(t->userData) = (struct ModuleState){false, false, false};
}

void
test_init_modules(struct TestContext *t)
{
    EXPECT_NULL(s_freeModules);
    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_FALSE(s_modulesInitialized);
    EXPECT_FALSE(s_modulesClosed);

    InitModules();

    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_TRUE(s_modulesInitialized);
    EXPECT_FALSE(s_modulesClosed);

    // the free list is singly linked, only next should be populated
    EXPECT_PTR_EQUAL(s_freeModules, &s_staticModules[0]);
    EXPECT_NULL(s_freeModules->links.prev);
    EXPECT_PTR_EQUAL(s_freeModules->links.next, &s_staticModules[1]);
    EXPECT_NULL(s_freeModules->links.next->prev);
    EXPECT_PTR_EQUAL(s_freeModules->links.next->next, &s_staticModules[2]);
    EXPECT_NULL(s_freeModules->links.next->next->prev);
    EXPECT_PTR_EQUAL(s_freeModules->links.next->next->next, &s_staticModules[3]);
    EXPECT_NULL(s_freeModules->links.next->next->next->prev);
    EXPECT_NULL(s_freeModules->links.next->next->next->next);

    for (int i = 0; i < 4; ++i) {
        EXPECT_VAL_EQUAL(s_staticModules[i].id, 0);
        EXPECT_NULL(s_staticModules[i].init);
        EXPECT_NULL(s_staticModules[i].start);
        EXPECT_NULL(s_staticModules[i].close);
        EXPECT_NULL(s_staticModules[i].context);
        EXPECT_FALSE(s_staticModules[i].allocd);
    }
}

void
test_register_context_module(struct TestContext *t)
{
    EXPECT_ERROR(EINVAL, RegisterContextModule((enum ModuleID)0, NULL));
    EXPECT_ERROR(EINVAL, RegisterContextModule(MAX_MODULE_ID, NULL));

    // Context modules *must* have a context
    EXPECT_ERROR(EINVAL, RegisterContextModule(MOD_CMDLINE, NULL));

    EXPECT_SUCCESS(RegisterContextModule(MOD_CMDLINE, t));

    // This shouldn't have touched the static list.
    EXPECT_PTR_EQUAL(s_freeModules, &s_staticModules[0]);

    // But it should have allocated a module header and used that
    EXPECT_NOT_NULL(s_modulesHead);
    EXPECT_PTR_EQUAL(s_modulesHead, s_modulesTail);
    EXPECT_FALSE(s_modulesHead == &s_staticModules[0]);

    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesHead->links.next);
    EXPECT_VAL_EQUAL(s_modulesHead->id, MOD_CMDLINE);
    EXPECT_NULL(s_modulesHead->init);
    EXPECT_NULL(s_modulesHead->start);
    EXPECT_NULL(s_modulesHead->close);
    EXPECT_PTR_EQUAL(s_modulesHead->context, t);
    EXPECT_TRUE(s_modulesHead->allocd);

    EXPECT_STR_EQUAL("cmdline", s_modulesHead->name)
}

void
test_get_module(struct TestContext *t)
{
    EXPECT_NULL(GetModule((enum ModuleID)0));
    EXPECT_NULL(GetModule(MAX_MODULE_ID));

    struct Module *module = GetModule(MOD_CMDLINE);
    EXPECT_NOT_NULL(module);
    EXPECT_PTR_EQUAL(module, s_modulesHead);
    EXPECT_VAL_EQUAL(module->id, MOD_CMDLINE);
}

void
test_close_module(struct TestContext *t)
{
    EXPECT_ERROR(EINVAL, CloseModule(NULL, 0));
    EXPECT_ERROR(EFAULT, CloseModule((struct Module *)t, 0));

    struct Module *module = GetModule(MOD_CMDLINE);
    EXPECT_NOT_NULL(module);
    EXPECT_SUCCESS(CloseModule(module, 0));
    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_FALSE(s_modulesClosed);
    EXPECT_PTR_EQUAL(s_freeModules, &s_staticModules[0]);

    EXPECT_ERROR(EFAULT, CloseModule(module, 0));
}

void
test_new_static_module(struct TestContext *t)
{
    struct ModuleState *ms = (struct ModuleState *)(t->userData);
    struct Module *     module = NULL;

    EXPECT_SUCCESS(NewModule(true, MOD_COMPILER, modInit, modStart, modClose, ms, &module));
    EXPECT_NOT_NULL(module);
    EXPECT_PTR_EQUAL(s_modulesHead, module);
    EXPECT_PTR_EQUAL(s_modulesTail, module);
    EXPECT_PTR_EQUAL(&s_staticModules[0], module);
    EXPECT_TRUE(ms->init);
    EXPECT_FALSE(ms->start);
    EXPECT_FALSE(ms->close);

    // Shouldn't be linked to anything
    EXPECT_NULL(module->links.prev);
    EXPECT_NULL(module->links.next);
    EXPECT_VAL_EQUAL(MOD_COMPILER, module->id);

    EXPECT_PTR_EQUAL(s_freeModules, &s_staticModules[1]);
}

void
test_start_static_module(struct TestContext *t)
{
    struct ModuleState *ms = (struct ModuleState *)(t->userData);

    EXPECT_NOT_NULL(s_modulesHead);
    EXPECT_PTR_EQUAL(s_modulesHead, s_modulesTail);
    EXPECT_PTR_EQUAL(s_modulesHead, &s_staticModules[0]);
    EXPECT_SUCCESS(StartModules());
    EXPECT_FALSE(ms->init);
    EXPECT_TRUE(ms->start);
    EXPECT_FALSE(ms->close);
}

void
test_close_static_module(struct TestContext *t)
{
    struct ModuleState *ms = (struct ModuleState *)(t->userData);

    struct Module *module = GetModule(MOD_COMPILER);
    EXPECT_NOT_NULL(module);
    EXPECT_PTR_EQUAL(module, &s_staticModules[0]);
    EXPECT_PTR_EQUAL(ms, module->context);
    EXPECT_PTR_EQUAL(&s_staticModules[1], s_freeModules);

    EXPECT_SUCCESS(CloseModule(module, 42));

    EXPECT_FALSE(ms->init);
    EXPECT_FALSE(ms->start);
    EXPECT_TRUE(ms->close);
    EXPECT_VAL_EQUAL(ms->closeErr, 42);

    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_PTR_EQUAL(&s_staticModules[0], s_freeModules);
    EXPECT_PTR_EQUAL(&s_staticModules[1], s_freeModules->links.next);
}

void
test_multiple_modules(struct TestContext *t)
{
    struct ModuleState ms1 = {false, false, false};
    struct Module *    module1 = NULL;

    EXPECT_SUCCESS(NewModule(false, MOD_CMDLINE, modInit, modStart, modClose, &ms1, &module1));
    EXPECT_NOT_NULL(module1);
    EXPECT_PTR_EQUAL(s_modulesHead, module1);
    EXPECT_PTR_EQUAL(s_modulesTail, module1);
    EXPECT_PTR_EQUAL(&ms1, module1->context);
    EXPECT_TRUE(module1->allocd);
    EXPECT_TRUE(ms1.init);
    EXPECT_FALSE(ms1.start);
    EXPECT_FALSE(ms1.close);
    EXPECT_VAL_EQUAL(MOD_CMDLINE, module1->id);

    struct ModuleState ms2 = {false, false, false};
    struct Module *    module2 = NULL;
    EXPECT_SUCCESS(NewModule(true, MOD_COMPILER, modInit, modStart, modClose, &ms2, &module2));
    EXPECT_NOT_NULL(module2);
    EXPECT_PTR_EQUAL(s_modulesTail, module2);  // FILO order
    EXPECT_PTR_EQUAL(s_modulesHead, module1);
    EXPECT_PTR_EQUAL(&ms2, module2->context);
    EXPECT_FALSE(module2->allocd);
    EXPECT_TRUE(ms2.init);
    EXPECT_FALSE(ms2.start);
    EXPECT_FALSE(ms2.close);
    EXPECT_PTR_EQUAL(&s_staticModules[0], module2);

    EXPECT_PTR_EQUAL(s_modulesHead->links.next, s_modulesTail);
    EXPECT_PTR_EQUAL(s_modulesTail->links.prev, s_modulesHead);

    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);

    struct ModuleState ms3 = {false, false, false};
    struct Module *    module3 = NULL;
    EXPECT_SUCCESS(NewModule(false, MOD_STRINGS, modInit, modStart, modClose, &ms3, &module3));
    EXPECT_NOT_NULL(module3);
    EXPECT_PTR_EQUAL(s_modulesTail, module3);  // FILO order
    EXPECT_PTR_EQUAL(s_modulesTail->links.prev, module2);
    EXPECT_PTR_EQUAL(s_modulesHead, module1);
    EXPECT_PTR_EQUAL(s_modulesHead->links.next, module2);
    EXPECT_PTR_EQUAL(&ms1, module1->context);
    EXPECT_PTR_EQUAL(&ms2, module2->context);
    EXPECT_PTR_EQUAL(&ms3, module3->context);
    EXPECT_TRUE(module3->allocd);
    EXPECT_TRUE(ms3.init);
    EXPECT_FALSE(ms3.start && ms3.close);
    EXPECT_PTR_EQUAL(&s_staticModules[1], s_freeModules);

    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);

    // Remove the middle module
    EXPECT_SUCCESS(CloseModule(module2, 50));
    EXPECT_TRUE(ms2.init && ms2.close);
    EXPECT_FALSE(ms2.start);
    EXPECT_VAL_EQUAL(ms2.closeErr, 50);
    EXPECT_PTR_EQUAL(&s_staticModules[0], s_freeModules);
    EXPECT_PTR_EQUAL(&s_staticModules[1], s_staticModules[0].links.next);
    EXPECT_NULL(s_staticModules[0].links.prev);

    EXPECT_PTR_EQUAL(s_modulesHead, module1);
    EXPECT_PTR_EQUAL(s_modulesTail, module3);
    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);
    EXPECT_PTR_EQUAL(s_modulesHead->links.next, s_modulesTail);
    EXPECT_PTR_EQUAL(s_modulesTail->links.prev, s_modulesHead);

    // Start the other two
    EXPECT_SUCCESS(StartModules());
    EXPECT_TRUE(ms1.init && ms1.start);
    EXPECT_FALSE(ms1.close);
    EXPECT_FALSE(ms2.start);
    EXPECT_TRUE(ms3.init && ms3.start);
    EXPECT_FALSE(ms3.close);

    // Close the remaining modules
    CloseModules(45);
    EXPECT_TRUE(ms1.init && ms1.start && ms1.close);
    EXPECT_TRUE(ms3.init && ms3.start && ms3.close);
    EXPECT_VAL_EQUAL(45, ms1.closeErr);
    EXPECT_VAL_EQUAL(45, ms3.closeErr);
    EXPECT_TRUE(s_modulesClosed);

    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_PTR_EQUAL(&s_staticModules[0], s_freeModules);
}

void
modules_tests(struct TestContext *t)
{
    struct ModuleState ms;
    t->userData = &ms;
    t->tearUp = modules_test_tearUp;

    RUN_TEST(test_init_modules);
    RUN_TEST(test_register_context_module);
    RUN_TEST(test_get_module);
    RUN_TEST(test_close_module);
    RUN_TEST(test_new_static_module);
    RUN_TEST(test_start_static_module);
    RUN_TEST(test_close_static_module);
    RUN_TEST(test_multiple_modules);
}
