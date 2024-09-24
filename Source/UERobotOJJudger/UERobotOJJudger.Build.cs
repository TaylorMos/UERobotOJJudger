// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class UERobotOJJudger : ModuleRules
{
	
	public UERobotOJJudger(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore","RenderCore","RHI" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

		LoadOpenCV(base.Target);
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
	private string ThirdPartyPath
	{
		get{return Path.GetFullPath(Path.Combine(ModuleDirectory,"../../ThirdParty"));}
	}
	public bool LoadOpenCV(ReadOnlyTargetRules Target)
	{
		//获得OpenCV第三方库的根路径
		string OpenCVPath = Path.Combine(ThirdPartyPath, "OpenCV");
		//Lib文件路径
		string LibPath = "";
		if (Target.Platform==UnrealTargetPlatform.Win64)
		{
			//引擎将会include该文件夹下的第三方库，如.hpp文件.
			PublicIncludePaths.AddRange(new string[] { Path.Combine(OpenCVPath, "Includes") });

			//指向ThirdpartyPath/Libraries/Win64/
			LibPath = Path.Combine(OpenCVPath, "Libraries", "Win64");

			//这将告诉引擎lib文件的路径
			PublicSystemLibraryPaths.Add(LibPath);
			//这将告诉引擎需要加载的lib文件的名称，他将去上面的lib文件路径下寻找我们提供的lib文件
			PublicAdditionalLibraries.Add("opencv_world4100.lib");
			PublicAdditionalLibraries.Add("opencv_img_hash4100.lib");
			/**
			 * dll需要拷贝到项目根目录/Binaries/当前平台/路径下
			 *运行时依赖，可以在打包时，自动将dll从源文件路径拷贝到目标文件路径
			 * RuntimeDependencies.Add(源文件路径，目标文件路径）
			 * $(BinaryOutputDir),表示Binaries/当前平台/路径
			 * 参考：https://docs.unrealengine.com/4.27/zh-CN/ProductionPipelines/BuildTools/UnrealBuildTool/ThirdPartyLibraries/
			 */
			RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)","opencv_world4100.dll"),Path.Combine(LibPath,"opencv_world4100.dll"));
			RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)","opencv_videoio_ffmpeg4100_64.dll"),Path.Combine(LibPath,"opencv_videoio_ffmpeg4100_64.dll"));
			RuntimeDependencies.Add(Path.Combine("$(BinaryOutputDir)","opencv_img_hash4100.dll"),Path.Combine(LibPath,"opencv_img_hash4100.dll"));
			return true;
		}
		return false;
	}
}
