// Copyright Evianaive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNodeBase.h"
#include "AnimNode_CurveViewer.generated.h"

/**
 * 
 */
USTRUCT(BlueprintInternalUseOnly)
struct ANIMCURVEVIEWER_API FAnimNode_CurveViewer : public FAnimNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, EditFixedSize, BlueprintReadWrite, Category = Links)
	FPoseLink SourcePose;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, editfixedsize, Category = CurveViewer, meta = (PinShownByDefault))
	TArray<float> CurveValues;
	// Curve to view
	UPROPERTY()
	TArray<FName> CurveNames;
	
	// Whether to log curve value to log
	UPROPERTY(EditAnywhere, Category = CurveViewer)
	bool bLog{false};
	//View all active curve 
	UPROPERTY(EditAnywhere, Category = CurveViewer)
	bool bViewAll{false};

	FAnimNode_CurveViewer();

	// FAnimNode_Base interface
	virtual void Initialize_AnyThread(const FAnimationInitializeContext& Context) override;
	virtual void CacheBones_AnyThread(const FAnimationCacheBonesContext& Context) override;
	virtual void Evaluate_AnyThread(FPoseContext& Output) override;
	virtual void Update_AnyThread(const FAnimationUpdateContext& Context) override;
	// End of FAnimNode_Base interface

#if WITH_EDITOR
	/** Add new curve being modified */
	void AddCurve(const FName& InName, float InValue);
	/** Remove a curve from being modified */
	void RemoveCurve(int32 PoseIndex);
#endif // WITH_EDITOR
};