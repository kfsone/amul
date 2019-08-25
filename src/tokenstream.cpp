////////////
// WORK IN PROGRESS
////////////

#include <cctype>
#include <iterator>

#include "h/tokenstream.h"

// void
// ZeroPlus(TokenStream &ts, Patterns patterns)
// {
//     if (ts.end())
//         accept();
//     for (auto &&pattern : patterns) {
//         pattern.match(ts);
//         if (ts.valid())
// 			accept();
// 		else
// 			rewind();
//     }
// }

// using ExpressionOp = bool (*)(TokenStream &);
//
// struct Expression final {
//     ExpressionOp m_op;
//     constexpr Expression(ExpressionOp op) noexcept
//         : m_op(op)
//     {
//     }
//     bool operator()(TokenStream &ts) {}
// };
//

void
TokenStream::Atomize()
{
    Atom *comment = nullptr;
    do {
        const char *start = buffer.it();
        AtomType    at{NextAtomType(buffer)};
        switch (at) {
        case A_END:
            if (*start) {
                atoms.Push(Atom{A_END, "\n", 1});
            } else {
                atoms.Push(Atom{A_END, "", 0});
            }
            break;
        case A_SPACE:
            atoms.Push(Atom{A_SPACE, " ", 1});
            break;
        case A_PUNCT:
            if (*start == ';' && !comment)
                comment = atoms.m_cur;
            /*FALLTHROUGH*/
        default:
            size_t len = buffer.it() - start;
            atoms.Push(Atom{at, start, len});
        }
    } while (*atoms.Back() != A_END);

    if (comment) {
        // Consolidate comments into a single A_END token
        comment->m_end = atoms.Back()->m_end;
        atoms.Pop(atoms.end() - comment);
        comment->m_type = A_END;
    }
}

void
TokenStream::ScanLine()
{
    Atomize();
}

void
testme()
{
    Buffer      buffer{"r=a b1 \n"};
    TokenStream ts{buffer};
    ts.SetPattern(nullptr /*tbd*/);
    ts.ScanLine();
}
