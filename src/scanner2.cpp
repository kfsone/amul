#include <cctype>

#define REF(name) *name[static 0]

struct AtomSet {
    Atom REF(atoms);
    size_t numAtoms;
    Atom REF(cur), REF(end);
};

typedef bool (*ASPredicate)(const Atom REF(*cur), const Atom REF(end))

// PREDICATE declares a function prototype as an atom predicate
#define PREDICATE(name) bool name(const Atom REF(*cur), const Atom REF(end))

// TYPE_PREDICATE creates an instance of a PREDICATE that consumes
// one+ instances of things that match an atom type.
#define TYPE_PREDICATE(name, type)                                                                 \
    PREDICATE(name)                                                                                \
    {                                                                                              \
        if ((*cur) >= end || !type(**cur))                                                         \
            return false;                                                                          \
        do {                                                                                       \
            ++cur;                                                                                 \
        } while ((*cur) < end && type(**cur));                                                     \
        return true;                                                                               \
    }

        TYPE_PREDICATE(aAlpha, isalpha) TYPE_PREDICATE(aDigit, isdigit)
                TYPE_PREDICATE(aSpace, isblank)

                        bool iseol(const char REF(p))
{
    return (p == '\n' || p == '\r');
}

PREDICATE(aEol)
{
    if ((*cur) >= end || !iseol(**cur))
        return false;
    char lastEol = *(*(cur++));
    // consume \r after \n or \n after \r but not \n\n
    if ((*cur) < end && **cur != lastEol && iseol(**cur))
        ++cur;
    return true;
}

#define SYMBOL_PREDICATE(name, symbol)                                                             \
    PREDICATE(name) { return symbolPredicate(cur, end, symbol); }

bool
symbolPredicate(const Atom **cur, const Atom *end, char symbol)
{
    if ((*cur) >= end || (**cur) != symbol)
        return false;
    ++(*cur);
    return true;
}

SYMBOL_PREDICATE(aDot, '.')
SYMBOL_PREDICATE(aUnderscore, '_')
SYMBOL_PREDICATE(aMinus, '-')
SYMBOL_PREDICATE(aEqual, '=')
SYMBOL_PREDICATE(aAt, '@')
SYMBOL_PREDICATE(aBackslash, '\\')

///////////////////////////////////////////////////////////////////////////////
// Multi-atom lex

PREDICATE(aFloat)
{
    if (aDigit(&cur, end)) {
        if (aDot(&cur, end)) {
            aDigit(&cur, end);
        }
        return false;
    }
    return AtomSetConsume(cur, end, { aDot, aDigit, nullptr });
}

PREDICATE(aNumber)
{
    Atom REF(pos) = *cur;
    aMinus(&pos, end);
    if (aDigit(&pos, end)) {
        *cur = pos;
        return true;
    }
    return false;
}

bool
aWord(const Atom **cur, const Atom *end)
{
    const Atom *pos = *cur;
    if (pos >= end)
        return false;

    if (!aAlpha(&pos, end))
        return false;

    while (pos < end && (aMinus(&pos, end) || aUnderscore(&pos, end) || aAlpha(&pos, end)))
        ;

    *cur = pos;
    return true;
}

bool
aIdentifier(const Atom **cur, const Atom *end)
{
    const Atom *pos = *cur;
    if (pos >= end)
        return false;

    if (!aAlpha(&pos, end))
        return false;

    while (pos < end && (aUnderscore(&pos, end) || aAlpha(&pos, end)))
        ;

    *cur = pos;
    return true;
}

bool
aStringLit(const Atom **cur, const Atom *end)
{
    if (!symbolPredicate(cur, end, '"') && !symbolPredicate(cur, end, '\''))
        return false;
    const char quote = **cur;
    const Atom *pos = cur + 1;
    while (pos < end) {
        if (iseol(*pos))
            break;
        if (symbolPredicate(pos, end, quote))
            break;
        if (aBackslash(pos, end, quote)) {
            if (pos >= end || iseol(*pos))
                return false;
            ++pos;
            continue;
        }
        ++pos;
    }
    *cur = pos;
    return true;
}

bool
AtomSetConsume(const Atom REF(*cur), const Atom REF(end), ASPredicate REF(predicates))
{
    Atom REF = *cur;
    ASPredicate *pred = predicates;
    for (*pred) {
        if (pos >= as->end)
            return false;
        if (!pred(&pos, as->end))
            return false;
        ++pred;
    }
    *cur = pos;
    return true;
}
