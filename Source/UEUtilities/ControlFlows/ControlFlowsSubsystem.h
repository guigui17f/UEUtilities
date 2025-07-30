// Copyright 2025 guigui17f. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ControlFlowNode.h"
#include "ControlFlowsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLevelInitProgress, FGuid, FlowId, float, Progress, FString, ResultMessage);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FLevelInitResult, FGuid, FlowId, bool, bResult, FString, ResultMessage);

DECLARE_MULTICAST_DELEGATE_TwoParams(FControlFlowsManagerResponse, bool, FString);

enum class EControlFlowsTaskStatus : uint8
{
	None,
	Success,
	Failed
};

enum class EControlFlowsManagerStatus :uint8
{
	None,
	Processing,
	Success,
	Failed
};

struct FControlFlowsTask : TSharedFromThis<FControlFlowsTask>
{
	FControlFlowsTask():
		TaskId(FGuid::NewGuid()),
		ManagerId(FGuid()),
		TaskStatus(EControlFlowsTaskStatus::None),
		ResultMessage(TEXT(""))
	{
	}

	virtual ~FControlFlowsTask()
	{
	}

	FGuid TaskId;
	FGuid ManagerId;
	EControlFlowsTaskStatus TaskStatus;
	FString ResultMessage;
};

struct FControlFlowsManager : TSharedFromThis<FControlFlowsManager>
{
	FControlFlowsManager():
		ManagerId(FGuid::NewGuid()),
		ManagerStatus(EControlFlowsManagerStatus::None),
		ResultMessage(TEXT("")),
		TaskNum(0)
	{
	}

	virtual ~FControlFlowsManager()
	{
	}

	FGuid ManagerId;
	EControlFlowsManagerStatus ManagerStatus;
	FString ResultMessage;
	int32 TaskNum;
	TArray<TSharedPtr<FControlFlowsTask>> Tasks;
	FControlFlowsManagerResponse ManagerResponse;

	EControlFlowsManagerStatus CheckManagerStatus()
	{
		if (TaskNum != Tasks.Num())
		{
			ManagerStatus = EControlFlowsManagerStatus::Processing;
			return ManagerStatus;
		}

		ManagerStatus = EControlFlowsManagerStatus::Success;
		ResultMessage = TEXT("");
		for (auto& TaskPtr : Tasks)
		{
			if (TaskPtr->TaskStatus == EControlFlowsTaskStatus::None)
			{
				ManagerStatus = EControlFlowsManagerStatus::Processing;
				continue;
			}
			if (TaskPtr->TaskStatus == EControlFlowsTaskStatus::Failed)
			{
				ResultMessage += FString::Printf(TEXT("manager %s task %s failed, message: %s\n"),
				                                 *ManagerId.ToString(), *TaskPtr->TaskId.ToString(), *TaskPtr->ResultMessage);
				if (ManagerStatus != EControlFlowsManagerStatus::Processing)
				{
					ManagerStatus = EControlFlowsManagerStatus::Failed;
				}
				continue;
			}
			ResultMessage += FString::Printf(TEXT("manager %s task %s success, message: %s\n"),
			                                 *ManagerId.ToString(), *TaskPtr->TaskId.ToString(), *TaskPtr->ResultMessage);
		}
		return ManagerStatus;
	}
};

/**
 * 
 */
UCLASS()
class UEUTILITIES_API UControlFlowsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable)
	void InitLevel();

	UPROPERTY(BlueprintAssignable)
	FLevelInitProgress InitProgress;

	UPROPERTY(BlueprintAssignable)
	FLevelInitResult InitResult;

protected:
	void InitLocalAssets(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage);

	void InitNetworkInfo(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage);

	void InitPlayerInfo(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage);

	void RefreshDisplay(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage);

	void FinishInitialize(FControlFlowNodeRef SubFlow, float Progress, FString StartMessage);

	void UpdateInitTaskStatus(FGuid ManagerId, FGuid TaskId, bool bResult, FString ResultMessage);

	void BroadcastInitProgress(float Progress, FString ResultMessage);

	void HandleInitFailed(FControlFlowNodeRef SubFlow);

	void CheckInitManagersStatus();

	bool bInitializing = false;

	FGuid InitFlowId;

	TMap<FGuid, TSharedPtr<FControlFlowsManager>> InitManagers;
};
