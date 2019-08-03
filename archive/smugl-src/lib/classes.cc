#include "includes.hpp"
#include "structs.hpp"
#include <fcntl.h>

int
BASIC_OBJ::Write(FILE *file)
{
    WRITE(id);
    WRITE(adj);
    WRITE(bob);
    WRITE(next);
    WRITE(type);
    WRITE(state);
    WRITE(std_flags);
    WRITE(flags);
    WRITE(weight);
    WRITE(max_weight);
    WRITE(contents_weight);
    WRITE(value);
    WRITE(damage);
    WRITE(strength);
    WRITE(locations);
    WRITE(contents);
    WRITE(conLocation);
    WRITE(conTent);
    WRITE(s_descrip);
    WRITE(l_descrip);
    WRITE(dmove);
    return 1;
}

int
BASIC_OBJ::Read(FILE *file)
{
    READ(id);
    READ(adj);
    READ(bob);
    READ(next);
    READ(type);
    READ(state);
    READ(std_flags);
    READ(flags);
    READ(weight);
    READ(max_weight);
    READ(contents_weight);
    READ(value);
    READ(damage);
    READ(strength);
    READ(locations);
    READ(contents);
    READ(conLocation);
    READ(conTent);
    READ(s_descrip);
    READ(l_descrip);
    READ(dmove);
    return 1;
}

int
OBJ::Write(FILE *file)
{
    BASIC_OBJ::Write(file);
    WRITE(putto);
    WRITE(article);
    WRITE(nstates);
    WRITE(mobile);
    WRITE(states);
    return 1;
}

int
OBJ::Read(FILE *file)
{
    BASIC_OBJ::Read(file);
    READ(putto);
    READ(article);
    READ(nstates);
    READ(mobile);
    READ(states);
    return 1;
}

int
PLAYER::Write(FILE *file)
{
    BASIC_OBJ::Write(file);
    WRITE(_name);
    WRITE(passwd);
    WRITE(score);
    WRITE(rdmode);
    WRITE(plays);
    WRITE(tasks);
    WRITE(last_session);
    WRITE(stamina);
    WRITE(dext);
    WRITE(wisdom);
    WRITE(experience);
    WRITE(magicpts);
    WRITE(rank);
    WRITE(tries);
    WRITE(pclass);
    WRITE(sex);
    WRITE(llen);
    WRITE(slen);

    WRITE(sctg);
    WRITE(rec);
    WRITE(dextadj);
    WRITE(wield);
    WRITE(light);
    WRITE(hadlight);
    WRITE(helping);
    WRITE(helped);
    WRITE(following);
    WRITE(followed);
    WRITE(fighting);
    WRITE(pre);
    WRITE(post);
    WRITE(arr);
    WRITE(dep);

    return 1;
}

int
PLAYER::Read(FILE *file)
{
    BASIC_OBJ::Read(file);
    READ(_name);
    READ(passwd);
    READ(score);
    READ(rdmode);
    READ(plays);
    READ(tasks);
    READ(last_session);
    READ(stamina);
    READ(dext);
    READ(wisdom);
    READ(experience);
    READ(magicpts);
    READ(rank);
    READ(tries);
    READ(pclass);
    READ(sex);
    READ(llen);
    READ(slen);

    READ(sctg);
    READ(rec);
    READ(dextadj);
    READ(wield);
    READ(light);
    READ(hadlight);
    READ(helping);
    READ(helped);
    READ(following);
    READ(followed);
    READ(fighting);
    READ(pre);
    READ(post);
    READ(arr);
    READ(dep);

    return 1;
}

int
ROOM::Write(FILE *file)
{
    BASIC_OBJ::Write(file);
    WRITE(visitor_bf);
    WRITE(tabptr);
    WRITE(ttlines);

    return 1;
}

int
ROOM::Read(FILE *file)
{
    BASIC_OBJ::Read(file);
    READ(visitor_bf);
    READ(tabptr);
    READ(ttlines);

    return 1;
}
