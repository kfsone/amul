#include "amul.actions.h"
#include "amul.demonactions.h"
#include "amulinc.h"
#include "message.execdemon.h"
#include "parser.context.h"
#include "typedefs.h"

// Daemon processing bizness!

namespace Action::Demon
{

// Start a demon
void
Schedule(verbid_t verbId, int delay, bool global)
{
    // Immediate?
    if (delay == 0) {
        auto savedIverb = iverb;
        auto savedLverb = lverb;
        auto savedLdir = ldir;
        auto savedLroom = lroom;
        auto savedAdj1 = iadj1;
        auto savedAdj2 = iadj2;
        auto savedNoun1 = inoun1;
        auto savedNoun2 = inoun2;
        auto savedIprep = iprep;
        auto savedLastHim = last_him;
        auto savedLastHer = last_her;
        auto savedLastIt = last_it;
        WType savedWtypes[6];
        std::copy(std::begin(wtype), std::end(wtype), savedWtypes);

        lang_proc(verbId, 0);

        iverb = savedIverb;
        lverb = savedLverb;
        ldir = savedLdir;
        lroom = savedLroom;
        iadj1 = savedAdj1;
        iadj2 = savedAdj2;
        inoun1 = savedNoun1;
        inoun2 = savedNoun2;
        iprep = savedIprep;
        last_him = savedLastHim;
        last_her = savedLastHer;
        std::copy(std::begin(savedWtypes), std::end(savedWtypes), wtype);
    } else {
#ifdef MESSAGE_CODE
        amanp->p1 = inoun1;
        amanp->p2 = inoun2;
        amanp->p3 = wtype[2];
        amanp->p4 = wtype[5];
        SendIt(type, action, seconds);  // Inform AMAN...
#else
		(void)global;
#endif
    }
}

// Cancel a demon
void
Cancel(verbid_t verbId)
{
	///TODO: Implement
	(void)verbId;
}

// Force a demon to execute
void
ForceExecute(verbid_t verbId)
{
	///TODO: Implement
	(void)verbId;
}

int
Status(verbid_t verbId)
{
#ifdef MESSAGE_CODE
    SendIt(MCHECKD, d, nullptr);
    if (amul->data == -1) {
        Print("eventually");
        return;
    }
    FormatTimeInterval(block, amul->p1);
    Print(block);
#else
	(void)verbId;
#endif
    return -1;
}

}  // namespace Action::Demon
