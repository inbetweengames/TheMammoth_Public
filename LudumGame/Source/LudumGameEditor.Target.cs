// copyright 2015 inbetweengames GBR

using UnrealBuildTool;
using System.Collections.Generic;

public class LudumGameEditorTarget : TargetRules
{
	public LudumGameEditorTarget(TargetInfo Target)
	{
		Type = TargetType.Editor;
	}

	//
	// TargetRules interface.
	//

	public override void SetupBinaries(
		TargetInfo Target,
		ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
		ref List<string> OutExtraModuleNames
		)
	{
		OutExtraModuleNames.AddRange( new string[] { "LudumGame" } );
	}
}
