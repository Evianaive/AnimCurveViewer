// Copyright Evianaive, Inc. All Rights Reserved.

#include "AnimGraphNode_CurveViewer.h"
#include "Textures/SlateIcon.h"
#include "ScopedTransaction.h"
#include "Kismet2/CompilerResultsLog.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "SGraphNode.h"
#include "Framework/Commands/UIAction.h"
#include "ToolMenus.h"
#include "BoneControllers/AnimNode_ObserveBone.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetDebugUtilities.h"
#include "KismetNodes/KismetNodeInfoContext.h"

#define LOCTEXT_NAMESPACE "CurveViewer"

class SGraphNode_CurveViewer : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNode_CurveViewer){}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UAnimGraphNode_CurveViewer* InNode)
	{
		GraphNode = InNode;
		SetCursor(EMouseCursor::CardinalCross);
		UpdateGraphNode();
	}
	virtual void UpdateGraphNode() override
	{
		SGraphNode::UpdateGraphNode();

		// Remove the comment bubble slot
		RemoveSlot(ENodeZone::TopCenter);
	}

	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override
	{
		constexpr FLinearColor TimelineBubbleColor(1.0f, 0.1f, 0.1f, 0.5f);
		const FKismetNodeInfoContext* K2Context = static_cast<FKismetNodeInfoContext*>(Context);

		// Display the observed bone transform status bubble
		if (UObject* ActiveObject = K2Context->ActiveObjectBeingDebugged)
		{
			FProperty* NodeProperty = FKismetDebugUtilities::FindClassPropertyForNode(K2Context->SourceBlueprint, GraphNode);
			if (const FStructProperty* StructProperty = CastField<FStructProperty>(NodeProperty))
			{
				UClass* ContainingClass = StructProperty->GetTypedOwner<UClass>();
				if (ActiveObject->IsA(ContainingClass) && (StructProperty->Struct == FAnimNode_CurveViewer::StaticStruct()))
				{
					FAnimNode_CurveViewer* ViewCurve = StructProperty->ContainerPtrToValuePtr<FAnimNode_CurveViewer>(ActiveObject);

					check(ViewCurve);
					FString Message;
					for (int32 i = 0; i < ViewCurve->CurveNames.Num(); ++i)
					{
						if(ViewCurve->CurveValues[i] == -MAX_FLT)
						{
							Message += FString::Printf(TEXT("%s : Not Valid\n"),ToCStr(ViewCurve->CurveNames[i].ToString()));
						}
						else
						{
							Message += FString::Printf(TEXT("%s : %f\n"),ToCStr(ViewCurve->CurveNames[i].ToString()),ViewCurve->CurveValues[i]);
						}
					}

					new (Popups) FGraphInformationPopupInfo(NULL, TimelineBubbleColor, Message);
				}
				else
				{
					const FString ErrorText = FText::Format(LOCTEXT("StaleDebugDataFmt", "Stale debug data\nProperty is on {0}\nDebugging a {1}"), FText::FromString(ContainingClass->GetName()), FText::FromString(ActiveObject->GetClass()->GetName())).ToString();
					new (Popups) FGraphInformationPopupInfo(NULL, TimelineBubbleColor, ErrorText);
				}
			}
		}

		SGraphNode::GetNodeInfoPopups(Context, Popups);
	}
};

UAnimGraphNode_CurveViewer::UAnimGraphNode_CurveViewer() 
{
}

FString UAnimGraphNode_CurveViewer::GetNodeCategory() const
{
	return TEXT("Debug Nodes");
}

FText UAnimGraphNode_CurveViewer::GetTooltipText() const
{
	return GetNodeTitle(ENodeTitleType::ListView);
}

FText UAnimGraphNode_CurveViewer::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("AnimGraphNode_CurveViewer_Title", "Curve Viewer");
}

TSharedPtr<SGraphNode> UAnimGraphNode_CurveViewer::CreateVisualWidget()
{
	return SNew(SGraphNode_CurveViewer, this);
}


TArray<FName> UAnimGraphNode_CurveViewer::GetCurvesToAdd() const
{
	TArray<FName> CurvesToAdd;

	if (const FSmartNameMapping* Mapping = GetAnimBlueprint()->TargetSkeleton->GetSmartNameContainer(USkeleton::AnimCurveMappingName))
	{
		Mapping->FillNameArray(CurvesToAdd);

		for (FName ExistingCurveName : Node.CurveNames)
		{
			CurvesToAdd.RemoveSingleSwap(ExistingCurveName, false);
		}

		CurvesToAdd.Sort(FNameLexicalLess());
	}

	return CurvesToAdd;
}

void UAnimGraphNode_CurveViewer::GetAddCurveMenuActions(FMenuBuilder& MenuBuilder) const
{
	TArray<FName> CurvesToAdd = GetCurvesToAdd();
	for (FName CurveName : CurvesToAdd)
	{
		FUIAction Action = FUIAction(FExecuteAction::CreateUObject(const_cast<UAnimGraphNode_CurveViewer*>(this), &UAnimGraphNode_CurveViewer::AddCurvePin, CurveName));
		MenuBuilder.AddMenuEntry(FText::FromName(CurveName), FText::GetEmpty(), FSlateIcon(), Action);
	}
}

void UAnimGraphNode_CurveViewer::GetRemoveCurveMenuActions(FMenuBuilder& MenuBuilder) const
{
	for (FName CurveName : Node.CurveNames)
	{
		FUIAction Action = FUIAction(FExecuteAction::CreateUObject(const_cast<UAnimGraphNode_CurveViewer*>(this), &UAnimGraphNode_CurveViewer::RemoveCurvePin, CurveName));
		MenuBuilder.AddMenuEntry(FText::FromName(CurveName), FText::GetEmpty(), FSlateIcon(), Action);
	}
}


void UAnimGraphNode_CurveViewer::GetNodeContextMenuActions(UToolMenu* Menu, UGraphNodeContextMenuContext* Context) const
{
	if (!Context->bIsDebugging)
	{
		FToolMenuSection& Section = Menu->AddSection("AnimGraphNodeCurveViewer", LOCTEXT("CurveViewer", "Curve Viewer"));

		// Clicked pin
		if (Context->Pin != NULL)
		{
			// Get property from pin
			FProperty* AssociatedProperty;
			int32 ArrayIndex;
			GetPinAssociatedProperty(GetFNodeType(), Context->Pin, /*out*/ AssociatedProperty, /*out*/ ArrayIndex);

			if (AssociatedProperty != nullptr)
			{
				const FName PinPropertyName = AssociatedProperty->GetFName();

				if (PinPropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_CurveViewer, CurveValues) && Context->Pin->Direction == EGPD_Input)
				{
					const FString PinName = Context->Pin->PinFriendlyName.ToString();
					const FUIAction Action = FUIAction(FExecuteAction::CreateUObject(const_cast<UAnimGraphNode_CurveViewer*>(this), &UAnimGraphNode_CurveViewer::RemoveCurvePin, FName(*PinName)));
					const FText RemovePinLabelText = FText::Format(LOCTEXT("RemoveThisPin", "Remove This Curve Pin: {0}"), FText::FromString(PinName));
					Section.AddMenuEntry("RemoveThisPin", RemovePinLabelText, LOCTEXT("RemoveThisPinTooltip", "Remove this curve pin from this node"), FSlateIcon(), Action);
				}
			}
		}

		// If we have more curves to add, create submenu to offer them
		if (GetCurvesToAdd().Num() > 0)
		{
			Section.AddSubMenu(
				"AddCurvePin",
				LOCTEXT("AddCurvePin", "Add Curve Pin"),
				LOCTEXT("AddCurvePinTooltip", "Add a new pin to drive a curve"),
				FNewMenuDelegate::CreateUObject(this, &UAnimGraphNode_CurveViewer::GetAddCurveMenuActions));
		}

		// If we have curves to remove, create submenu to offer them
		if (Node.CurveNames.Num() > 0)
		{
			Section.AddSubMenu(
				"RemoveCurvePin",
				LOCTEXT("RemoveCurvePin", "Remove Curve Pin"),
				LOCTEXT("RemoveCurvePinTooltip", "Remove a pin driving a curve"),
				FNewMenuDelegate::CreateUObject(this, &UAnimGraphNode_CurveViewer::GetRemoveCurveMenuActions));
		}
	}
}

void UAnimGraphNode_CurveViewer::RemoveCurvePin(FName CurveName)
{
	// Make sure we have a curve pin with that name
	const int32 CurveIndex = Node.CurveNames.Find(CurveName);
	if (CurveIndex != INDEX_NONE)
	{
		FScopedTransaction Transaction( LOCTEXT("RemoveCurvePinTrans", "Remove Curve Pin") );
		Modify();

		Node.RemoveCurve(CurveIndex);
	
		ReconstructNode();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}
}

void UAnimGraphNode_CurveViewer::AddCurvePin(FName CurveName)
{
	// Make sure it doesn't already exist
	const int32 CurveIndex = Node.CurveNames.Find(CurveName);
	if (CurveIndex == INDEX_NONE)
	{
		FScopedTransaction Transaction(LOCTEXT("AddCurvePinTrans", "Add Curve Pin"));
		Modify();

		Node.AddCurve(CurveName, 0.f);

		ReconstructNode();
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(GetBlueprint());
	}
}


void UAnimGraphNode_CurveViewer::CustomizePinData(UEdGraphPin* Pin, FName SourcePropertyName, int32 ArrayIndex) const
{
	if (SourcePropertyName == GET_MEMBER_NAME_CHECKED(FAnimNode_CurveViewer, CurveValues))
	{
		if (Node.CurveNames.IsValidIndex(ArrayIndex))
		{
			Pin->PinFriendlyName = FText::FromName(Node.CurveNames[ArrayIndex]);
		}
	}
}

#undef LOCTEXT_NAMESPACE
