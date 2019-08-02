#include "atom.h"
#include "buffer.h"
#include "gtest_aliases.h"
#include <gtest/gtest.h>

TEST(AtomTest, AtomCtorFull)
{
    const char *spaces = " \t ";
    Atom        atom{A_SPACE, spaces, 3};
    EXPECT_EQ(atom.m_type, A_SPACE);
    EXPECT_EQ(atom.m_start, spaces);
    EXPECT_EQ(atom.m_end, spaces + 3);
}

TEST(AtomTest, AtomCtorAtomType)
{
    Atom atom{A_END};
    EXPECT_EQ(atom.m_type, A_END);
    EXPECT_EQ(*atom.m_start, 0);
    EXPECT_EQ(atom.m_start, atom.m_end);
}

TEST(AtomTest, AtomCtorPunct)
{
    Atom atom{";"};
    EXPECT_EQ(atom.m_type, A_PUNCT);
    EXPECT_EQ(*atom.m_start, ';');
    EXPECT_EQ(atom.m_end, atom.m_start + 1);
}

TEST(AtomTest, AtomType)
{
    Atom atom{A_LETTER};
    EXPECT_EQ(atom.Type(), A_LETTER);
}

TEST(AtomTest, AtomFirst)
{
    Atom atom{"!"};
    EXPECT_EQ(atom.First(), '!');
}

TEST(AtomTest, OperatorEq)
{
    Atom atom{"*"};
    EXPECT_TRUE(atom == A_PUNCT);
    EXPECT_TRUE(atom == '*');
    EXPECT_FALSE(atom == A_END);
    EXPECT_FALSE(atom == '!');
}

TEST(AtomTest, OperatorNe)
{
    Atom atom{"*"};
    EXPECT_FALSE(atom != A_PUNCT);
    EXPECT_FALSE(atom != '*');
    EXPECT_TRUE(atom != A_END);
    EXPECT_TRUE(atom != '!');
}

TEST(AtomTest, NextAtomType)
{
    Buffer      buffer{"hello\n123\r!="};
    const char *start = buffer.it();
    AtomType    at = NextAtomType(buffer);
    EXPECT_NE(buffer.it(), start);
    EXPECT_EQ(at, A_LETTER);
    EXPECT_EQ(strncmp(start, "hello", buffer.it() - start), 0);

    start = buffer.it();
    at = NextAtomType(buffer);
    EXPECT_NE(buffer.it(), start);
    EXPECT_EQ(at, A_END);
    EXPECT_EQ(*start, '\n');
    EXPECT_EQ(start + 1, buffer.it());

    start = buffer.it();
    at = NextAtomType(buffer);
    EXPECT_EQ(at, A_DIGIT);
    EXPECT_EQ(strncmp(start, "123", buffer.it() - start), 0);

    start = buffer.it();
    at = NextAtomType(buffer);
    EXPECT_EQ(*start, '\r');
    EXPECT_EQ(start + 1, buffer.it());

    start = buffer.it();
    at = NextAtomType(buffer);
    EXPECT_EQ(at, A_PUNCT);
    EXPECT_EQ(*start, '!');

    start = buffer.it();
    at = NextAtomType(buffer);
    EXPECT_EQ(at, A_PUNCT);
    EXPECT_EQ(*start, '=');

    EXPECT_EQ(A_END, NextAtomType(buffer));

    char space[] = "   \t\t   "
                   "\r\n"
                   "\r\n"
                   "\t\t\t"
                   "\n\r"
                   "\n\r"
                   "\r\n"
                   "\n"
                   "!";
    buffer.Assign(space, sizeof(space) + 1);
    EXPECT_EQ(NextAtomType(buffer), A_SPACE);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_SPACE);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_END);
    EXPECT_EQ(NextAtomType(buffer), A_PUNCT);
}

TEST(AtomTest, AtomCtorBufferEmpty)
{
    Buffer empty{};
    Atom   atom{empty};
    EXPECT_EQ(atom.Type(), A_END);
    EXPECT_EQ(atom.m_start, nullptr);
    EXPECT_EQ(atom.m_end, nullptr);
}

TEST(AtomTest, AtomCtorBufferString)
{
    Buffer space_nl_hi{"\t \r\nhi"};
    Atom   space{space_nl_hi};
    EXPECT_EQ(space.Type(), A_SPACE);
	EXPECT_EQ(space.First(), '\t');
    Atom nl{space_nl_hi};
    EXPECT_EQ(nl.Type(), A_END);
    Atom hi{space_nl_hi};
    EXPECT_EQ(hi.Type(), A_LETTER);
    EXPECT_EQ(hi.First(), 'h');
    Atom end{space_nl_hi};
    EXPECT_EQ(end.Type(), A_END);
    EXPECT_EQ(end.First(), 0);
}