#include "amulcom.includes.h"
#include "extras.h"

TEST(ExtrasTest, StripLead)
{
	char testData[64];
	strcpy(testData, "test=foo");
	EXPECT_TRUE(striplead("test=", testData));
	EXPECT_STREQ(testData, "foo");
	EXPECT_FALSE(striplead("test=", testData));
	EXPECT_STREQ(testData, "foo");
}

TEST(ExtrasTest, GetWord_First)
{
	const char *first = "first";
	p = getword("first");
	EXPECT_STREQ(Word, "first");
	EXPECT_EQUAL(p, first + strlen("first"));
	EXPECT_EQUAL(*p, 0);
}

TEST(ExtrasTest, GetWord_HelloWorld)
{
	const char *p = getword("  hello world");
	EXPECT_STREQ(p, "world");
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
