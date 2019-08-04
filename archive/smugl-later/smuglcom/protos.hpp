#ifndef SMUGL_SMUGLCOM_PROTOS_H
#define SMUGL_SMUGLCOM_PROTOS_H

#include "typedefs.hpp"

int is_room_flag(const char *s);
int is_room_param(char *&s);
int isoflag1(const char *s);
int isoparm();
int isoflag2(const char *s);
void checksys();
int iscond(const char *s);
int isact(const char *s);
void section(int i);
void errabort();
/*
int higerhalph(char *s1, char *s2);
void sort_objs();
*/
void close_ofps();
int nextc(int);
void fopenw(const char *s);
void fopena(const char *s);
void fopenr(const char *s);
void Err(const char *s, const char *t);
void ttroomupdate();
void skipblock();
char *skipdata(char *p);
void tidy(char *s);
void clean_trim(char *s);
int is_verb(const char *s);
char *blkget();
char *cleanget(offset_t off = 0);
void lang_proc();
int actualval(const char *, arg_t);
char *precon(char *);
char *preact(char *);
char *chkaparms(char *, int);
char *chkcparms(char *, int);
void umsg_proc();
void sys_proc();
void room_proc();
void finish_rooms();
void smsg_proc();
void mob_proc1();
void obds_proc();
void objs_proc();
void trav_proc();
void syn_proc();
void opentxt(const char *s);
int isnoun(const char *s);
msgno_t ttumsgchk(char *s);
char *getword(char *s);
char *skiplead(const char *skip, char *in);
char *skipline(char *s);
void repspc(char *s);
void clean_up(char *p);
void tx(const char *s);
void *grow(void *ptr, size_t size, const char *msg);
void save_messages();
msgno_t add_msg(const char *);
msgno_t ismsgid(const char *);
vocid_t new_word(const char *, bool);
vocid_t is_word(const char *);
void save_vocab_index();
void hash_stats();
void set_version_string();
container_t add_container(basic_obj self, basic_obj container);
bool is_inside(basic_obj item, basic_obj container);
basic_obj add_basic_obj(struct BASIC_OBJ *ptr, char type, flag_t flags);
void save_basic_objs();
basic_obj is_bob(const char *name, char type = -1);
basic_obj is_container(const char *name);
signed int handle_std_flag(const char *phrase, flag_t &flags, flag_t filter);

static inline char *
skipspc(char *s)
{
    while (*s && *s == 32)
        s++;
    return s;
}

#endif  // SMUGL_SMUGLCOM_PROTOS_H
