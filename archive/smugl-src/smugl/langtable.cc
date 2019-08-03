// This may look like C, but it's really -*- C++ -*-
// $Id: langtable.cc,v 1.2 1999/06/30 15:00:58 oliver Exp $
//
// language table entry processor. This houses all the condition and
// action code, and the tables used to invoke them

#include <cctype>

#include "aliases.hpp"
#include "io.hpp"
#include "lang.hpp"
#include "parser.hpp"
#include "smugl.hpp"
#include "structs.hpp"

bool
quit()
{
    char quityn[3];
    prompt(REALLYQUIT);
    fetch_input(quityn, 2);
    if (tolower(quityn[0]) == 'y') {
        exiting = ecQuit;
        return TRUE;
    }
    return FALSE;
}

void
look()
{
    cur_loc->describe();
}

bool
do_condition(VBTAB *vt, bool lastCond)
{
    bool result;

    switch (vt->condition) {
        case CANTEP:  // And .. endparse
        case CAND:    // And
            result = lastCond;
            break;

        case CALTEP:   // Always .. endparse
        case CSTAR:    // Always
        case CALWAYS:  // Always literally
            result = TRUE;
            break;

        case CELTEP:  // Else ... endparse
        case CELSE:   // Opposite of and
            result = !(lastCond);
            break;

        case CLIGHT:       // If the room is illuminated
        case CISHERE:      // If given item is present
        case CMYRANK:      // If my rank equates to
        case CSTATE:       // If object is in a specified state
        case CMYSEX:       // If my gender is
        case CLASTVB:      // If last verb used was
        case CLASTDIR:     // If last travel verb was
        case CLASTROOM:    // If the last room I was in was
        case CASLEEP:      // If the player is sleeping
        case CSITTING:     // If the player is sitting
        case CLYING:       // If the player is lying down
        case CRAND:        // Compare two random numbers
        case CRDMODE:      // Based on 'room description' mode
        case CONLYUSER:    // If I'm the only person in game
        case CALONE:       // Only player present in location
        case CINROOM:      // If I am in a given location
        case COPENS:       // If the specified object opens
        case CGOTNOWT:     // Carrying nothing
        case CCARRYING:    // Am I carrying said item
        case CNEARTO:      // Is it carried/in room
        case CHIDDEN:      // Am I hidden from other players
        case CCANGIVE:     // Can this item be given to the other player
        case CINFL:        // Is player inflicted with X
        case CINFLICTED:   // Is player inflicted with X
        case CSAMEROOM:    // Same room as player?
        case CSOMEONEHAS:  // If obj is being carried
        case CTOPRANK:     // If you are a top-rank player
        case CGOTA:        // Are you carrying an object in state N
        case CACTIVE:      // Is the specified daemon active?
        case CTIMER:       // Check time left on given daemon
        case CBURNS:       // If object is flamable
        case CCONTAINER:   // If object is a container
        case CEMPTY:       // If object is empty
        case COBJSIN:      // Number of objects in item
        case CHELPING:     // If we are helping player X
        case CGOTHELP:     // If someone is helping us
        case CANYHELP:     // Are we helping ANYONE
        case CSTAT:        // If attrib <> no
        case COBJINV:      // If object is invisible
        case CVISIBLETO:   // Is player <X> visible to me
        case CFIGHTING:    // Is player fighting someone
        case CTASKSET:     // Has player done this task
        case CCANSEE:      // Can I see player <X>
        case CNOUN1:       // Match noun 1
        case CNOUN2:       // Match noun 2
        case CAUTOEXITS:   // Auto exits on?
        case CDEBUG:       // Debug mode on?
        case CFULL:        // Stat at max?
        case CTIME:        // Time remaining?
        case CDEC:         // Decrement and test obj state
        case CINC:         // Increment and test state
        case CLIT:         // Is object lit?
        case CFIRE:        // Is object flamable?
        case CHEALTH:      // Is player's health %?
        case CMAGIC:       // Can magic be done?
        case CSPELL:       // Can spell be done?
        case CIN:          // In <room> <noun>
        case CEXISTS:      // Does object exist (in play)?
        case CWILLGO:      // Will object fit inside container?
            tx("Alas, SMUGL does not yet know how to do this. Be patient\n");
            result = FALSE;
            break;

        default:
            if (debug)
                txprintf("Unknown condition number %d\n", vt->condition);
            result = FALSE;
            break;
    }

    if (!vt->not_condition)
        return result;
    else
        return (!(result));
}

slotResult
do_action(VBTAB *vt, bool /*lastCond*/)
{
    assert(vt->action_type == ACT_DO);

    if (debug)
        txprintf("trying action [%ld]\n", vt->action);

    switch (vt->action) {
        case AQUIT:
            if (quit())
                return slotProcessed;
            break;

        case ALOOK:
            look();
            return slotProcessed;

        case APRINT:
            tx(message(vt->pptr[0]));
            return slotProcessed;

        default:
            if (debug > 1)
                txprintf("Unimplemented action %d\n", vt->action);
    }
    return slotFailed;
}
