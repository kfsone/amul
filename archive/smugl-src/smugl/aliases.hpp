// $Id: aliases.hpp,v 1.4 1997/05/22 02:21:18 oliver Exp $
// alias class definitions and function protos

class Alias : public ALIAS
{
  public:
    static long locate(const char *s);
    static long locate(vocid_t id);
    static inline vocid_t meaning(long num) { return data->aliasbase[num].means; };
};
