#include "amulcom.includes.h"
#include "extras.h"
#include "gtest/gtest.h"

TEST(ExtrasTest, SkipLead)
{
    const char *p = "";
    EXPECT_FALSE(skiplead("foo=", &p));
    EXPECT_STREQ(p, "");

    p = "foo";
    EXPECT_FALSE(skiplead("foo=", &p));
    EXPECT_STREQ(p, "foo");

    p = "foo=";
    EXPECT_TRUE(skiplead("foo=", &p));
    EXPECT_EQ(*p, 0);

	p = "foo=bar";
    EXPECT_TRUE(skiplead("foo=", &p));
    EXPECT_STREQ(p, "bar");
}

TEST(ExtrasTest, StripLead)
{
    char p1[] = "";
    EXPECT_FALSE(striplead("foo=", p1));

    char p2[] = "foo";
    EXPECT_FALSE(striplead("foo=", p2));

    char p3[] = "foo=";
    EXPECT_TRUE(striplead("foo=", p3));
    EXPECT_EQ(p3[0], 0);

    char p4[] = "foo=bar";
    EXPECT_TRUE(striplead("foo=", p4));
    EXPECT_STREQ(p4, "bar");
}

TEST(ExtrasTest, GetWord_First)
{
    const char *first = "first";
    const char *p = getword("first");
    EXPECT_STREQ(Word, "first");
    EXPECT_EQ(p, first + strlen("first"));
    EXPECT_EQ(*p, 0);
}

TEST(ExtrasTest, GetWord_HelloWorld)
{
    const char *p = getword("  hello world");
    EXPECT_STREQ(p, " world");
    EXPECT_STREQ(Word, "hello");
}

TEST(ExtrasTest, GetWord_OverFlow_Semi)
{
    const char *p = getword("   12345678901234567890123456789012345678901234567890123456789012345678901234567890;end");
    EXPECT_STREQ(Word, "123456789012345678901234567890123456789012345678901234567890123");
    EXPECT_STREQ(p, ";end");
}

TEST(ExtrasTest, GetWord_OverFlow_Space)
{
    const char *p = getword("   12345678901234567890123456789012345678901234567890123456789012345678901234567890 end");
    EXPECT_STREQ(Word, "123456789012345678901234567890123456789012345678901234567890123");
    EXPECT_STREQ(p, " end");
}
