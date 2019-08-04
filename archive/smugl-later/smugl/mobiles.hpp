#ifndef MOBILES_H
#define MOBILES_H 1

class Mobile : public MOB_ENT
{
  public:
    bool describe();               // Not really apropriate
    inline class Room *dmoveRm();  // What room should we dmove to?
};

class MobileIdx
{
  public:
    static class Mobile *locate(char *s);
    static class Mobile *locate(long id);
};

#endif  // MOBILES_H
