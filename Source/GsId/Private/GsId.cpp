#include "GsId.h"

#include <galactic_shrine/gsid/gsid.hpp>

#include "Containers/StringConv.h"
#include "HAL/UnrealMemory.h"

static_assert(
    sizeof(GalacticShrine::GsId) == FGsId::ByteLength,
    "The native GalacticShrine::GsId must contain exactly 32 bytes."
);

namespace GsIdUnreal
{
    static GalacticShrine::GsIdFormat ToNativeFormat(const EGsIdFormat Format)
    {
        return Format == EGsIdFormat::N
            ? GalacticShrine::GsIdFormat::N
            : GalacticShrine::GsIdFormat::D;
    }

    static GalacticShrine::GsIdCase ToNativeCase(const EGsIdLetterCase LetterCase)
    {
        return LetterCase == EGsIdLetterCase::Lower
            ? GalacticShrine::GsIdCase::Lower
            : GalacticShrine::GsIdCase::Upper;
    }

    static void WriteUint32BigEndian(uint8* Destination, const uint32 Value)
    {
        Destination[0] = static_cast<uint8>((Value >> 24) & 0xFFu);
        Destination[1] = static_cast<uint8>((Value >> 16) & 0xFFu);
        Destination[2] = static_cast<uint8>((Value >> 8) & 0xFFu);
        Destination[3] = static_cast<uint8>(Value & 0xFFu);
    }

    static uint32 RotateRight(const uint32 Value, const uint32 Count)
    {
        return (Value >> Count) | (Value << (32u - Count));
    }

    /** Small, self-contained SHA-256 used only for deterministic GsId creation. */
    static void ComputeSha256(TArrayView<const uint8> Input, uint8 OutDigest[32])
    {
        static constexpr uint32 K[64] =
        {
            0x428a2f98u, 0x71374491u, 0xb5c0fbcfu, 0xe9b5dba5u,
            0x3956c25bu, 0x59f111f1u, 0x923f82a4u, 0xab1c5ed5u,
            0xd807aa98u, 0x12835b01u, 0x243185beu, 0x550c7dc3u,
            0x72be5d74u, 0x80deb1feu, 0x9bdc06a7u, 0xc19bf174u,
            0xe49b69c1u, 0xefbe4786u, 0x0fc19dc6u, 0x240ca1ccu,
            0x2de92c6fu, 0x4a7484aau, 0x5cb0a9dcu, 0x76f988dau,
            0x983e5152u, 0xa831c66du, 0xb00327c8u, 0xbf597fc7u,
            0xc6e00bf3u, 0xd5a79147u, 0x06ca6351u, 0x14292967u,
            0x27b70a85u, 0x2e1b2138u, 0x4d2c6dfcu, 0x53380d13u,
            0x650a7354u, 0x766a0abbu, 0x81c2c92eu, 0x92722c85u,
            0xa2bfe8a1u, 0xa81a664bu, 0xc24b8b70u, 0xc76c51a3u,
            0xd192e819u, 0xd6990624u, 0xf40e3585u, 0x106aa070u,
            0x19a4c116u, 0x1e376c08u, 0x2748774cu, 0x34b0bcb5u,
            0x391c0cb3u, 0x4ed8aa4au, 0x5b9cca4fu, 0x682e6ff3u,
            0x748f82eeu, 0x78a5636fu, 0x84c87814u, 0x8cc70208u,
            0x90befffau, 0xa4506cebu, 0xbef9a3f7u, 0xc67178f2u
        };

        uint32 State[8] =
        {
            0x6a09e667u,
            0xbb67ae85u,
            0x3c6ef372u,
            0xa54ff53au,
            0x510e527fu,
            0x9b05688cu,
            0x1f83d9abu,
            0x5be0cd19u
        };

        const uint64 BitLength = static_cast<uint64>(Input.Num()) * 8ull;
        const int64 RequiredPaddedLength =
            ((static_cast<int64>(Input.Num()) + 9 + 63) / 64) * 64;
        check(RequiredPaddedLength <= MAX_int32);
        const int32 PaddedLength = static_cast<int32>(RequiredPaddedLength);
        TArray<uint8> Padded;
        Padded.SetNumZeroed(PaddedLength);
        if (Input.Num() > 0)
        {
            FMemory::Memcpy(Padded.GetData(), Input.GetData(), Input.Num());
        }
        Padded[Input.Num()] = 0x80u;
        for (int32 Index = 0; Index < 8; ++Index)
        {
            Padded[PaddedLength - 1 - Index] = static_cast<uint8>((BitLength >> (Index * 8)) & 0xFFull);
        }

        for (int32 Offset = 0; Offset < PaddedLength; Offset += 64)
        {
            uint32 W[64]{};
            for (int32 Index = 0; Index < 16; ++Index)
            {
                const uint8* Source = Padded.GetData() + Offset + Index * 4;
                W[Index] = (static_cast<uint32>(Source[0]) << 24)
                    | (static_cast<uint32>(Source[1]) << 16)
                    | (static_cast<uint32>(Source[2]) << 8)
                    | static_cast<uint32>(Source[3]);
            }
            for (int32 Index = 16; Index < 64; ++Index)
            {
                const uint32 S0 = RotateRight(W[Index - 15], 7)
                    ^ RotateRight(W[Index - 15], 18)
                    ^ (W[Index - 15] >> 3);
                const uint32 S1 = RotateRight(W[Index - 2], 17)
                    ^ RotateRight(W[Index - 2], 19)
                    ^ (W[Index - 2] >> 10);
                W[Index] = W[Index - 16] + S0 + W[Index - 7] + S1;
            }

            uint32 A = State[0];
            uint32 B = State[1];
            uint32 C = State[2];
            uint32 D = State[3];
            uint32 E = State[4];
            uint32 F = State[5];
            uint32 G = State[6];
            uint32 H = State[7];

            for (int32 Index = 0; Index < 64; ++Index)
            {
                const uint32 Sigma1 = RotateRight(E, 6) ^ RotateRight(E, 11) ^ RotateRight(E, 25);
                const uint32 Choose = (E & F) ^ ((~E) & G);
                const uint32 Temp1 = H + Sigma1 + Choose + K[Index] + W[Index];
                const uint32 Sigma0 = RotateRight(A, 2) ^ RotateRight(A, 13) ^ RotateRight(A, 22);
                const uint32 Majority = (A & B) ^ (A & C) ^ (B & C);
                const uint32 Temp2 = Sigma0 + Majority;

                H = G;
                G = F;
                F = E;
                E = D + Temp1;
                D = C;
                C = B;
                B = A;
                A = Temp1 + Temp2;
            }

            State[0] += A;
            State[1] += B;
            State[2] += C;
            State[3] += D;
            State[4] += E;
            State[5] += F;
            State[6] += G;
            State[7] += H;
        }

        for (int32 Index = 0; Index < 8; ++Index)
        {
            WriteUint32BigEndian(OutDigest + Index * 4, State[Index]);
        }
    }
}

FGsId::FGsId()
{
    FMemory::Memzero(Bytes, ByteLength);
}

FGsId::FGsId(const GalacticShrine::GsId& NativeValue)
{
    const GalacticShrine::GsId::ByteArray& NativeBytes = NativeValue.Bytes();
    FMemory::Memcpy(Bytes, NativeBytes.data(), ByteLength);
}

GalacticShrine::GsId FGsId::ToNative() const
{
    return GalacticShrine::GsId::FromBytes(
        std::span<const std::uint8_t>(Bytes, ByteLength)
    );
}

FGsId FGsId::Empty()
{
    return FGsId();
}

FGsId FGsId::NewGsId()
{
    FGsId Result;
    TryNewGsId(Result);
    return Result;
}

bool FGsId::TryNewGsId(FGsId& OutGsId)
{
    try
    {
        GalacticShrine::GsId NativeValue;
        if (!GalacticShrine::GsId::TryNewGsId(NativeValue))
        {
            OutGsId = Empty();
            return false;
        }
        OutGsId = FGsId(NativeValue);
        return true;
    }
    catch (const std::exception& Exception)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("GalacticShrine::GsId generation failed: %s"),
            UTF8_TO_TCHAR(Exception.what())
        );
        OutGsId = Empty();
        return false;
    }
    catch (...)
    {
        UE_LOG(
            LogTemp,
            Error,
            TEXT("GalacticShrine::GsId generation failed with an unknown error.")
        );
        OutGsId = Empty();
        return false;
    }
}

FGsId FGsId::FromBytes(const uint8* InBytes, const int32 InByteCount)
{
    FGsId Result;
    if (InBytes == nullptr || InByteCount != ByteLength)
    {
        return Result;
    }

    const GalacticShrine::GsId Native = GalacticShrine::GsId::FromBytes(
        std::span<const std::uint8_t>(InBytes, ByteLength)
    );
    return FGsId(Native);
}

FGsId FGsId::CreateDeterministic(const TArrayView<const uint8> InputBytes)
{
    uint8 Digest[ByteLength]{};
    GsIdUnreal::ComputeSha256(InputBytes, Digest);
    return FGsId(GalacticShrine::GsId::FromBytes(
        std::span<const std::uint8_t>(Digest, ByteLength)
    ));
}

bool FGsId::Parse(const FString& Value, FGsId& OutGsId)
{
    const FTCHARToUTF8 Utf8(*Value);
    GalacticShrine::GsId Native;
    if (!GalacticShrine::GsId::TryParse(
        std::string_view(Utf8.Get(), Utf8.Length()),
        Native
    ))
    {
        OutGsId = Empty();
        return false;
    }

    OutGsId = FGsId(Native);
    return true;
}

bool FGsId::ParseExact(
    const FString& Value,
    const EGsIdFormat Format,
    FGsId& OutGsId
)
{
    const FTCHARToUTF8 Utf8(*Value);
    const std::string_view Text(Utf8.Get(), Utf8.Length());
    if (!GalacticShrine::GsIdValidator::IsValid(
        Text,
        GsIdUnreal::ToNativeFormat(Format)
    ))
    {
        OutGsId = Empty();
        return false;
    }

    return Parse(Value, OutGsId);
}

FString FGsId::ToString(
    const EGsIdFormat Format,
    const EGsIdLetterCase LetterCase
) const
{
    const std::string NativeText = ToNative().ToString(
        GsIdUnreal::ToNativeFormat(Format),
        GsIdUnreal::ToNativeCase(LetterCase)
    );
    return UTF8_TO_TCHAR(NativeText.c_str());
}

bool FGsId::IsEmpty() const
{
    return ToNative().IsEmpty();
}

void FGsId::CopyBytes(uint8* OutBytes, const int32 OutByteCount) const
{
    if (OutBytes != nullptr && OutByteCount >= ByteLength)
    {
        FMemory::Memcpy(OutBytes, Bytes, ByteLength);
    }
}

TArray<uint8> FGsId::ToByteArray() const
{
    TArray<uint8> Result;
    Result.Append(Bytes, ByteLength);
    return Result;
}

bool FGsId::Serialize(FArchive& Archive)
{
    Archive.Serialize(Bytes, ByteLength);
    return !Archive.IsError();
}

bool FGsId::ExportTextItem(
    FString& ValueStr,
    const FGsId& DefaultValue,
    UObject* Parent,
    const int32 PortFlags,
    UObject* ExportRootScope
) const
{
    static_cast<void>(DefaultValue);
    static_cast<void>(Parent);
    static_cast<void>(PortFlags);
    static_cast<void>(ExportRootScope);
    ValueStr += ToString(EGsIdFormat::D, EGsIdLetterCase::Upper);
    return true;
}

bool FGsId::ImportTextItem(
    const TCHAR*& Buffer,
    const int32 PortFlags,
    UObject* Parent,
    FOutputDevice* ErrorText
)
{
    static_cast<void>(PortFlags);
    static_cast<void>(Parent);
    if (Buffer == nullptr)
    {
        return false;
    }

    const TCHAR* Cursor = Buffer;
    while (*Cursor != TEXT('\0') && FChar::IsWhitespace(*Cursor))
    {
        ++Cursor;
    }

    FString Token;
    if (*Cursor == TEXT('"'))
    {
        ++Cursor;
        while (*Cursor != TEXT('\0') && *Cursor != TEXT('"'))
        {
            Token.AppendChar(*Cursor++);
        }
        if (*Cursor != TEXT('"'))
        {
            return false;
        }
        ++Cursor;
    }
    else
    {
        while (*Cursor != TEXT('\0') &&
            !FChar::IsWhitespace(*Cursor) &&
            *Cursor != TEXT(',') &&
            *Cursor != TEXT(')') &&
            *Cursor != TEXT(']') &&
            *Cursor != TEXT('}'))
        {
            Token.AppendChar(*Cursor++);
        }
    }

    FGsId Parsed;
    if (Token.IsEmpty() || !Parse(Token, Parsed))
    {
        if (ErrorText != nullptr)
        {
            ErrorText->Logf(TEXT("Invalid GsId text value: %s"), *Token);
        }
        return false;
    }

    *this = Parsed;
    Buffer = Cursor;
    return true;
}

bool FGsId::NetSerialize(
    FArchive& Archive,
    UPackageMap* Map,
    bool& bOutSuccess
)
{
    static_cast<void>(Map);
    Archive.Serialize(Bytes, ByteLength);
    bOutSuccess = !Archive.IsError();
    return true;
}

bool FGsId::operator==(const FGsId& Other) const
{
    return FMemory::Memcmp(Bytes, Other.Bytes, ByteLength) == 0;
}

bool FGsId::operator<(const FGsId& Other) const
{
    return FMemory::Memcmp(Bytes, Other.Bytes, ByteLength) < 0;
}

FString LexToString(const FGsId& Value)
{
    return Value.ToString(EGsIdFormat::D, EGsIdLetterCase::Upper);
}

void LexFromString(FGsId& OutValue, const TCHAR* Buffer)
{
    if (Buffer == nullptr || !FGsId::Parse(FString(Buffer), OutValue))
    {
        OutValue = FGsId::Empty();
    }
}

FGsId UGsIdBlueprintLibrary::NewGsId()
{
    return FGsId::NewGsId();
}

bool UGsIdBlueprintLibrary::TryNewGsId(FGsId& OutGsId)
{
    return FGsId::TryNewGsId(OutGsId);
}

FGsId UGsIdBlueprintLibrary::EmptyGsId()
{
    return FGsId::Empty();
}

bool UGsIdBlueprintLibrary::IsEmpty(const FGsId& Value)
{
    return Value.IsEmpty();
}

bool UGsIdBlueprintLibrary::IsValid(const FGsId& Value)
{
    return Value.IsValid();
}

bool UGsIdBlueprintLibrary::EqualEqual_GsIdGsId(const FGsId& A, const FGsId& B)
{
    return A == B;
}

bool UGsIdBlueprintLibrary::NotEqual_GsIdGsId(const FGsId& A, const FGsId& B)
{
    return A != B;
}

bool UGsIdBlueprintLibrary::Less_GsIdGsId(const FGsId& A, const FGsId& B)
{
    return A < B;
}

FString UGsIdBlueprintLibrary::ToString(
    const FGsId& Value,
    const EGsIdFormat Format,
    const EGsIdLetterCase LetterCase
)
{
    return Value.ToString(Format, LetterCase);
}

bool UGsIdBlueprintLibrary::Parse(const FString& Value, FGsId& OutGsId)
{
    return FGsId::Parse(Value, OutGsId);
}

TArray<uint8> UGsIdBlueprintLibrary::ToBytes(const FGsId& Value)
{
    return Value.ToByteArray();
}

bool UGsIdBlueprintLibrary::FromBytes(const TArray<uint8>& Bytes, FGsId& OutGsId)
{
    if (Bytes.Num() != FGsId::ByteLength)
    {
        OutGsId = FGsId::Empty();
        return false;
    }
    OutGsId = FGsId::FromBytes(Bytes.GetData(), Bytes.Num());
    return true;
}

FGsId UGsIdBlueprintLibrary::CreateDeterministicGsId(const TArray<uint8>& InputBytes)
{
    return FGsId::CreateDeterministic(InputBytes);
}

int64 UGsIdBlueprintLibrary::GetStableHash(const FGsId& Value)
{
    return static_cast<int64>(GetTypeHash(Value));
}
