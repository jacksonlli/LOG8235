// Copyright 1998-2015 Epic Games, Inc. All Rights Reserved.

#ifndef __SOFTDESIGNTRAINING_H__
#define __SOFTDESIGNTRAINING_H__

#include "EngineMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSoftDesignTraining, Log, All);

class SoftDesignTrainingModuleImpl : public FDefaultGameModuleImpl
{
public:
    virtual void StartupModule();

    static void WorldTickStart(UWorld*, ELevelTick, float);

    static void WorldBeginPlay();

    static void WorldPostActorTick(UWorld* /*World*/, ELevelTick/**Tick Type*/, float/**Delta Seconds*/);

    static void WorldCleanup(UWorld*, bool, bool);

    virtual void ShutdownModule();
};

#endif
