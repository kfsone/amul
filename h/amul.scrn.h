#ifndef H_AMUL_SCRN_H
#define H_AMUL_SCRN_H 1

/* Amul custom screen bits! Pinched from ParFilEd!!!! SNIGGER! */

struct NewScreen NewScreenStructure = {
        0,
        0, /* screen XY origin relative to View */
        640,
        256, /* screen width and height */
        1,   /* screen depth (number of bitplanes) */
        0,
        1,            /* detail and block pens */
        HIRES,        /* display modes for this screen */
        CUSTOMSCREEN, /* screen type */
        NULL,         /* pointer to default screen font */
        NULL,         /* No screen title */
        NULL,         /* first in list of custom screen gadgets */
        NULL          /* pointer to custom BitMap structure */
};

#define NEWSCREENSTRUCTURE NewScreenStructure

struct NewWindow PFEWNewWindowStructure1 = {
        0,
        0, /* window XY origin relative to TopLeft of screen */
        640,
        255, /* window width and height */
        0,
        1,    /* detail and block pens */
        NULL, /* IDCMP flags */
        ACTIVATE + BACKDROP + GIMMEZEROZERO + RMBTRAP + BORDERLESS +
                NOCAREREFRESH, /* other window flags */
        NULL,                  /* first gadget in gadget list */
        NULL,                  /* custom CHECKMARK imagery */
        NULL,                  /* window title */
        NULL,                  /* custom screen pointer */
        NULL,                  /* custom bitmap */
        -1,
        -1, /* minimum width and height */
        -1,
        -1,          /* maximum width and height */
        CUSTOMSCREEN /* destination screen type */
};

USHORT Palette[] = {
        0x0000, /* color #0 */
        0x0FFF  /* color #1 */
#define PaletteColorCount 2
};

#define PALETTE Palette

#endif
