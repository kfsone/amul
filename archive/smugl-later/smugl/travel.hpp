#ifndef SMUGL_SMUGL_TRAVEL_H
#define SMUGL_SMUGL_TRAVEL_H

class TTEnt final : public TT_ENT
{
  public:
    bool describe();
};

class TTIdx final
{
  public:
    static class TTEnt *locate(char *s);
    static class TTEnt *locate(long id);
};

#endif  // SMUGL_SMUGL_TRAVEL_H
