#ifndef AMUL_MSGS_H
#define AMUL_MSGS_H 1
/*
 ****    AMUL.MSGS.H.....Adventure Compiler    ****
 ****         system message #defines!         ****
 */

enum SysMsgNo {
    TITLE = 0,
    RESETTING,    /* Reset in progress */
    NOSLOTS,      /* No free slots */
    RETURN,       /* '[Press RETURN] ' */
    WHATNAME,     /* 'Persona name: ' */
    LENWRONG,     /* Must be between 3-20 chars long */
    NAME_USED,    /* When system already uses name */
    LOGINASME,    /* Message->user already logged in */
    ALREADYIN,    /* User is already logged in */
    CREATE,       /* Create new user? */
    WHATGENDER,   /* (M)ale or (F)emale? */
    GENDINVALID, /* Invalid, only M or F valid */
    ENTERPASSWD, /* Enter password: */
    PASLENWRONG, /* Invalid length! */
    ASK4ANSI,    /* Do you want ANSI codes? */
    YOUBEGIN,    /* You begin as... */
    TRIESOUT,    /* Out of tries (passwd failed) */
    FAILEDTRIES, /* No. of failed attempts */
    WELCOMEBAK,  /* Welcome back! */
    ANSION,      /* ANSI Enabled! */
    COMMENCED,   /* Message to others when you begin */
    HELP,        /* Available commands */
    BYEPASSWD,   /* Byebye, passwd is... */
    EXITED,      /* I've exited */
    DIED,        /* You have died */
    HEDIED,      /* @me just died */
    SPELLFAIL,   /* Spell failed */
    ISPLAYING,   /* is online... */
    MADERANK,    /* When you achieve a rank */
    SAVED,       /* When saving score */
    REALLYQUIT,  /* Really quit (y/N): */
    NOWTOODARK,  /* It is now too dark to see. */
    NOWLIGHT,    /* It is now light enough to see. */
    TOODARK,     /* It is too dark to see. */
    TOOMAKE,     /* Too dark to make anything out */
    NOWTSPECIAL, /* You can see nothing special. */
    ISHERE,      /* %s the %q is here... */
    RESETSTART,  /* Message when reset starts... */
    INVALIDVERB, /* Try using a verb... */
    CANTGO,      /* Can't go that way... */
    CANTDO,      /* Cant do that */
    SGO,         /* Kaapppoowwww! */
    SGOVANISH,   /* ->others when you SGOleave */
    SGOAPPEAR,   /* ->others when you SGOarrive */
    NONOUN,      /* Missing noun! */
    WORDMIX,     /* When your words are mixed up */
    ALMOST,      /* Almost understood */
    BEENSUMND,   /* You have been summoned */
    SUMVANISH,   /* @me vanishes... */
    SUMARRIVE,   /* @me arrives out of nowhere */
    WOKEN,       /* @me has just woken up. */
    IWOKEN,      /* You have just woken up. */
    CANTSUMN,    /* %s is already here! */
    LEFT,        /* @me has just left! */
    ARRIVED,     /* @me has just arrived. */
    NOROOM,      /* There is no room in there for you. */
    NOROOMIN,    /* @me just tried to enter, but there wasn't enuff room! */
    CHANGESEX,   /* You have magically become @gn! */
    ATTACK,      /*You strike @pl with your bare hands! */
    DEFEND,      /* You manage to block @me's strike. */
    WATTACK,     /*You strike @pl with your trusty @o1! */
    WDEFEND,     /*You wield your @o2 and block @me's strike. */
    AMHIT,       /* @me strikes you with his bare hands. */
    BLOCK,       /*@pl manages to block your strike. */
    WHIT,        /* @me strikes you with his @o1! */
    WBLOCK,      /*@pl wields a @o2 and blocks your attack! */
    MISSED,      /*You attack @pl but miss! */
    HEMISSED,    /*@me attacks you but misses! */
    TOPRANK,     /*Well done! You have obtained the highest rank possible! */
    NOTASK,      /*Sorry you haven't completed adequate tasks for this level. */
    YOURBLIND,   /*You can't see anything, you're blind. */
    WHO_HIDE,    /*You can't be sure if there are other people nearby... */
    CERTDEATH,   /*Exit description of Death room */
    NOFIGHT,     /*Sorry no fighting allowed here. */
    LOWLEVEL,    /*Sorry you're not high enough to cast that spell. */
    NOMAGIC,     /*Sorry you have not enough magic points for that spell. */
};

#endif
