struct AtomSet
{
    Atom*       atoms;
    size_t      numAtoms;
    Atom*       cur;
    Atom*       end;
};

typedef bool (*ASPredicate)(const Atom **cur, const Atom *end)

bool
AtomSetConsume(const Atom **cur, const Atom *end, ASPredicate *predicates)
{
    Atom *pos = *cur;
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

#define PREDICATE(name) bool name (const Atom **cur, const Atom *end)
#define CHECK_EOF() if (*cur >= end) return false;

TYPE_PREDICATE(name, type)                      \
PREDICATE(name)                                 \
{                                               \
    if ((*cur) >= end || !type(**cur))          \
        return false;                           \
    do {                                        \
        ++cur;                                  \
    } while ((*cur) < end && type(**cur));      \
    return true;                                \
}

bool
symbolPredicate(const Atom **cur, const Atom *end, char symbol)
{
    if ((*cur) >= end || (**cur) != symbol)
        return false;
    ++(*cur);
    return true;
}
#define SYMBOL_PREDICATE(name, symbol)          \
PREDICATE(name)                                 \
{                                               \
    return symbolPredicate(cur, end, symbol);   \
}

bool iseol(const char *p) { return (p == '\n' || p == '\r') };

TYPE_PREDICATE(aAlpha, isalpha)
TYPE_PREDICATE(aDigit, isdigit)
TYPE_PREDICATE(aSpace, isblank)
TYPE_PREDICATE(aEol,   iseol)

SYMBOL_PREDICATE(aDot, '.')
SYMBOL_PREDICATE(aUnderscore, '_')
SYMBOL_PREDICATE(aMinus, '-')
SYMBOL_PREDICATE(aEqual, '=')
SYMBOL_PREDICATE(aAt, '@')
SYMBOL_PREDICATE(aBackslash, '\\')

bool
aFloat(const Atom **cur, const Atom *end)
{
    if (aDigit(&cur, end)) {
        if (aDot(&cur, end)) {
            aDigit(&cur, end);
        }
        return false;
    }
    return AtomSetConsume(cur, end, { aDot, aDigit, NULL });
}

bool
aNumber(const Atom **cur, const Atom *end)
{
    Atom *pos = *cur;
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