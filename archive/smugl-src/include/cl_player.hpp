#ifndef SMUGL_H_CL_PLAYER_H
#define SMUGL_H_CL_PLAYER_H 1

////////////////////////////// PLAYER STRUCTURE

#include "cl_basicobj.hpp"
#include "defines.hpp"

class PLAYER : public BASIC_OBJ
{
  public:
    ~PLAYER() override = default;

    //// Player::FUNCTIONS
    bool describe() override { return false; };
    bool describe_verbose() override { return false; }
    int Write(FILE *) override;
    int Read(FILE *) override;

    //// Player::DATA
    char _name[NAMEL + 1]{ 0 };  // Player's name
    char passwd[41]{ 0 };        // Password or User ID
    long score{ 0 };             // Score to date...
    RDMode rdmode;               // Users RD Mode
    counter_t plays{ 0 };        // How many times they've played
    long tasks{ 0 };
    time_t last_session{ 0 };   // Start of session of last game
    long stamina{ 0 };          // Stamina
    long dext{ 0 };             // Dexterity
    long wisdom{ 0 };           // Wisdom
    long experience{ 0 };       // Experience
    long magicpts{ 0 };         // Magic points
    long rank{ 0 };             // How he rates!
    counter_t tries{ 0 };       // Bad tries since last
    char pclass{ 0 };           // Player class
    Gender sex{ EITHER };       // Players sex
    char llen{ 0 }, slen{ 0 };  // Screen Length/Width

    long sctg{ 0 };              // SCore This Game
    long rec{ 0 };               // My record no.
    short dextadj{ 0 };          // Dexterity Adjustments
    long wield{ 0 };             // Current Weapon
    short light{ 0 };            // If player has a light
    short hadlight{ 0 };         // If I had a light
    playerid_t helping{ 0 };     // Player I am helping
    playerid_t following{ 0 };   // Who I am following
    playerid_t fighting{ 0 };    // player I'm trying to fight
    playermask_t helped{ 0 };    // Getting help from
    playermask_t followed{ 0 };  // Who is following me
    char pre[81]{ 0 };           // Pre-rank description
    char post[81]{ 0 };          // Post-rank description
    const char *arr{ nullptr };  // String when I arrive
    const char *dep{ nullptr };  // String when I depart
};

#endif  // PLAYER_H
