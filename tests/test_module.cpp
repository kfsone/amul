#include "gtest_aliases.h"
#include <gtest/gtest.h>

#include "filesystem.h"
#include "modules.h"

extern Module *s_modulesHead;
extern Module *s_modulesTail;
extern bool    s_modulesInitialized;
extern bool    s_modulesClosed;

struct ModuleState {
    bool    inited{false};
    bool    started{false};
    bool    closed{false};
    error_t closeErr{0};
};

error_t
modInit(Module *module)
{
    ((ModuleState *)(module->context))->inited = true;
    return 0;
}

error_t
modStart(Module *module)
{
    ((ModuleState *)(module->context))->started = true;
    return 0;
}

error_t
modClose(Module *module, error_t err)
{
    ((ModuleState *)(module->context))->closed = true;
    ((ModuleState *)(module->context))->closeErr = err;
    return 0;
}

TEST(ModuleTest, InitModules)
{
    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_FALSE(s_modulesInitialized);
    EXPECT_FALSE(s_modulesClosed);

    InitModules();

    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_TRUE(s_modulesInitialized);
    EXPECT_FALSE(s_modulesClosed);
}

TEST(ModuleTest, RegisterContextModuleChecks)
{
    EXPECT_ERROR(EINVAL, RegisterContextModule((ModuleID)0, NULL));
    EXPECT_ERROR(EINVAL, RegisterContextModule(MAX_MODULE_ID, NULL));

    // Context modules *must* have a context
    EXPECT_ERROR(EINVAL, RegisterContextModule(MOD_CMDLINE, NULL));
}

TEST(ModuleTest, RegisterContextModule)
{
    EXPECT_SUCCESS(RegisterContextModule(MOD_CMDLINE, this));

    EXPECT_NOT_NULL(s_modulesHead);
    EXPECT_EQ(s_modulesHead, s_modulesTail);

    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesHead->links.next);
    EXPECT_EQ(s_modulesHead->id, MOD_CMDLINE);
    EXPECT_NULL(s_modulesHead->init);
    EXPECT_NULL(s_modulesHead->start);
    EXPECT_NULL(s_modulesHead->close);
    EXPECT_EQ(s_modulesHead->context, this);

    EXPECT_STREQ("cmdline", s_modulesHead->name);
}

TEST(ModuleTest, GetModuleChecks)
{
    EXPECT_NULL(GetModule((ModuleID)0));
    EXPECT_NULL(GetModule(MAX_MODULE_ID));
}

TEST(ModuleTest, GetModule)
{
	EXPECT_NOT_NULL(s_modulesHead);
	EXPECT_STREQ("cmdline", s_modulesHead->name);

    Module *module = GetModule(MOD_CMDLINE);
    EXPECT_NOT_NULL(module);
    EXPECT_EQ(module, s_modulesHead);
    EXPECT_EQ(module->id, MOD_CMDLINE);
}

TEST(ModuleTest, CloseModuleChecks)
{
    EXPECT_ERROR(EINVAL, CloseModule(NULL, 0));
    EXPECT_ERROR(EFAULT, CloseModule(reinterpret_cast<Module *>(this), 0));
}

TEST(ModuleTest, CloseModule)
{
    Module *module = GetModule(MOD_CMDLINE);
    EXPECT_NOT_NULL(module);
    EXPECT_SUCCESS(CloseModule(module, 0));
    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
    EXPECT_FALSE(s_modulesClosed);

    EXPECT_ERROR(EFAULT, CloseModule(module, 0));
}

TEST(ModuleTest, MultipleModules)
{
    ModuleState ms1{};
    Module *    module1{nullptr};

    EXPECT_SUCCESS(NewModule(MOD_CMDLINE, modInit, modStart, modClose, &ms1, &module1));
    EXPECT_NOT_NULL(module1);
    EXPECT_EQ(s_modulesHead, module1);
    EXPECT_EQ(s_modulesTail, module1);
    EXPECT_EQ(&ms1, module1->context);
    EXPECT_TRUE(ms1.inited);
    EXPECT_FALSE(ms1.started);
    EXPECT_FALSE(ms1.closed);
    EXPECT_EQ(MOD_CMDLINE, module1->id);

    ModuleState ms2{};
    Module *    module2 = NULL;
    EXPECT_SUCCESS(NewModule(MOD_COMPILER, modInit, modStart, modClose, &ms2, &module2));
    EXPECT_NOT_NULL(module2);
    EXPECT_EQ(s_modulesTail, module2);  // FILO order
    EXPECT_EQ(s_modulesHead, module1);
    EXPECT_EQ(&ms2, module2->context);
    EXPECT_TRUE(ms2.inited);
    EXPECT_FALSE(ms2.started);
    EXPECT_FALSE(ms2.closed);

    EXPECT_EQ(s_modulesHead->links.next, (DoubleLinkedNode *)s_modulesTail);
    EXPECT_EQ(s_modulesTail->links.prev, (DoubleLinkedNode *)s_modulesHead);

    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);

    ModuleState ms3 = {false, false, false};
    Module *    module3 = NULL;
    EXPECT_SUCCESS(NewModule(MOD_STRINGS, modInit, modStart, modClose, &ms3, &module3));
    EXPECT_NOT_NULL(module3);
    EXPECT_EQ(s_modulesTail, module3);  // FILO order
    EXPECT_EQ(s_modulesTail->links.prev, (DoubleLinkedNode *)module2);
    EXPECT_EQ(s_modulesHead, module1);
    EXPECT_EQ(s_modulesHead->links.next, (DoubleLinkedNode *)module2);
    EXPECT_EQ(&ms1, module1->context);
    EXPECT_EQ(&ms2, module2->context);
    EXPECT_EQ(&ms3, module3->context);
    EXPECT_TRUE(ms3.inited);
    EXPECT_FALSE(ms3.started && ms3.closed);
    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);

    // Remove the middle module
    EXPECT_SUCCESS(CloseModule(module2, 50));
    EXPECT_TRUE(ms2.inited && ms2.closed);
    EXPECT_FALSE(ms2.started);
    EXPECT_EQ(ms2.closeErr, 50);

    EXPECT_EQ(s_modulesHead, module1);
    EXPECT_EQ(s_modulesTail, module3);
    EXPECT_NULL(s_modulesHead->links.prev);
    EXPECT_NULL(s_modulesTail->links.next);
    EXPECT_EQ(s_modulesHead->links.next, (DoubleLinkedNode *)s_modulesTail);
    EXPECT_EQ(s_modulesTail->links.prev, (DoubleLinkedNode *)s_modulesHead);

    // Start the other two
    EXPECT_SUCCESS(StartModules());
    EXPECT_TRUE(ms1.inited && ms1.started);
    EXPECT_FALSE(ms1.closed);
    EXPECT_FALSE(ms2.started);
    EXPECT_TRUE(ms3.inited && ms3.started);
    EXPECT_FALSE(ms3.closed);

    // Close the remaining modules
    CloseModules(45);
    EXPECT_TRUE(ms1.inited && ms1.started && ms1.closed);
    EXPECT_TRUE(ms3.inited && ms3.started && ms3.closed);
    EXPECT_EQ(45, ms1.closeErr);
    EXPECT_EQ(45, ms3.closeErr);
    EXPECT_TRUE(s_modulesClosed);

    EXPECT_NULL(s_modulesHead);
    EXPECT_NULL(s_modulesTail);
}
