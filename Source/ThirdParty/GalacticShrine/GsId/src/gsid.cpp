#include <galactic_shrine/gsid/gsid.hpp>

#include <algorithm>
#include <array>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <fstream>
#include <limits>
#include <mutex>
#include <ostream>

#if defined(_WIN32)
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
    #include <windows.h>
    #include <bcrypt.h>
#elif defined(__APPLE__)
    #include <Security/Security.h>
#elif defined(__linux__)
    #include <sys/random.h>
#endif

namespace GalacticShrine
{
    namespace
    {
        constexpr std::string_view UpperHexAlphabet = "0123456789ABCDEF";
        constexpr std::string_view LowerHexAlphabet = "0123456789abcdef";
        constexpr std::array<std::size_t, GsIdConstants::HyphenCount> HyphenPositions{16, 25, 34, 43, 52};

        struct OptionsState final
        {
            std::mutex Mutex;
            GsIdOptionsValues Values{};
            bool Locked = false;
        };

        OptionsState& GetOptionsState()
        {
            static OptionsState State;
            return State;
        }

        void EnsureUnlocked(const OptionsState& State)
        {
            if (State.Locked)
            {
                throw GsIdException("Les options GsId sont verrouillees et ne peuvent plus etre modifiees.");
            }
        }

        void ValidateFormat(const GsIdFormat Format)
        {
            if (Format != GsIdFormat::N && Format != GsIdFormat::D)
            {
                throw GsIdException("Le format GsId demande n'est pas supporte.");
            }
        }

        std::string_view Trim(std::string_view Value) noexcept
        {
            while (!Value.empty() && std::isspace(static_cast<unsigned char>(Value.front())) != 0)
            {
                Value.remove_prefix(1);
            }
            while (!Value.empty() && std::isspace(static_cast<unsigned char>(Value.back())) != 0)
            {
                Value.remove_suffix(1);
            }
            return Value;
        }

        bool IsHexCharacter(const char Character) noexcept
        {
            return (Character >= '0' && Character <= '9')
                || (Character >= 'a' && Character <= 'f')
                || (Character >= 'A' && Character <= 'F');
        }

        std::uint8_t ConvertHexCharacter(const char Character)
        {
            if (Character >= '0' && Character <= '9') return static_cast<std::uint8_t>(Character - '0');
            if (Character >= 'a' && Character <= 'f') return static_cast<std::uint8_t>(10 + Character - 'a');
            if (Character >= 'A' && Character <= 'F') return static_cast<std::uint8_t>(10 + Character - 'A');
            throw GsIdException("Le GsId contient un caractere non hexadecimal.");
        }

        bool IsHyphenPosition(const std::size_t Index) noexcept
        {
            return std::find(HyphenPositions.begin(), HyphenPositions.end(), Index) != HyphenPositions.end();
        }

        void FillSecureRandom(std::span<std::uint8_t> Destination)
        {
#if defined(_WIN32)
            if (Destination.size() > static_cast<std::size_t>(std::numeric_limits<ULONG>::max()))
            {
                throw GsIdException("Le tampon demande est trop grand pour BCryptGenRandom.");
            }
            const NTSTATUS Status = BCryptGenRandom(
                nullptr,
                reinterpret_cast<PUCHAR>(Destination.data()),
                static_cast<ULONG>(Destination.size()),
                BCRYPT_USE_SYSTEM_PREFERRED_RNG);
            if (Status < 0)
            {
                throw GsIdException("BCryptGenRandom n'a pas pu generer un GsId securise.");
            }
#elif defined(__APPLE__)
            if (SecRandomCopyBytes(kSecRandomDefault, Destination.size(), Destination.data()) != errSecSuccess)
            {
                throw GsIdException("SecRandomCopyBytes n'a pas pu generer un GsId securise.");
            }
#elif defined(__linux__)
            std::size_t Offset = 0;
            while (Offset < Destination.size())
            {
                const ssize_t Received = ::getrandom(
                    Destination.data() + Offset,
                    Destination.size() - Offset,
                    0);
                if (Received > 0)
                {
                    Offset += static_cast<std::size_t>(Received);
                    continue;
                }
                if (Received < 0 && errno == EINTR)
                {
                    continue;
                }
                throw GsIdException(
                    std::string("getrandom n'a pas pu generer un GsId securise : ") + std::strerror(errno));
            }
#else
            std::ifstream Source("/dev/urandom", std::ios::binary);
            if (!Source)
            {
                throw GsIdException("Aucune source aleatoire cryptographiquement sure n'est disponible.");
            }
            Source.read(reinterpret_cast<char*>(Destination.data()), static_cast<std::streamsize>(Destination.size()));
            if (Source.gcount() != static_cast<std::streamsize>(Destination.size()))
            {
                throw GsIdException("/dev/urandom n'a pas retourne assez d'octets.");
            }
#endif
        }

        std::array<char, GsIdConstants::HexLength> EncodeNormalized(
            const GsId::ByteArray& Bytes,
            const GsIdCase LetterCase) noexcept
        {
            const std::string_view Alphabet = LetterCase == GsIdCase::Lower ? LowerHexAlphabet : UpperHexAlphabet;
            std::array<char, GsIdConstants::HexLength> Result{};
            for (std::size_t Index = 0; Index < Bytes.size(); ++Index)
            {
                Result[Index * 2] = Alphabet[Bytes[Index] >> 4];
                Result[Index * 2 + 1] = Alphabet[Bytes[Index] & 0x0F];
            }
            return Result;
        }
    }

    GsIdCase GsIdOptions::GetDefaultCase()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Values.DefaultCase;
    }

    GsIdFormat GsIdOptions::GetDefaultTextFormat()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Values.DefaultTextFormat;
    }

    GsIdFormat GsIdOptions::GetDefaultJsonFormat()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Values.DefaultJsonFormat;
    }

    GsIdFormat GsIdOptions::GetDefaultDatabaseFormat()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Values.DefaultDatabaseFormat;
    }

    GsIdOptionsValues GsIdOptions::GetValues()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Values;
    }

    bool GsIdOptions::IsLocked()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        return State.Locked;
    }

    void GsIdOptions::SetDefaultCase(const GsIdCase Value)
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        State.Values.DefaultCase = Value;
    }

    void GsIdOptions::SetDefaultTextFormat(const GsIdFormat Value)
    {
        ValidateFormat(Value);
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        State.Values.DefaultTextFormat = Value;
    }

    void GsIdOptions::SetDefaultJsonFormat(const GsIdFormat Value)
    {
        ValidateFormat(Value);
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        State.Values.DefaultJsonFormat = Value;
    }

    void GsIdOptions::SetDefaultDatabaseFormat(const GsIdFormat Value)
    {
        ValidateFormat(Value);
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        State.Values.DefaultDatabaseFormat = Value;
    }

    void GsIdOptions::Configure(
        const std::optional<GsIdCase> DefaultCase,
        const std::optional<GsIdFormat> DefaultTextFormat,
        const std::optional<GsIdFormat> DefaultJsonFormat,
        const std::optional<GsIdFormat> DefaultDatabaseFormat)
    {
        if (DefaultTextFormat) ValidateFormat(*DefaultTextFormat);
        if (DefaultJsonFormat) ValidateFormat(*DefaultJsonFormat);
        if (DefaultDatabaseFormat) ValidateFormat(*DefaultDatabaseFormat);

        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        if (DefaultCase) State.Values.DefaultCase = *DefaultCase;
        if (DefaultTextFormat) State.Values.DefaultTextFormat = *DefaultTextFormat;
        if (DefaultJsonFormat) State.Values.DefaultJsonFormat = *DefaultJsonFormat;
        if (DefaultDatabaseFormat) State.Values.DefaultDatabaseFormat = *DefaultDatabaseFormat;
    }

    void GsIdOptions::Lock()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        State.Locked = true;
    }

    void GsIdOptions::Reset()
    {
        OptionsState& State = GetOptionsState();
        std::scoped_lock Lock(State.Mutex);
        EnsureUnlocked(State);
        State.Values = GsIdOptionsValues{};
    }

    GsId GsId::NewGsId() { return GsIdGenerator::NewGsId(); }
    bool GsId::TryNewGsId(GsId& Result) noexcept { return GsIdGenerator::TryNewGsId(Result); }
    GsId GsId::FromBytes(const std::span<const std::uint8_t> Bytes)
    {
        if (Bytes.size() != GsIdConstants::ByteLength)
        {
            throw GsIdException("Un GsId brut doit contenir exactement 32 octets.");
        }
        ByteArray Result{};
        std::copy(Bytes.begin(), Bytes.end(), Result.begin());
        return GsId(Result);
    }
    GsId GsId::FromNormalized(const std::string_view Value) { return GsIdParser::Parse(Value); }
    GsId GsId::FromString(const std::string_view Value) { return GsIdParser::Parse(Value); }
    GsId GsId::Parse(const std::string_view Value) { return GsIdParser::Parse(Value); }
    bool GsId::TryParse(const std::string_view Value, GsId& Result) noexcept { return GsIdParser::TryParse(Value, Result); }
    std::optional<GsId> GsId::TryParse(const std::string_view Value) noexcept { return GsIdParser::TryParse(Value); }

    bool GsId::IsEmpty() const noexcept
    {
        return std::all_of(bytes_.begin(), bytes_.end(), [](const std::uint8_t Value) { return Value == 0; });
    }

    std::string GsId::ToString() const
    {
        const GsIdOptionsValues Options = GsIdOptions::GetValues();
        return ToString(Options.DefaultTextFormat, Options.DefaultCase);
    }

    std::string GsId::ToString(const GsIdFormat Format) const
    {
        return ToString(Format, GsIdOptions::GetDefaultCase());
    }

    std::string GsId::ToString(const GsIdFormat Format, const GsIdCase LetterCase) const
    {
        ValidateFormat(Format);
        const std::size_t Length = Format == GsIdFormat::N
            ? GsIdConstants::HexLength
            : GsIdConstants::FormattedLength;
        std::string Result(Length, '\0');
        std::size_t CharsWritten = 0;
        if (!TryFormat(std::span<char>(Result.data(), Result.size()), CharsWritten, Format, LetterCase))
        {
            throw GsIdException("Impossible de formater le GsId.");
        }
        Result.resize(CharsWritten);
        return Result;
    }

    std::string GsId::ToString(const char Format) const
    {
        switch (Format)
        {
            case 'N': return ToString(GsIdFormat::N, GsIdCase::Upper);
            case 'D': return ToString(GsIdFormat::D, GsIdCase::Upper);
            case 'n': return ToString(GsIdFormat::N, GsIdCase::Lower);
            case 'd': return ToString(GsIdFormat::D, GsIdCase::Lower);
            default: throw GsIdException("Le format GsId demande n'est pas supporte.");
        }
    }

    std::string GsId::ToNormalizedString() const { return ToString(GsIdFormat::N, GsIdOptions::GetDefaultCase()); }
    std::string GsId::ToNormalizedString(const GsIdCase LetterCase) const { return ToString(GsIdFormat::N, LetterCase); }

    bool GsId::TryFormat(
        const std::span<char> Destination,
        std::size_t& CharsWritten,
        const GsIdFormat Format,
        const GsIdCase LetterCase) const noexcept
    {
        CharsWritten = 0;
        if (Format != GsIdFormat::N && Format != GsIdFormat::D) return false;
        const std::size_t Required = Format == GsIdFormat::N
            ? GsIdConstants::HexLength
            : GsIdConstants::FormattedLength;
        if (Destination.size() < Required) return false;

        const auto Normalized = EncodeNormalized(bytes_, LetterCase);
        if (Format == GsIdFormat::N)
        {
            std::copy(Normalized.begin(), Normalized.end(), Destination.begin());
            CharsWritten = GsIdConstants::HexLength;
            return true;
        }

        std::size_t SourceIndex = 0;
        for (std::size_t DestinationIndex = 0; DestinationIndex < GsIdConstants::FormattedLength; ++DestinationIndex)
        {
            if (IsHyphenPosition(DestinationIndex))
            {
                Destination[DestinationIndex] = '-';
            }
            else
            {
                Destination[DestinationIndex] = Normalized[SourceIndex++];
            }
        }
        CharsWritten = GsIdConstants::FormattedLength;
        return true;
    }

    bool GsId::TryFormat(
        const std::span<char> Destination,
        std::size_t& CharsWritten,
        const GsIdFormat Format) const noexcept
    {
        return TryFormat(Destination, CharsWritten, Format, GsIdOptions::GetDefaultCase());
    }

    bool GsId::TryFormat(const std::span<char> Destination, std::size_t& CharsWritten) const noexcept
    {
        const GsIdOptionsValues Options = GsIdOptions::GetValues();
        return TryFormat(Destination, CharsWritten, Options.DefaultTextFormat, Options.DefaultCase);
    }

    GsId GsIdGenerator::NewGsId()
    {
        GsId::ByteArray Bytes{};
        FillSecureRandom(Bytes);
        return GsId(Bytes);
    }

    bool GsIdGenerator::TryNewGsId(GsId& Result) noexcept
    {
        try
        {
            Result = NewGsId();
            return true;
        }
        catch (...)
        {
            Result = GsId::Empty();
            return false;
        }
    }

    GsId GsIdParser::Parse(const std::string_view Input)
    {
        const std::string_view Value = Trim(Input);
        const bool bNormalized = Value.size() == GsIdConstants::HexLength;
        const bool bFormatted = Value.size() == GsIdConstants::FormattedLength;
        if (!bNormalized && !bFormatted)
        {
            throw GsIdException("Un GsId doit contenir 64 caracteres hexadecimaux ou suivre le format D de 69 caracteres.");
        }

        GsId::ByteArray Bytes{};
        std::size_t NibbleIndex = 0;
        std::uint8_t CurrentByte = 0;
        for (std::size_t Index = 0; Index < Value.size(); ++Index)
        {
            if (bFormatted && IsHyphenPosition(Index))
            {
                if (Value[Index] != '-') throw GsIdException("Les separateurs du format D sont invalides.");
                continue;
            }
            if (!IsHexCharacter(Value[Index])) throw GsIdException("Le GsId contient un caractere non hexadecimal.");
            const std::uint8_t Nibble = ConvertHexCharacter(Value[Index]);
            if ((NibbleIndex & 1u) == 0u)
            {
                CurrentByte = static_cast<std::uint8_t>(Nibble << 4);
            }
            else
            {
                CurrentByte = static_cast<std::uint8_t>(CurrentByte | Nibble);
                Bytes[NibbleIndex / 2] = CurrentByte;
            }
            ++NibbleIndex;
        }
        if (NibbleIndex != GsIdConstants::HexLength) throw GsIdException("Le GsId ne contient pas exactement 32 octets.");
        return GsId(Bytes);
    }

    bool GsIdParser::TryParse(const std::string_view Value, GsId& Result) noexcept
    {
        try
        {
            Result = Parse(Value);
            return true;
        }
        catch (...)
        {
            Result = GsId::Empty();
            return false;
        }
    }

    std::optional<GsId> GsIdParser::TryParse(const std::string_view Value) noexcept
    {
        GsId Result;
        if (!TryParse(Value, Result)) return std::nullopt;
        return Result;
    }

    std::string GsIdParser::Normalize(const std::string_view Value)
    {
        return Normalize(Value, GsIdOptions::GetDefaultCase());
    }

    std::string GsIdParser::Normalize(const std::string_view Value, const GsIdCase LetterCase)
    {
        return Parse(Value).ToString(GsIdFormat::N, LetterCase);
    }

    bool GsIdValidator::IsValid(const std::string_view Value) noexcept
    {
        GsId Result;
        return GsIdParser::TryParse(Value, Result);
    }

    bool GsIdValidator::IsValid(const std::string_view Input, const GsIdFormat Format) noexcept
    {
        const std::string_view Value = Trim(Input);
        if ((Format == GsIdFormat::N && Value.size() != GsIdConstants::HexLength)
            || (Format == GsIdFormat::D && Value.size() != GsIdConstants::FormattedLength))
        {
            return false;
        }
        GsId Result;
        return GsIdParser::TryParse(Value, Result);
    }

    std::ostream& operator<<(std::ostream& Stream, const GsId& Value)
    {
        Stream << Value.ToString();
        return Stream;
    }
}

std::size_t std::hash<GalacticShrine::GsId>::operator()(const GalacticShrine::GsId& Value) const noexcept
{
    std::size_t Hash = sizeof(std::size_t) == 8
        ? static_cast<std::size_t>(1469598103934665603ull)
        : static_cast<std::size_t>(2166136261u);
    const std::size_t Prime = sizeof(std::size_t) == 8
        ? static_cast<std::size_t>(1099511628211ull)
        : static_cast<std::size_t>(16777619u);
    for (const std::uint8_t Byte : Value.Bytes())
    {
        Hash ^= static_cast<std::size_t>(Byte);
        Hash *= Prime;
    }
    return Hash;
}
