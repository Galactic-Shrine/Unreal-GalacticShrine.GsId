#include "Localization/GsIdEditorLocalizedMetadata.h"

#include "EditorCategoryUtils.h"
#include "Internationalization/Text.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"
#include "GsId.h"

#define LOCTEXT_NAMESPACE "GsIdEditorLocalization"

namespace GsIdEditorLocalization
{
    namespace
    {
        FDelegateHandle ReloadCompleteHandle;

        void SetUFieldDisplayName(UField* Field, const FText& Text)
        {
            if (Field)
            {
                Field->SetMetaData(TEXT("DisplayName"), *Text.ToString());
            }
        }

        void SetPropertyDisplayName(FProperty* Property, const FText& Text)
        {
            if (Property)
            {
                Property->SetMetaData(TEXT("DisplayName"), *Text.ToString());
            }
        }

        void SetFunctionTextMetadata(UFunction* Function, const TCHAR* Key, const FText& Text)
        {
            if (Function)
            {
                Function->SetMetaData(Key, *Text.ToString());
            }
        }

        void HandleReloadComplete(const EReloadCompleteReason)
        {
            ApplyLocalizedMetadata();
        }
    }

    void ApplyLocalizedMetadata()
    {
        // Shared prefixed brand label resolved through a stable technical category key.
        FEditorCategoryUtils::RegisterCategoryKey(
            TEXT("GalacticShrine"),
            FText::FromString(TEXT("# GalacticShrine")),
            FText::GetEmpty()
        );
        FEditorCategoryUtils::RegisterCategoryKey(
            TEXT("GsId_Galactic_Shrine_GsId"),
            LOCTEXT("Category_GsId_Galactic_Shrine_GsId", "GSID"),
            FText::GetEmpty()
        );

        if (auto* Type_EGsIdFormat = StaticEnum<EGsIdFormat>())
        {
            SetUFieldDisplayName(Type_EGsIdFormat, LOCTEXT("Type_EGsIdFormat_DisplayName", "GsId Format"));
            if (const int32 Index = Type_EGsIdFormat->GetIndexByNameString(TEXT("N")); Index != INDEX_NONE)
            {
                Type_EGsIdFormat->SetMetaData(TEXT("DisplayName"), *LOCTEXT("Enum_EGsIdFormat_N_DisplayName", "N — 64 hexadecimal characters").ToString(), Index);
            }
            if (const int32 Index = Type_EGsIdFormat->GetIndexByNameString(TEXT("D")); Index != INDEX_NONE)
            {
                Type_EGsIdFormat->SetMetaData(TEXT("DisplayName"), *LOCTEXT("Enum_EGsIdFormat_D_DisplayName", "D — 16-8-8-8-8-16").ToString(), Index);
            }
        }

        if (auto* Type_EGsIdLetterCase = StaticEnum<EGsIdLetterCase>())
        {
            SetUFieldDisplayName(Type_EGsIdLetterCase, LOCTEXT("Type_EGsIdLetterCase_DisplayName", "GsId Letter Case"));
            if (const int32 Index = Type_EGsIdLetterCase->GetIndexByNameString(TEXT("Upper")); Index != INDEX_NONE)
            {
                Type_EGsIdLetterCase->SetMetaData(TEXT("DisplayName"), *LOCTEXT("Enum_EGsIdLetterCase_Upper_DisplayName", "Uppercase").ToString(), Index);
            }
            if (const int32 Index = Type_EGsIdLetterCase->GetIndexByNameString(TEXT("Lower")); Index != INDEX_NONE)
            {
                Type_EGsIdLetterCase->SetMetaData(TEXT("DisplayName"), *LOCTEXT("Enum_EGsIdLetterCase_Lower_DisplayName", "Lowercase").ToString(), Index);
            }
        }

        if (auto* Type_FGsId = FGsId::StaticStruct())
        {
            SetUFieldDisplayName(Type_FGsId, LOCTEXT("Type_FGsId_DisplayName", "GsId"));
        }

        if (auto* Type_UGsIdBlueprintLibrary = UGsIdBlueprintLibrary::StaticClass())
        {
            SetUFieldDisplayName(Type_UGsIdBlueprintLibrary, LOCTEXT("Type_UGsIdBlueprintLibrary_DisplayName", "GsId Blueprint Library"));
            if (UFunction* Function_NewGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("NewGsId")))
            {
                SetUFieldDisplayName(Function_NewGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_NewGsId_DisplayName", "New GsId"));
                SetFunctionTextMetadata(Function_NewGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_NewGsId_Keywords", "new gsid gsid"));
                SetFunctionTextMetadata(Function_NewGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_NewGsId_ReturnDisplayName", "GsId"));
            }
            if (UFunction* Function_TryNewGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("TryNewGsId")))
            {
                SetUFieldDisplayName(Function_TryNewGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_TryNewGsId_DisplayName", "Try New GsId"));
                SetFunctionTextMetadata(Function_TryNewGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_TryNewGsId_Keywords", "try new gsid gsid"));
                SetFunctionTextMetadata(Function_TryNewGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_TryNewGsId_ReturnDisplayName", "Success"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_TryNewGsId, TEXT("OutGsId")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_TryNewGsId_OutGsId_DisplayName", "Out Gs Id"));
            }
            if (UFunction* Function_EmptyGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("EmptyGsId")))
            {
                SetUFieldDisplayName(Function_EmptyGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_EmptyGsId_DisplayName", "Empty GsId"));
                SetFunctionTextMetadata(Function_EmptyGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_EmptyGsId_Keywords", "empty gsid gsid"));
                SetFunctionTextMetadata(Function_EmptyGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_EmptyGsId_ReturnDisplayName", "GsId"));
            }
            if (UFunction* Function_IsEmpty = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("IsEmpty")))
            {
                SetUFieldDisplayName(Function_IsEmpty, LOCTEXT("Function_UGsIdBlueprintLibrary_IsEmpty_DisplayName", "Is Empty"));
                SetFunctionTextMetadata(Function_IsEmpty, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_IsEmpty_Keywords", "is empty gsid"));
                SetFunctionTextMetadata(Function_IsEmpty, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_IsEmpty_ReturnDisplayName", "Result"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_IsEmpty, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_IsEmpty_Value_DisplayName", "Value"));
            }
            if (UFunction* Function_IsValid = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("IsValid")))
            {
                SetUFieldDisplayName(Function_IsValid, LOCTEXT("Function_UGsIdBlueprintLibrary_IsValid_DisplayName", "Is Valid"));
                SetFunctionTextMetadata(Function_IsValid, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_IsValid_Keywords", "is valid gsid"));
                SetFunctionTextMetadata(Function_IsValid, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_IsValid_ReturnDisplayName", "Result"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_IsValid, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_IsValid_Value_DisplayName", "Value"));
            }
            if (UFunction* Function_EqualEqual_GsIdGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("EqualEqual_GsIdGsId")))
            {
                SetUFieldDisplayName(Function_EqualEqual_GsIdGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_EqualEqual_GsIdGsId_DisplayName", "Equal (GsId)"));
                SetFunctionTextMetadata(Function_EqualEqual_GsIdGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_EqualEqual_GsIdGsId_Keywords", "equal gsid gsid"));
                SetFunctionTextMetadata(Function_EqualEqual_GsIdGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_EqualEqual_GsIdGsId_ReturnDisplayName", "Result"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_EqualEqual_GsIdGsId, TEXT("A")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_EqualEqual_GsIdGsId_A_DisplayName", "A"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_EqualEqual_GsIdGsId, TEXT("B")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_EqualEqual_GsIdGsId_B_DisplayName", "B"));
            }
            if (UFunction* Function_NotEqual_GsIdGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("NotEqual_GsIdGsId")))
            {
                SetUFieldDisplayName(Function_NotEqual_GsIdGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_NotEqual_GsIdGsId_DisplayName", "Not Equal (GsId)"));
                SetFunctionTextMetadata(Function_NotEqual_GsIdGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_NotEqual_GsIdGsId_Keywords", "not equal gsid gsid"));
                SetFunctionTextMetadata(Function_NotEqual_GsIdGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_NotEqual_GsIdGsId_ReturnDisplayName", "Result"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_NotEqual_GsIdGsId, TEXT("A")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_NotEqual_GsIdGsId_A_DisplayName", "A"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_NotEqual_GsIdGsId, TEXT("B")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_NotEqual_GsIdGsId_B_DisplayName", "B"));
            }
            if (UFunction* Function_Less_GsIdGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("Less_GsIdGsId")))
            {
                SetUFieldDisplayName(Function_Less_GsIdGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_Less_GsIdGsId_DisplayName", "Less (GsId)"));
                SetFunctionTextMetadata(Function_Less_GsIdGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_Less_GsIdGsId_Keywords", "less gsid gsid"));
                SetFunctionTextMetadata(Function_Less_GsIdGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_Less_GsIdGsId_ReturnDisplayName", "Result"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_Less_GsIdGsId, TEXT("A")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_Less_GsIdGsId_A_DisplayName", "A"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_Less_GsIdGsId, TEXT("B")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_Less_GsIdGsId_B_DisplayName", "B"));
            }
            if (UFunction* Function_ToString = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("ToString")))
            {
                SetUFieldDisplayName(Function_ToString, LOCTEXT("Function_UGsIdBlueprintLibrary_ToString_DisplayName", "To String"));
                SetFunctionTextMetadata(Function_ToString, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_ToString_Keywords", "to string gsid"));
                SetFunctionTextMetadata(Function_ToString, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_ToString_ReturnDisplayName", "Text"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_ToString, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_ToString_Value_DisplayName", "Value"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_ToString, TEXT("Format")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_ToString_Format_DisplayName", "Format"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_ToString, TEXT("LetterCase")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_ToString_LetterCase_DisplayName", "Letter Case"));
            }
            if (UFunction* Function_Parse = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("Parse")))
            {
                SetUFieldDisplayName(Function_Parse, LOCTEXT("Function_UGsIdBlueprintLibrary_Parse_DisplayName", "Parse"));
                SetFunctionTextMetadata(Function_Parse, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_Parse_Keywords", "parse gsid"));
                SetFunctionTextMetadata(Function_Parse, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_Parse_ReturnDisplayName", "Success"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_Parse, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_Parse_Value_DisplayName", "Value"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_Parse, TEXT("OutGsId")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_Parse_OutGsId_DisplayName", "GsId"));
            }
            if (UFunction* Function_ToBytes = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("ToBytes")))
            {
                SetUFieldDisplayName(Function_ToBytes, LOCTEXT("Function_UGsIdBlueprintLibrary_ToBytes_DisplayName", "To Bytes"));
                SetFunctionTextMetadata(Function_ToBytes, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_ToBytes_Keywords", "to bytes gsid"));
                SetFunctionTextMetadata(Function_ToBytes, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_ToBytes_ReturnDisplayName", "Bytes"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_ToBytes, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_ToBytes_Value_DisplayName", "Value"));
            }
            if (UFunction* Function_FromBytes = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("FromBytes")))
            {
                SetUFieldDisplayName(Function_FromBytes, LOCTEXT("Function_UGsIdBlueprintLibrary_FromBytes_DisplayName", "From Bytes"));
                SetFunctionTextMetadata(Function_FromBytes, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_FromBytes_Keywords", "from bytes gsid"));
                SetFunctionTextMetadata(Function_FromBytes, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_FromBytes_ReturnDisplayName", "Success"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_FromBytes, TEXT("Bytes")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_FromBytes_Bytes_DisplayName", "Bytes"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_FromBytes, TEXT("OutGsId")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_FromBytes_OutGsId_DisplayName", "GsId"));
            }
            if (UFunction* Function_CreateDeterministicGsId = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("CreateDeterministicGsId")))
            {
                SetUFieldDisplayName(Function_CreateDeterministicGsId, LOCTEXT("Function_UGsIdBlueprintLibrary_CreateDeterministicGsId_DisplayName", "Create Deterministic GsId"));
                SetFunctionTextMetadata(Function_CreateDeterministicGsId, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_CreateDeterministicGsId_Keywords", "create deterministic gsid gsid"));
                SetFunctionTextMetadata(Function_CreateDeterministicGsId, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_CreateDeterministicGsId_ReturnDisplayName", "GsId"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_CreateDeterministicGsId, TEXT("InputBytes")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_CreateDeterministicGsId_InputBytes_DisplayName", "Input Bytes"));
            }
            if (UFunction* Function_GetStableHash = Type_UGsIdBlueprintLibrary->FindFunctionByName(TEXT("GetStableHash")))
            {
                SetUFieldDisplayName(Function_GetStableHash, LOCTEXT("Function_UGsIdBlueprintLibrary_GetStableHash_DisplayName", "Get Stable Hash"));
                SetFunctionTextMetadata(Function_GetStableHash, TEXT("Keywords"), LOCTEXT("Function_UGsIdBlueprintLibrary_GetStableHash_Keywords", "get stable hash gsid"));
                SetFunctionTextMetadata(Function_GetStableHash, TEXT("ReturnDisplayName"), LOCTEXT("Function_UGsIdBlueprintLibrary_GetStableHash_ReturnDisplayName", "Stable Hash"));
                SetPropertyDisplayName(FindFProperty<FProperty>(Function_GetStableHash, TEXT("Value")), LOCTEXT("Parameter_UGsIdBlueprintLibrary_GetStableHash_Value_DisplayName", "Value"));
            }
        }
    }

    void StartupLocalizedMetadata()
    {
        ApplyLocalizedMetadata();
        ReloadCompleteHandle = FCoreUObjectDelegates::ReloadCompleteDelegate.AddStatic(&HandleReloadComplete);
    }

    void ShutdownLocalizedMetadata()
    {
        if (ReloadCompleteHandle.IsValid())
        {
            FCoreUObjectDelegates::ReloadCompleteDelegate.Remove(ReloadCompleteHandle);
            ReloadCompleteHandle.Reset();
        }
    }
}

#undef LOCTEXT_NAMESPACE
