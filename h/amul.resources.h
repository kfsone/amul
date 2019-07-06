#pragma once

#define COMPILED_SUFFIX ".amulc"

namespace Resources {
	namespace Compiled {
		static inline const char *gameProfile() noexcept { return "game" COMPILED_SUFFIX; }
		static inline const char *roomDesc() noexcept { return "roomdesc" COMPILED_SUFFIX; }
		static inline const char *roomData() noexcept { return "roomdata" COMPILED_SUFFIX; }
		static inline const char *rankData() noexcept { return "rankdata" COMPILED_SUFFIX; }
		static inline const char *travelTable() noexcept { return "travelentries" COMPILED_SUFFIX; }
		static inline const char *travelParameters() noexcept { return "travelparams" COMPILED_SUFFIX; }
		static inline const char *lang1() noexcept { return "lang1" COMPILED_SUFFIX; }
		static inline const char *lang2() noexcept { return "lang2" COMPILED_SUFFIX; }
		static inline const char *lang3() noexcept { return "lang3" COMPILED_SUFFIX; }
		static inline const char *lang4() noexcept { return "lang4" COMPILED_SUFFIX; }
		static inline const char *synonymData() noexcept { return "syndata" COMPILED_SUFFIX; }
		static inline const char *synonymIndex() noexcept { return "synindex" COMPILED_SUFFIX; }
		static inline const char *objData() noexcept { return "objdata" COMPILED_SUFFIX; }
		static inline const char *objDesc() noexcept { return "objdesc" COMPILED_SUFFIX; }
		static inline const char *objLoc() noexcept { return "objloc" COMPILED_SUFFIX; }
		static inline const char *objState() noexcept { return "objstate" COMPILED_SUFFIX; }
		static inline const char *nounTable() noexcept { return "noun" COMPILED_SUFFIX; }
		static inline const char *adjTable() noexcept { return "adj" COMPILED_SUFFIX; }
		static inline const char *mobData() noexcept { return "mobdata" COMPILED_SUFFIX; }
		static inline const char *mobCmd() noexcept { return "mobcmd" COMPILED_SUFFIX; }
}}
