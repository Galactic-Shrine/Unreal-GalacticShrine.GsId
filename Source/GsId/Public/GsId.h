#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Serialization/Archive.h"

#include "GsId.generated.h"

namespace GalacticShrine
{
    class GsId;
}

UENUM(BlueprintType)
enum class EGsIdFormat : uint8
{
    N UMETA(DisplayName = "N — 64 hexadecimal characters"),
    D UMETA(DisplayName = "D — 16-8-8-8-8-16")
};

UENUM(BlueprintType)
enum class EGsIdLetterCase : uint8
{
    Upper UMETA(DisplayName = "Uppercase"),
    Lower UMETA(DisplayName = "Lowercase")
};

/**
 * Unreal representation of a Galactic-Shrine 256-bit identifier.
 * The structure contains exactly the 32 native GsId bytes, without reordering.
 */
USTRUCT(BlueprintType)
struct GSID_API FGsId
{
    GENERATED_BODY()

public:
    static constexpr int32 ByteLength = 32;
    static constexpr int32 NormalizedStringLength = 64;
    static constexpr int32 FormattedStringLength = 69;

    FGsId();

#if CPP
    explicit FGsId(const GalacticShrine::GsId& NativeValue);
    GalacticShrine::GsId ToNative() const;
#endif

    static FGsId Empty();
    static FGsId NewGsId();
    static bool TryNewGsId(FGsId& OutGsId);
    static FGsId FromBytes(const uint8* InBytes, int32 InByteCount);
    static FGsId CreateDeterministic(TArrayView<const uint8> InputBytes);

    static bool Parse(const FString& Value, FGsId& OutGsId);
    static bool ParseExact(
        const FString& Value,
        EGsIdFormat Format,
        FGsId& OutGsId
    );

    FString ToString(
        EGsIdFormat Format = EGsIdFormat::D,
        EGsIdLetterCase LetterCase = EGsIdLetterCase::Upper
    ) const;

    bool IsEmpty() const;
    bool IsValid() const { return !IsEmpty(); }

    const uint8* GetBytes() const { return Bytes; }
    void CopyBytes(uint8* OutBytes, int32 OutByteCount) const;
    TArray<uint8> ToByteArray() const;

    bool Serialize(FArchive& Archive);
    bool ExportTextItem(
        FString& ValueStr,
        const FGsId& DefaultValue,
        UObject* Parent,
        int32 PortFlags,
        UObject* ExportRootScope
    ) const;
    bool ImportTextItem(
        const TCHAR*& Buffer,
        int32 PortFlags,
        UObject* Parent,
        FOutputDevice* ErrorText
    );
    bool NetSerialize(
        FArchive& Archive,
        class UPackageMap* Map,
        bool& bOutSuccess
    );

    bool operator==(const FGsId& Other) const;
    bool operator!=(const FGsId& Other) const { return !(*this == Other); }
    bool operator<(const FGsId& Other) const;
    bool operator>(const FGsId& Other) const { return Other < *this; }
    bool operator<=(const FGsId& Other) const { return !(Other < *this); }
    bool operator>=(const FGsId& Other) const { return !(*this < Other); }

    friend uint32 GetTypeHash(const FGsId& Value)
    {
        uint32 Hash = 2166136261u;
        for (const uint8 Byte : Value.Bytes)
        {
            Hash ^= Byte;
            Hash *= 16777619u;
        }
        return Hash;
    }
    friend FArchive& operator<<(FArchive& Archive, FGsId& Value)
    {
        Value.Serialize(Archive);
        return Archive;
    }

private:
    uint8 Bytes[32];
};

static_assert(sizeof(FGsId) == 32, "FGsId must contain exactly 32 bytes.");

GSID_API FString LexToString(const FGsId& Value);
GSID_API void LexFromString(FGsId& OutValue, const TCHAR* Buffer);

UCLASS()
class GSID_API UGsIdBlueprintLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static FGsId NewGsId();

    UFUNCTION(BlueprintCallable, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool TryNewGsId(FGsId& OutGsId);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static FGsId EmptyGsId();

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool IsEmpty(const FGsId& Value);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool IsValid(const FGsId& Value);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Equal (GsId)", CompactNodeTitle = "==", ScriptMethod = "Equals", ScriptOperator = "=="), Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool EqualEqual_GsIdGsId(const FGsId& A, const FGsId& B);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Not Equal (GsId)", CompactNodeTitle = "!=", ScriptMethod = "NotEqual", ScriptOperator = "!="), Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool NotEqual_GsIdGsId(const FGsId& A, const FGsId& B);

    UFUNCTION(BlueprintPure, meta = (DisplayName = "Less (GsId)", CompactNodeTitle = "<", ScriptOperator = "<"), Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool Less_GsIdGsId(const FGsId& A, const FGsId& B);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static FString ToString(
        const FGsId& Value,
        EGsIdFormat Format = EGsIdFormat::D,
        EGsIdLetterCase LetterCase = EGsIdLetterCase::Upper
    );

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool Parse(const FString& Value, FGsId& OutGsId);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static TArray<uint8> ToBytes(const FGsId& Value);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static bool FromBytes(const TArray<uint8>& Bytes, FGsId& OutGsId);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static FGsId CreateDeterministicGsId(const TArray<uint8>& InputBytes);

    UFUNCTION(BlueprintPure, Category = "{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    static int64 GetStableHash(const FGsId& Value);
};

template<>
struct TStructOpsTypeTraits<FGsId> : TStructOpsTypeTraitsBase2<FGsId>
{
    enum
    {
        WithSerializer = true,
        WithExportTextItem = true,
        WithImportTextItem = true,
        WithNetSerializer = true,
        WithIdenticalViaEquality = true,
        WithZeroConstructor = true,
        WithNoDestructor = true
    };
};
