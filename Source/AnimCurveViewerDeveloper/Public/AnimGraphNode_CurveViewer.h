// Copyright Evianaive, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AnimGraphNode_Base.h"
#include "AnimNode_CurveViewer.h"
#include "AnimGraphNode_CurveViewer.generated.h"

/**
 * 
 */
UCLASS()
class ANIMCURVEVIEWERDEVELOPER_API UAnimGraphNode_CurveViewer : public UAnimGraphNode_Base
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category=Settings)
	FAnimNode_CurveViewer Node;
	
public:
	UAnimGraphNode_CurveViewer();

	// UEdGraphNode interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual TSharedPtr<SGraphNode> CreateVisualWidget() override;
	// End of UEdGraphNode interface

	// UAnimGraphNode_Base interface
	virtual FString GetNodeCategory() const override;
	virtual void CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const override;
	// End of UAnimGraphNode_Base interface

	// UK2Node interface
	virtual void GetNodeContextMenuActions(class UToolMenu* Menu, class UGraphNodeContextMenuContext* Context) const override;
	// End of UK2Node interface

	private:
	/** Remove a curve pin with the given name */
	void RemoveCurvePin(FName CurveName);
	/** Add a curve pin with the given name */
	void AddCurvePin(FName CurveName);
	/** Create submenu with options for curves to add */
	void GetAddCurveMenuActions(FMenuBuilder& MenuBuilder) const;
	/** Create submenu with options for curves to remove */
	void GetRemoveCurveMenuActions(FMenuBuilder& MenuBuilder) const;
	/** Returns list of curves we have not added yet */
	TArray<FName> GetCurvesToAdd() const;
};
