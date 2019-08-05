// Object: Item or npc in the game world
struct _OBJ_STRUCT {
    char               id[IDL + 1];                   /// TODO: noun id
    int16_t            idno;                          /* Object's ID no	 */
    adjid_t            adj;                           /* Adjective. -1 = none	 */
    int16_t            inside;                        /* No. objects inside	 */
    int16_t            flags;                         /* Fixed flags		 */
    int32_t            contains;                      /* How much it will hold */
    int8_t             nstates;                       /* No. of states	 */
    int8_t             putto;                         /* Where things go	 */
    int8_t             state;                         /* Current state	 */
    int8_t             mobile;                        /* Mobile character	 */
    struct _OBJ_STATE *states;                        /* Ptr to states!	 */
    int16_t            nrooms;                        /* No. of rooms its in	 */
    roomid_t *         rmlist; /* List of rooms	 */  /// TODO: rmlist[]
};

// Object: State specific properties
struct _OBJ_STATE {
    uint32_t   weight;    // In grammes
    int32_t    value;     // Base points for dropping in a swamp
    uint16_t   strength;  // } Unclear: May be health of the item,
    uint16_t   damage;    // } damage it does or damage done to it
    stringid_t description;
    uint16_t   flags;
};

