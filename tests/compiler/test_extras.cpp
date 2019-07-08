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

    p = " foo=bar";
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
    const char *p = getword(
            "   "
            "12345678901234567890123456789012345678901234567890123456789012345678901234567890;end");
    EXPECT_STREQ(Word, "123456789012345678901234567890123456789012345678901234567890123");
    EXPECT_STREQ(p, ";end");
}

TEST(ExtrasTest, GetWord_OverFlow_Space)
{
    const char *p = getword(
            "   12345678901234567890123456789012345678901234567890123456789012345678901234567890 "
            "end");
    EXPECT_STREQ(Word, "123456789012345678901234567890123456789012345678901234567890123");
    EXPECT_STREQ(p, " end");
}

TEST(ExtrasTest, ExtractLine_Empty)
{
    char        output[100]{};
    const char *input1 = "";
    EXPECT_EQ(extractLine(input1, output), input1);
    EXPECT_EQ(output[0], 0);

    const char *input2 = ";";
    EXPECT_EQ(extractLine(input2, output), input2 + 1);
    EXPECT_EQ(output[0], 0);

    const char *input3 = "\n";
    EXPECT_EQ(extractLine(input3, output), input3 + 1);
    EXPECT_EQ(output[0], 0);
}

TEST(ExtrasTest, ExtractLine_Comments)
{
    char        output[100]{};
    const char *input1 = ";\n;cmt\n;comment\nHello";
    const char *p1 = extractLine(input1, output);
    EXPECT_EQ(*p1, 0);
    EXPECT_EQ(p1, input1 + strlen(input1));
    EXPECT_STREQ(output, "Hello");

    const char *input2 = ";\n;cmt\n;comment\nHello\n";
    const char *p2 = extractLine(input2, output);
    EXPECT_EQ(*p2, 0);
    EXPECT_EQ(p2, input2 + strlen(input2));
    EXPECT_STREQ(output, "Hello");

    const char *input3 = ";\n;\nHello ;world\n";
    const char *p3 = extractLine(input3, output);
    EXPECT_EQ(*p3, 0);
    EXPECT_EQ(p3, input3 + strlen(input3));
    EXPECT_STREQ(output, "Hello ;world");
}

TEST(ExtrasTest, ExtractLine_Multiline)
{
    char        output[100]{};
    const char *input = "Line 1\n; Comment\n2nd\n";

    const char *p = extractLine(input, output);
    EXPECT_STREQ(output, "Line 1");
    EXPECT_FALSE(p == input);
    EXPECT_EQ(*p, ';');

    p = extractLine(p, output);
    EXPECT_STREQ(output, "2nd");
    EXPECT_EQ(p, input + strlen(input));
}

TEST(ExtrasTest, SkipLine)
{
    EXPECT_STREQ(skipline(""), "");
    EXPECT_STREQ(skipline("abc\ndef"), "def");
    EXPECT_STREQ(skipline("\r\nboo"), "boo");
}