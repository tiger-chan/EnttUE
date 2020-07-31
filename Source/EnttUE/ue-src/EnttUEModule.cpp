// Copyright Epic Games, Inc. All Rights Reserved.

#include "EnttUEModule.h"
#include "Core.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FEnttUEModule"

void FEnttUEModule::StartupModule()
{
}

void FEnttUEModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEnttUEModule, EnttUE)
