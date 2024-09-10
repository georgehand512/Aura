// Copyright GH Indie Developments


#include "AbilitySystem/Data/LevelUpInfo.h"

int32 ULevelUpInfo::FindLevelForXP(int32 XP) const
{
	// return XP level (L_250)
	int32 Level = 1;
	bool bSearching = true;

	while (bSearching)
	{	
		// LevelUpInformation[1] = Level 1 Information (array element)
		// LevelUpInformation[2] = Level 1 Information (array element)
		if (LevelUpInformation.Num() - 1 <= Level) return Level;

		if (XP >= LevelUpInformation[Level].LevelUpRequirement)
		{
			++Level;
		}
		else
		{
			bSearching = false;
		}
	}
	
	return Level;
}
