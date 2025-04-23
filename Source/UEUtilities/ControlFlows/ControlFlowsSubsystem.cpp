// Copyright guigui17f.

#include "ControlFlow.h"
#include "ControlFlowManager.h"
#include "ControlFlowsSubsystem.h"

bool UControlFlowsSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	return Super::ShouldCreateSubsystem(Outer);
}

void UControlFlowsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UControlFlowsSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UControlFlowsSubsystem::InitLevel()
{
	if (bInitializing)
	{
		return;
	}
	bInitializing = true;
	InitFlowId = FGuid::NewGuid();

	FControlFlow& Flow = FControlFlowStatics::Create(this, TEXT("LevelInitFlow"));
	Flow.QueueStep(TEXT("InitLocalAssets"), this, &UControlFlowsSubsystem::InitLocalAssets, 0.2f, FString(TEXT("initialize local assets")))
	    .QueueStep(TEXT("InitNetworkInfo"), this, &UControlFlowsSubsystem::InitNetworkInfo, 0.4f, FString(TEXT("initialize network information")))
	    .QueueStep(TEXT("InitPlayerInfo"), this, &UControlFlowsSubsystem::InitPlayerInfo, 0.6f, FString(TEXT("initialize player information")))
	    .QueueStep(TEXT("RefreshDisplay"), this, &UControlFlowsSubsystem::RefreshDisplay, 0.8f, FString(TEXT("refresh display")))
	    .QueueStep(TEXT("FinishInitialize"), this, &UControlFlowsSubsystem::FinishInitialize, 1.f, FString(TEXT("finish initialize")));
	Flow.ExecuteFlow();
}

void UControlFlowsSubsystem::InitLocalAssets(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage)
{
	UE_LOG(LogTemp, Display, TEXT("frame %llu: %s"), GFrameCounter, *StartMessage);

	TSharedPtr<FControlFlowsManager> ManagerPtr = MakeShared<FControlFlowsManager>();
	ManagerPtr->TaskNum = 1;
	ManagerPtr->ManagerResponse.AddLambda([this,SubFlow,Progress](bool bResult, FString ResultMessage)
	{
		UE_LOG(LogTemp, Display, TEXT("InitLocalAssets result: %s, message: %s"), bResult ? TEXT("true") : TEXT("false"), *ResultMessage);
		if (bResult)
		{
			BroadcastInitProgress(Progress, ResultMessage);
			SubFlow->ContinueFlow();
		}
		else
		{
			HandleInitFailed(SubFlow);
		}
	});
	FGuid ManagerId = ManagerPtr->ManagerId;
	InitManagers.Emplace(ManagerId, ManagerPtr);

	TSharedPtr<FControlFlowsTask> TaskPtr = MakeShared<FControlFlowsTask>();
	TaskPtr->ManagerId = ManagerId;
	ManagerPtr->Tasks.Emplace(TaskPtr);
	FGuid TaskId = TaskPtr->TaskId;
	AsyncTask(ENamedThreads::AnyThread, [this, ManagerId, TaskId]
	{
		//do initialize work

		bool bTaskResult = true;
		FString ResultMessage = TEXT("");
		AsyncTask(ENamedThreads::GameThread, [this,ManagerId, TaskId, bTaskResult,ResultMessage]
		{
			UpdateInitTaskStatus(ManagerId, TaskId, bTaskResult, ResultMessage);
		});
	});
}

void UControlFlowsSubsystem::InitNetworkInfo(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage)
{
	UE_LOG(LogTemp, Display, TEXT("frame %llu: %s"), GFrameCounter, *StartMessage);

	TSharedPtr<FControlFlowsManager> ManagerPtr = MakeShared<FControlFlowsManager>();
	ManagerPtr->TaskNum = 1;
	ManagerPtr->ManagerResponse.AddLambda([this,SubFlow,Progress](bool bResult, FString ResultMessage)
	{
		UE_LOG(LogTemp, Display, TEXT("InitNetworkInfo result: %s, message: %s"), bResult ? TEXT("true") : TEXT("false"), *ResultMessage);
		if (bResult)
		{
			BroadcastInitProgress(Progress, ResultMessage);
			SubFlow->ContinueFlow();
		}
		else
		{
			HandleInitFailed(SubFlow);
		}
	});
	FGuid ManagerId = ManagerPtr->ManagerId;
	InitManagers.Emplace(ManagerId, ManagerPtr);

	TSharedPtr<FControlFlowsTask> TaskPtr = MakeShared<FControlFlowsTask>();
	TaskPtr->ManagerId = ManagerId;
	ManagerPtr->Tasks.Emplace(TaskPtr);
	FGuid TaskId = TaskPtr->TaskId;
	AsyncTask(ENamedThreads::AnyThread, [this, ManagerId, TaskId]
	{
		//do initialize work

		bool bTaskResult = true;
		FString ResultMessage = TEXT("");
		AsyncTask(ENamedThreads::GameThread, [this,ManagerId, TaskId, bTaskResult,ResultMessage]
		{
			UpdateInitTaskStatus(ManagerId, TaskId, bTaskResult, ResultMessage);
		});
	});
}

void UControlFlowsSubsystem::InitPlayerInfo(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage)
{
	UE_LOG(LogTemp, Display, TEXT("frame %llu: %s"), GFrameCounter, *StartMessage);

	TSharedPtr<FControlFlowsManager> ManagerPtr = MakeShared<FControlFlowsManager>();
	ManagerPtr->TaskNum = 1;
	ManagerPtr->ManagerResponse.AddLambda([this,SubFlow,Progress](bool bResult, FString ResultMessage)
	{
		UE_LOG(LogTemp, Display, TEXT("InitPlayerInfo result: %s, message: %s"), bResult ? TEXT("true") : TEXT("false"), *ResultMessage);
		if (bResult)
		{
			BroadcastInitProgress(Progress, ResultMessage);
			SubFlow->ContinueFlow();
		}
		else
		{
			HandleInitFailed(SubFlow);
		}
	});
	FGuid ManagerId = ManagerPtr->ManagerId;
	InitManagers.Emplace(ManagerId, ManagerPtr);

	TSharedPtr<FControlFlowsTask> TaskPtr = MakeShared<FControlFlowsTask>();
	TaskPtr->ManagerId = ManagerId;
	ManagerPtr->Tasks.Emplace(TaskPtr);
	FGuid TaskId = TaskPtr->TaskId;
	AsyncTask(ENamedThreads::AnyThread, [this, ManagerId, TaskId]
	{
		//do initialize work

		bool bTaskResult = true;
		FString ResultMessage = TEXT("");
		AsyncTask(ENamedThreads::GameThread, [this,ManagerId, TaskId, bTaskResult,ResultMessage]
		{
			UpdateInitTaskStatus(ManagerId, TaskId, bTaskResult, ResultMessage);
		});
	});
}

void UControlFlowsSubsystem::RefreshDisplay(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage)
{
	UE_LOG(LogTemp, Display, TEXT("frame %llu: %s"), GFrameCounter, *StartMessage);
	//do initialize work

	FString ResultMessage = TEXT("");
	InitProgress.Broadcast(InitFlowId, Progress, ResultMessage);
	SubFlow->ContinueFlow();
}

void UControlFlowsSubsystem::FinishInitialize(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage)
{
	UE_LOG(LogTemp, Display, TEXT("frame %llu: %s"), GFrameCounter, *StartMessage);
	//do finish work

	FString ResultMessage = TEXT("");
	InitProgress.Broadcast(InitFlowId, Progress, ResultMessage);
	SubFlow->ContinueFlow();
	InitResult.Broadcast(InitFlowId, true, TEXT("initialization success"));
	bInitializing = false;
	InitFlowId = FGuid();
}

void UControlFlowsSubsystem::UpdateInitTaskStatus(FGuid ManagerId, FGuid TaskId, bool bResult, FString ResultMessage)
{
	TSharedPtr<FControlFlowsManager>* ManagerPtrPointer = InitManagers.Find(ManagerId);
	if (ManagerPtrPointer)
	{
		TSharedPtr<FControlFlowsManager> ManagerPtr = *ManagerPtrPointer;
		for (auto& TaskPtr : ManagerPtr->Tasks)
		{
			if (TaskPtr->TaskId == TaskId)
			{
				TaskPtr->TaskStatus = bResult ? EControlFlowsTaskStatus::Success : EControlFlowsTaskStatus::Failed;
				TaskPtr->ResultMessage = ResultMessage;
				break;
			}
		}
		ManagerPtr->CheckManagerStatus();
		CheckInitManagersStatus();
	}
}

void UControlFlowsSubsystem::CheckInitManagersStatus()
{
	TArray<FGuid> FinishedManagerIds;
	for (auto& ManagerPair : InitManagers)
	{
		if (ManagerPair.Value->TaskNum != ManagerPair.Value->Tasks.Num())
		{
			continue;
		}
		if (ManagerPair.Value->ManagerStatus == EControlFlowsManagerStatus::Success || ManagerPair.Value->ManagerStatus == EControlFlowsManagerStatus::Failed)
		{
			FinishedManagerIds.Emplace(ManagerPair.Key);
		}
	}

	for (auto& ManagerId : FinishedManagerIds)
	{
		TSharedPtr<FControlFlowsManager> ManagerPtr = InitManagers[ManagerId];
		ManagerPtr->ManagerResponse.Broadcast(ManagerPtr->ManagerStatus == EControlFlowsManagerStatus::Success, ManagerPtr->ResultMessage);
		InitManagers.Remove(ManagerId);
	}
}

void UControlFlowsSubsystem::BroadcastInitProgress(float Progress, FString ResultMessage)
{
	AsyncTask(ENamedThreads::GameThread, [this, Progress, ResultMessage]
	{
		InitProgress.Broadcast(InitFlowId, Progress, ResultMessage);
	});
}

void UControlFlowsSubsystem::HandleInitFailed(FControlFlowNodeRef SubFlow)
{
	AsyncTask(ENamedThreads::GameThread, [this,SubFlow]
	{
		SubFlow->CancelFlow();
		InitResult.Broadcast(InitFlowId, false, TEXT("initialization failed"));
		bInitializing = false;
		InitFlowId = FGuid();
	});
}
