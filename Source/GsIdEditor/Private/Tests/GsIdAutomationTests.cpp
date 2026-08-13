#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "EditorCategoryUtils.h"

#include <galactic_shrine/gsid/gsid.hpp>

#include "GsId.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"

namespace GsIdAutomationTests
{
    static constexpr TCHAR KnownD[] =
        TEXT("4EE77CA611AB7EBC-9FABDC58-C9689676-F0763AF9-EFF3C0DF-575F94AB230C0C66");
    static constexpr TCHAR KnownN[] =
        TEXT("4EE77CA611AB7EBC9FABDC58C9689676F0763AF9EFF3C0DF575F94AB230C0C66");
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdNativeGenerationTest,
    "GsId.Native.Generation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdNativeGenerationTest::RunTest(const FString& Parameters)
{
    const GalacticShrine::GsId NativeFirst = GalacticShrine::GsId::NewGsId();
    const GalacticShrine::GsId NativeSecond = GalacticShrine::GsId::NewGsId();
    const FGsId First(NativeFirst);
    const FGsId Second(NativeSecond);

    TestEqual(TEXT("FGsId must remain exactly 32 bytes."), static_cast<int32>(sizeof(FGsId)), 32);
    TestTrue(TEXT("A generated GsId must be valid."), First.IsValid());
    TestTrue(TEXT("Two generated GsIds should differ."), First != Second);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdNativeTryGenerationTest,
    "GsId.Native.TryGeneration",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdNativeTryGenerationTest::RunTest(const FString& Parameters)
{
    FGsId Value;
    TestTrue(TEXT("TryNewGsId must report successful secure generation."), FGsId::TryNewGsId(Value));
    TestTrue(TEXT("TryNewGsId must return a valid value on success."), Value.IsValid());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdParsingTest,
    "GsId.Text.Parsing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdParsingTest::RunTest(const FString& Parameters)
{
    using namespace GsIdAutomationTests;
    FGsId FromD;
    FGsId FromN;
    TestTrue(TEXT("The D form must parse."), FGsId::Parse(KnownD, FromD));
    TestTrue(TEXT("The N form must parse."), FGsId::Parse(KnownN, FromN));
    TestTrue(TEXT("N and D must preserve the same bytes."), FromD == FromN);

    FGsId Invalid;
    TestFalse(TEXT("An invalid value must be rejected."), FGsId::Parse(TEXT("not-a-gsid"), Invalid));
    TestTrue(TEXT("A failed parse must return Empty."), Invalid.IsEmpty());
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdExactParsingTest,
    "GsId.Text.ExactParsing",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdExactParsingTest::RunTest(const FString& Parameters)
{
    using namespace GsIdAutomationTests;
    FGsId Value;
    TestTrue(TEXT("N must parse as N."), FGsId::ParseExact(KnownN, EGsIdFormat::N, Value));
    TestFalse(TEXT("N must not parse as D."), FGsId::ParseExact(KnownN, EGsIdFormat::D, Value));
    TestTrue(TEXT("D must parse as D."), FGsId::ParseExact(KnownD, EGsIdFormat::D, Value));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdFormattingTest,
    "GsId.Text.Formatting",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdFormattingTest::RunTest(const FString& Parameters)
{
    using namespace GsIdAutomationTests;
    FGsId Value;
    if (!FGsId::Parse(KnownD, Value))
    {
        return false;
    }
    TestEqual(TEXT("D uppercase must be stable."), Value.ToString(EGsIdFormat::D), FString(KnownD));
    TestEqual(TEXT("N uppercase must be stable."), Value.ToString(EGsIdFormat::N), FString(KnownN));
    TestEqual(
        TEXT("N lowercase must be supported."),
        Value.ToString(EGsIdFormat::N, EGsIdLetterCase::Lower),
        FString(KnownN).ToLower()
    );
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdUnrealTextSerializationTest,
    "GsId.Text.UnrealExportImport",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdUnrealTextSerializationTest::RunTest(const FString& Parameters)
{
    using namespace GsIdAutomationTests;
    FGsId Original;
    if (!FGsId::Parse(KnownD, Original))
    {
        return false;
    }

    FString Exported;
    TestTrue(TEXT("ExportTextItem must succeed."), Original.ExportTextItem(Exported, FGsId::Empty(), nullptr, 0, nullptr));
    TestEqual(TEXT("Text export must use the stable D uppercase form."), Exported, FString(KnownD));

    const TCHAR* Cursor = *Exported;
    FGsId Imported;
    TestTrue(TEXT("ImportTextItem must accept the exported form."), Imported.ImportTextItem(Cursor, 0, nullptr, nullptr));
    TestTrue(TEXT("Unreal text serialization must round-trip."), Imported == Original);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdBytesRoundTripTest,
    "GsId.Bytes.RoundTrip",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdBytesRoundTripTest::RunTest(const FString& Parameters)
{
    const FGsId Original = FGsId::NewGsId();
    const TArray<uint8> Bytes = Original.ToByteArray();
    TestEqual(TEXT("A GsId must expose 32 bytes."), Bytes.Num(), FGsId::ByteLength);
    const FGsId Restored = FGsId::FromBytes(Bytes.GetData(), Bytes.Num());
    TestTrue(TEXT("The byte representation must round-trip."), Original == Restored);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdSerializationTest,
    "GsId.Binary.Serialization",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdSerializationTest::RunTest(const FString& Parameters)
{
    FGsId Original = FGsId::NewGsId();
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes, true);
    Writer << Original;
    TestEqual(TEXT("Binary serialization must write 32 bytes."), Bytes.Num(), FGsId::ByteLength);

    FGsId Restored;
    FMemoryReader Reader(Bytes, true);
    Reader << Restored;
    TestTrue(TEXT("Binary serialization must round-trip."), Original == Restored);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdReplicationTest,
    "GsId.Network.Replication",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdReplicationTest::RunTest(const FString& Parameters)
{
    FGsId Original = FGsId::NewGsId();
    TArray<uint8> Bytes;
    FMemoryWriter Writer(Bytes, true);
    bool bWriteSucceeded = false;
    Original.NetSerialize(Writer, nullptr, bWriteSucceeded);
    TestTrue(TEXT("Network serialization must succeed."), bWriteSucceeded);
    TestEqual(TEXT("Replication must write 32 bytes."), Bytes.Num(), FGsId::ByteLength);

    FGsId Restored;
    FMemoryReader Reader(Bytes, true);
    bool bReadSucceeded = false;
    Restored.NetSerialize(Reader, nullptr, bReadSucceeded);
    TestTrue(TEXT("Network deserialization must succeed."), bReadSucceeded);
    TestTrue(TEXT("Replication must round-trip."), Original == Restored);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdHashingTest,
    "GsId.Hashing.Comparison",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdHashingTest::RunTest(const FString& Parameters)
{
    const FGsId First = FGsId::NewGsId();
    const FGsId Copy = First;
    TArray<uint8> ChangedBytes = First.ToByteArray();
    ChangedBytes.Last() ^= 0x5Au;
    const FGsId Different = FGsId::FromBytes(ChangedBytes.GetData(), ChangedBytes.Num());

    TestTrue(TEXT("Copied GsIds must compare equal."), First == Copy);
    TestEqual(TEXT("Equal GsIds must have equal hashes."), GetTypeHash(First), GetTypeHash(Copy));
    TestTrue(TEXT("Changing one byte must change the value."), First != Different);
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdDeterministicShaTest,
    "GsId.Deterministic.Sha256",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdDeterministicShaTest::RunTest(const FString& Parameters)
{
    const TArray<uint8> Input{
        static_cast<uint8>('a'),
        static_cast<uint8>('b'),
        static_cast<uint8>('c')
    };
    const FGsId Value = FGsId::CreateDeterministic(MakeArrayView(Input));
    TestEqual(
        TEXT("Deterministic generation must use stable SHA-256 bytes."),
        Value.ToString(EGsIdFormat::N),
        FString(TEXT("BA7816BF8F01CFEA414140DE5DAE2223B00361A396177A9CB410FF61F20015AD"))
    );
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdBlueprintLibraryTest,
    "GsId.Blueprint.Library",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdBlueprintLibraryTest::RunTest(const FString& Parameters)
{
    FGsId Generated;
    TestTrue(TEXT("The Blueprint TryNewGsId node must report success."), UGsIdBlueprintLibrary::TryNewGsId(Generated));
    const FGsId Original = Generated;
    const FString Text = UGsIdBlueprintLibrary::ToString(Original, EGsIdFormat::D);
    FGsId Parsed;
    TestTrue(TEXT("The Blueprint parser must accept generated values."), UGsIdBlueprintLibrary::Parse(Text, Parsed));
    TestTrue(TEXT("Blueprint operations must preserve equality."), UGsIdBlueprintLibrary::EqualEqual_GsIdGsId(Original, Parsed));
    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdNativeInteropTest,
    "GsId.Native.Interop",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdNativeInteropTest::RunTest(const FString& Parameters)
{
    const GalacticShrine::GsId Native = GalacticShrine::GsId::NewGsId();
    const FGsId UnrealValue(Native);
    const GalacticShrine::GsId Restored = UnrealValue.ToNative();
    TestTrue(TEXT("Native and Unreal GsId bytes must remain identical."), Native == Restored);
    return true;
}


IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FGsIdBlueprintCategoryRootTest,
    "GsId.Localization.Blueprint.CategoryRoot",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter
)

bool FGsIdBlueprintCategoryRootTest::RunTest(const FString& Parameters)
{
    FString ExpandedCategory = FEditorCategoryUtils::GetCategoryDisplayString(
        TEXT("{GalacticShrine}|{GsId_Galactic_Shrine_GsId}")
    );
    TestFalse(TEXT("No legacy hyphenated root may remain."),
        ExpandedCategory.Contains(TEXT("Galactic-Shrine"))
            || ExpandedCategory.Contains(TEXT("Galactic- Shrine"))
            || ExpandedCategory.Contains(TEXT("Galactic - Shrine")));
    TestFalse(TEXT("The category must not contain the obsolete WORD JOINER."),
        ExpandedCategory.Contains(TEXT("\u2060")));
    TestFalse(TEXT("The category must not contain the obsolete U+2010 HYPHEN."),
        ExpandedCategory.Contains(TEXT("\u2010")));
    TestTrue(TEXT("The registered key must render with the accepted prefixed Galactic Shrine root."),
        ExpandedCategory.StartsWith(TEXT("# Galactic Shrine|")));
    return true;
}

#endif
