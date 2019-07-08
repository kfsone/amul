#pragma once

#include <cstdint>

void    reset();
void    look(const char *roomName, int f);
void    agive(int objectNo, int to);
void    adrop(int objectNo, int roomNo, int f);
int     owner(int objectNo);
void    show_rank(int playerNo, int rankNo, int gender);
void    make_rank(int playerNo, int rankNo, int gender);
void    moveto(int roomNo);
int32_t mod(int32_t n, int32_t x);
int32_t rnd();
bool    gotin(int objectNo, int state);
int     achecknear(int objectNo);
int     acheckget(int objectNo);
void    look_here(int f, int rm);
void    desc_here(int f);
void    list_what(int roomNo, int i);
void    descobj(int objectNo);
void    inflict(int x, int s);
void    cure(int x, int s);
void    summon(int player);
void    adestroy(int objectNo);
void    arecover(int objectNo);
void    refresh();
void    zapme();
void    send(int objectNo, int to);
void    achange(int player);
void    newrank(int player, int rank);
void    aadd(int points, int stat, int player);
void    asub(int points, int stat, int player);
void    afix(int stat, int player);
void    announce(const char *s, int player);
void    announcein(int room, const char *s);
void    announcefrom(int objectNo, const char *s);
void    objannounce(int objectNo, const char *s);
void    action(const char *s, int player);
void    actionin(int room, const char *s);
void    actionfrom(int objectNo, const char *s);
void    objaction(int objectNo, const char *s);
void    fwait(int32_t n);
void    ableep(int n);
void    lighting(int x, int twho);
void    loseobj(int obj);
void    nohelp();
void    aforce(int whom, const char *cmd);
void    afight(int player);
void    clearfight();
void    finishfight(int player);
void    acombat();
void    exits();
int     isaroom(const char *s);
void    follow(int player, const char *cmd);
void    log(const char *s);
void    PutRankInto(char *s);
void    PutARankInto(char *s, int rankNo);
void    akillme();
void    show_tasks(int playerNo);
void    dropall(int intoRoom);
void    invent(int playerNo);
void    ascore(int type);
void    calcdext();
void    toprank();
void    damage(int object, int howmuch);
void    repair(int object, int howmuch);
void    asave();
void    save_me();
void    aquit();
void    whohere();
void    awho();
void    flagbits();
void    getllen();
void    getslen();
void    getrchar();
void    getflags();
int     numb(int32_t number, int32_t comparison);
void    atreatas(uint32_t verbno);
void    afailparse();
void    afinishparse();
void    aabortparse();
void    ado(int verb);
void    add_obj(int to);
void    rem_obj(int to, int object);
void    ainteract(int who);
void    asyntax(int n1, int n2);
void    iocopy(char *into, const char *src, uint32_t length);
void    qcopy(char *into, const char *src, uint32_t length);
void    DoThis(int playerNo, const char *cmd, short int type);
void    StopFollow();
void    internal(const char *s);
void    LoseFollower();
void    ShowFile(const char *s);
int32_t scaled(int32_t value, short int flags);
void    showin();
int     stfull(int st, int p);
void    asetstat(int obj, int stat);
void    awhere(int obj);
void    osflag(int o, int flag);
void    setmxy(int Flags, int Them);
