// This may look like C, but it's really -*- C++ -*-
// $Id: cl_player.hpp,v 1.1 1999/06/08 15:36:45 oliver Exp $
//
////////////////////////////// PLAYER STRUCTURE
//

#ifndef PLAYER_H
#define PLAYER_H 1

#include "cl_basicobj.hpp"

class PLAYER : public BASIC_OBJ
    {
public:
	virtual ~PLAYER() {}

    //// Player::FUNCTIONS
    virtual int describe(void) override { return 0; };
    virtual int describe_verbose(void) override { return 0; }
    virtual int Write(FILE *) override;
    virtual int Read(FILE *) override;

    //// Player::DATA
    char _name[NAMEL + 1];      // Player's name
    char passwd[41];            // Password or User ID
    long score;                 // Score to date...
    RDMode rdmode;              // Users RD Mode
    counter_t plays;            // How many times they've played
    long tasks;
    long last_session;          // Start of session of last game
    long stamina;               // Stamina
    long dext;                  // Dexterity
    long wisdom;                // Wisdom
    long experience;            // Experience
    long magicpts;              // Magic points
    long rank;                  // How he rates!
    counter_t tries;            // Bad tries since last
    char pclass;                // Player class
    Gender sex;                 // Players sex
    char llen, slen;            // Screen Length/Width

    long sctg;                  // SCore This Game
    long rec;                   // My record no.
    short dextadj;              // Dexterity Adjustments
    long wield;                 // Current Weapon
    short light;                // If player has a light
    short hadlight;             // If I had a light
    u_long helping;             // Player I am helping
    u_char helped;              // Getting help from
    u_char following;           // Who I am following
    u_char followed;            // Who is following me
    u_long fighting;
    char pre[81];               // Pre-rank description
    char post[81];              // Post-rank description
    const char *arr;            // String when I arrive
    const char *dep;            // String when I depart
    };

#endif /* PLAYER_H */
