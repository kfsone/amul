#ifndef AMUL_USERS_H
#define AMUL_USERS_H

#include <string>
#include <string_view>

#include "amul.typedefs.h"

error_t LoadPlayers(string_view playerDataFile);
void PlayerLogin();

void SaveCharacter();
void GetCRLFUse();
void ChooseAnsiMode();
void GetLineLength();
void GetScreenLength();
void GetRedoCharacter();

void SlashCommands(std::string cmd);

#endif  // AMUL_USERS_H
