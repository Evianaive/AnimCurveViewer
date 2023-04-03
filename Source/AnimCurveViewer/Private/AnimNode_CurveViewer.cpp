// Copyright Evianaive, Inc. All Rights Reserved.


#include "AnimNode_CurveViewer.h"
#include "Animation/AnimInstanceProxy.h"

FAnimNode_CurveViewer::FAnimNode_CurveViewer()
{
}

void FAnimNode_CurveViewer::Initialize_AnyThread(const FAnimationInitializeContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Initialize_AnyThread)
	Super::Initialize_AnyThread(Context);
	SourcePose.Initialize(Context);
}

void FAnimNode_CurveViewer::CacheBones_AnyThread(const FAnimationCacheBonesContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(CacheBones_AnyThread)
	Super::CacheBones_AnyThread(Context);
	SourcePose.CacheBones(Context);
}

void FAnimNode_CurveViewer::Evaluate_AnyThread(FPoseContext& Output)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Evaluate_AnyThread)
	FPoseContext SourceData(Output);
	SourcePose.Evaluate(SourceData);

	const USkeleton* Skeleton = Output.AnimInstanceProxy->GetSkeleton();
	
	Output = SourceData;
	if(bLog)
	{
		UE_LOG(LogTemp,Log,TEXT("NodeId : %d"),Output.GetCurrentNodeId());
	}
	if(bViewAll)
	{
		CurveNames.Reset();
		CurveValues.Reset();
		if(const FSmartNameMapping* RequestedMapping = Skeleton->GetSmartNameContainer(Skeleton->AnimCurveMappingName))
		{
			for (TConstSetBitIterator<decltype(Output.Curve)::Allocator> It(Output.Curve.ValidCurveWeights); It; ++It)
			{
				const int32 Idx = It.GetIndex();
				if (Output.Curve.ValidCurveWeights[Idx])
				{
					FName& CurveName = CurveNames.AddDefaulted_GetRef();
					RequestedMapping->GetName(Idx,CurveName);
					CurveValues.Add(Output.Curve.CurveWeights[Idx]);
				}
			}
		}
	}
	else
	{
		for (int32 ModIdx = 0; ModIdx < CurveNames.Num(); ModIdx++)
		{
			FName CurveName = CurveNames[ModIdx];
			const SmartName::UID_Type NameUID = Skeleton->GetUIDByName(USkeleton::AnimCurveMappingName, CurveName);
			if (NameUID != SmartName::MaxUID)
			{
				FString LogInfo;
				if (Output.Curve.ValidCurveWeights[NameUID])
				{
					const float CurrentValue = Output.Curve.Get(NameUID);
					CurveValues[ModIdx] = CurrentValue;
					LogInfo = FString::Printf(TEXT("%s : %.2f"),ToCStr(CurveName.ToString()),CurrentValue);
				}
				else
				{
					CurveValues[ModIdx] = -MAX_FLT;
					LogInfo = FString::Printf(TEXT("%s : Not Valid"),ToCStr(CurveName.ToString()));
				}
				if(bLog)
				{
					UE_LOG(LogTemp,Log,TEXT("%s"),ToCStr(LogInfo));
				}
			}
		}
	}
}

void FAnimNode_CurveViewer::Update_AnyThread(const FAnimationUpdateContext& Context)
{
	DECLARE_SCOPE_HIERARCHICAL_COUNTER_ANIMNODE(Update_AnyThread)

	// Run update on input pose nodes
	SourcePose.Update(Context);

	// Evaluate any BP logic plugged into this node
	GetEvaluateGraphExposedInputs().Execute(Context);
}

#if WITH_EDITOR

void FAnimNode_CurveViewer::AddCurve(const FName& InName, float InValue)
{
	CurveValues.Add(InValue);
	CurveNames.Add(InName);
}

void FAnimNode_CurveViewer::RemoveCurve(int32 PoseIndex)
{
	CurveValues.RemoveAt(PoseIndex);
	CurveNames.RemoveAt(PoseIndex);
}

#endif // WITH_EDITOR
