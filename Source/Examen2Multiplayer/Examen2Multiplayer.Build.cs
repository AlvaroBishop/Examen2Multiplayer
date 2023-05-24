// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Examen2Multiplayer : ModuleRules
{
	public Examen2Multiplayer(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "HeadMountedDisplay", "EnhancedInput" });
	}
}
