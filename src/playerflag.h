#pragma once
#ifndef AMUL_PLAYERFLAG_H
#define AMUL_PLAYERFLAG_H

#define PFINVIS 0x00001     // Player invisible
#define PFGLOW 0x00002      // Player glowing
#define PFBLIND 0x00004     // Blind
#define PFDUMB 0x00008      // Can't speak
#define PFDEAF 0x00010      // Player's deaf
#define PFCRIP 0x00020      // Can't move
#define PFDYING 0x00040     // Player is dying
#define PFLIMP 0x00080      // Limping
#define PFASLEEP 0x00100    // Sleeping
#define PFSITTING 0x00200   // Sitting down
#define PFLYING 0x00400     // Lying Down
#define PFFIGHT 0x00800     // Fighting
#define PFATTACKER 0x01000  // If you started the fight
#define PFMOVING 0x02000    // If you are 'in transit'
#define PFSINVIS 0x04000    // Player is Super Invis

#endif  // AMUL_PLAYERFLAG_H