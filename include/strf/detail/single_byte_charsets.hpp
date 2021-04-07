#ifndef STRF_DETAIL_SINGLE_BYTE_CHARSETS_HPP
#define STRF_DETAIL_SINGLE_BYTE_CHARSETS_HPP

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/detail/facets/char_encoding.hpp>

#if ! defined(STRF_CHECK_DEST)

#define STRF_CHECK_DEST                          \
    STRF_IF_UNLIKELY (dest_it == dest_end) {     \
        ob.advance_to(dest_it);                  \
        ob.recycle();                            \
        STRF_IF_UNLIKELY (!ob.good()) {          \
            return;                              \
        }                                        \
        dest_it = ob.pointer();                  \
        dest_end = ob.end();                     \
    }

#define STRF_CHECK_DEST_SIZE(SIZE)                  \
    STRF_IF_UNLIKELY (dest_it + SIZE > dest_end) {  \
        ob.advance_to(dest_it);                     \
        ob.recycle();                               \
        STRF_IF_UNLIKELY (!ob.good()) {             \
            return;                                 \
        }                                           \
        dest_it = ob.pointer();                     \
        dest_end = ob.end();                        \
    }

#endif // ! defined(STRF_CHECK_DEST)

#define STRF_DEF_SINGLE_BYTE_CHARSET(CHARSET)                                 \
    template <typename CharT>                                                 \
    class static_char_encoding<CharT, strf::eid_ ## CHARSET>                  \
        : public strf::detail::single_byte_char_encoding                      \
            < CharT, strf::detail::impl_ ## CHARSET >                         \
    { };                                                                      \
                                                                              \
    template <typename SrcCharT, typename DestCharT>                          \
    class static_transcoder                                                   \
        < SrcCharT, DestCharT, strf::eid_ ## CHARSET, strf::eid_ ## CHARSET > \
        : public strf::detail::single_byte_char_encoding_sanitizer            \
            < SrcCharT, DestCharT, strf::detail::impl_ ## CHARSET >           \
    {};                                                                       \
                                                                              \
    template <typename SrcCharT, typename DestCharT>                          \
    class static_transcoder                                                   \
        < SrcCharT, DestCharT, strf::eid_utf32, strf::eid_ ## CHARSET >       \
        : public strf::detail::utf32_to_single_byte_char_encoding             \
            < SrcCharT, DestCharT, strf::detail::impl_ ## CHARSET >           \
    {};                                                                       \
                                                                              \
    template <typename SrcCharT, typename DestCharT>                          \
    class static_transcoder                                                   \
        < SrcCharT, DestCharT, strf::eid_ ## CHARSET, strf::eid_utf32 >       \
        : public strf::detail::single_byte_char_encoding_to_utf32             \
            < SrcCharT, DestCharT, strf::detail::impl_ ## CHARSET >           \
    {};                                                                       \
                                                                              \
    template <typename CharT>                                                 \
    using CHARSET = strf::static_char_encoding<CharT, strf::eid_ ## CHARSET>;

namespace strf {

namespace detail {

template<size_t SIZE, class T>
constexpr STRF_HD size_t array_size(T (&)[SIZE]) {
    return SIZE;
}

struct ch32_to_char
{
    char32_t key;
    unsigned value;
};

struct cmp_ch32_to_char
{
    STRF_HD bool operator()(ch32_to_char a, ch32_to_char b) const
    {
        return a.key < b.key;
    }
};

struct impl_ascii
{
    static STRF_HD const char* name() noexcept
    {
        return "ASCII";
    };
    static constexpr strf::char_encoding_id id = strf::eid_ascii;

    static STRF_HD bool is_valid(std::uint8_t ch)
    {
        return (ch & 0x80) == 0;
    }
    static STRF_HD char32_t decode(std::uint8_t ch)
    {
        if ((ch & 0x80) == 0)
            return ch;
        return 0xFFFD;
    }
    static STRF_HD unsigned encode(char32_t ch)
    {
        return ch < 0x80 ? ch : 0x100;
    }
};

struct impl_iso_8859_1
{
    static STRF_HD const char* name() noexcept
    {
        return "ISO-8859-1";
    };
    static constexpr strf::char_encoding_id id = strf::eid_iso_8859_1;

    static STRF_HD bool is_valid(std::uint8_t)
    {
        return true;
    }
    static STRF_HD char32_t decode(std::uint8_t ch)
    {
        return ch;
    }
    static STRF_HD unsigned encode(char32_t ch)
    {
        return ch;
    }
};

struct impl_iso_8859_3
{
    static STRF_HD const char* name() noexcept
    {
        return "ISO-8859-3";
    };
    static constexpr strf::char_encoding_id id = strf::eid_iso_8859_3;

    static STRF_HD bool is_valid(std::uint8_t ch)
    {
        return ch != 0xA5
            && ch != 0xAE
            && ch != 0xBE
            && ch != 0xC3
            && ch != 0xD0
            && ch != 0xE3
            && ch != 0xF0;
    }

    static STRF_HD unsigned encode(char32_t ch);

    static STRF_HD char32_t decode(std::uint8_t ch);
};

#if ! defined(STRF_OMIT_IMPL)

template< class ForwardIt, class T, class Compare >
ForwardIt STRF_HD lower_bound
    ( ForwardIt first
    , ForwardIt last
    , const T& value
    , Compare comp )
{
    auto search_range_length { last - first };
        // We don't have the equivalent of std::distance on the device-side

    ForwardIt iter;
    while (search_range_length > 0) {
        auto half_range_length = search_range_length/2;
        iter = first;
        iter += half_range_length;
        if (comp(*iter, value)) {
            first = ++iter;
            search_range_length -= (half_range_length + 1);
                // the extra +1 is since we've just checked the midpoint
        }
        else {
            search_range_length = half_range_length;
        }
    }
    return first;
}

STRF_FUNC_IMPL STRF_HD unsigned impl_iso_8859_3::encode(char32_t ch)
{
    STRF_IF_LIKELY (ch < 0xA1) {
        return ch;
    }
    static const ch32_to_char enc_map[] =
        { {0x00A3, 0xA3}, {0x00A4, 0xA4}, {0x00A7, 0xA7}, {0x00A8, 0xA8}
        , {0x00AD, 0xAD}, {0x00B0, 0xB0}, {0x00B2, 0xB2}, {0x00B3, 0xB3}
        , {0x00B4, 0xB4}, {0x00B5, 0xB5}, {0x00B7, 0xB7}, {0x00B8, 0xB8}
        , {0x00BD, 0xBD}, {0x00C0, 0xC0}, {0x00C1, 0xC1}, {0x00C2, 0xC2}
        , {0x00C4, 0xC4}, {0x00C7, 0xC7}, {0x00C8, 0xC8}, {0x00C9, 0xC9}
        , {0x00CA, 0xCA}, {0x00CB, 0xCB}, {0x00CC, 0xCC}, {0x00CD, 0xCD}
        , {0x00CE, 0xCE}, {0x00CF, 0xCF}, {0x00D1, 0xD1}, {0x00D2, 0xD2}
        , {0x00D3, 0xD3}, {0x00D4, 0xD4}, {0x00D6, 0xD6}, {0x00D7, 0xD7}
        , {0x00D9, 0xD9}, {0x00DA, 0xDA}, {0x00DB, 0xDB}, {0x00DC, 0xDC}
        , {0x00DF, 0xDF}, {0x00E0, 0xE0}, {0x00E1, 0xE1}, {0x00E2, 0xE2}
        , {0x00E4, 0xE4}, {0x00E7, 0xE7}, {0x00E8, 0xE8}, {0x00E9, 0xE9}
        , {0x00EA, 0xEA}, {0x00EB, 0xEB}, {0x00EC, 0xEC}, {0x00ED, 0xED}
        , {0x00EE, 0xEE}, {0x00EF, 0xEF}, {0x00F1, 0xF1}, {0x00F2, 0xF2}
        , {0x00F3, 0xF3}, {0x00F4, 0xF4}, {0x00F6, 0xF6}, {0x00F7, 0xF7}
        , {0x00F9, 0xF9}, {0x00FA, 0xFA}, {0x00FB, 0xFB}, {0x00FC, 0xFC}
        , {0x0108, 0xC6}, {0x0109, 0xE6}, {0x010A, 0xC5}, {0x010B, 0xE5}
        , {0x011C, 0xD8}, {0x011D, 0xF8}, {0x011E, 0xAB}, {0x011F, 0xBB}
        , {0x0120, 0xD5}, {0x0121, 0xF5}, {0x0124, 0xA6}, {0x0125, 0xB6}
        , {0x0126, 0xA1}, {0x0127, 0xB1}, {0x0130, 0xA9}, {0x0131, 0xB9}
        , {0x0134, 0xAC}, {0x0135, 0xBC}, {0x015C, 0xDE}, {0x015D, 0xFE}
        , {0x015E, 0xAA}, {0x015F, 0xBA}, {0x016C, 0xDD}, {0x016D, 0xFD}
        , {0x017B, 0xAF}, {0x017C, 0xBF}, {0x02D8, 0xA2}, {0x02D9, 0xFF} };

    const ch32_to_char* enc_map_end = enc_map + detail::array_size(enc_map);
    auto it = strf::detail::lower_bound
        ( enc_map, enc_map_end, ch32_to_char{ch, 0}, cmp_ch32_to_char{} );
    return it != enc_map_end && it->key == ch ? it->value : 0x100;
}

STRF_FUNC_IMPL STRF_HD char32_t impl_iso_8859_3::decode(std::uint8_t ch)
{
    STRF_IF_LIKELY (ch < 0xA1) {
        return ch;
    }
    else {
        constexpr unsigned short undef = 0xFFFD;
        static const unsigned short ext[] =
            { /* A0*/ 0x0126, 0x02D8, 0x00A3, 0x00A4,  undef, 0x0124, 0x00A7
            , 0x00A8, 0x0130, 0x015E, 0x011E, 0x0134, 0x00AD,  undef, 0x017B
            , 0x00B0, 0x0127, 0x00B2, 0x00B3, 0x00B4, 0x00B5, 0x0125, 0x00B7
            , 0x00B8, 0x0131, 0x015F, 0x011F, 0x0135, 0x00BD,  undef, 0x017C
            , 0x00C0, 0x00C1, 0x00C2,  undef, 0x00C4, 0x010A, 0x0108, 0x00C7
            , 0x00C8, 0x00C9, 0x00CA, 0x00CB, 0x00CC, 0x00CD, 0x00CE, 0x00CF
            ,  undef, 0x00D1, 0x00D2, 0x00D3, 0x00D4, 0x0120, 0x00D6, 0x00D7
            , 0x011C, 0x00D9, 0x00DA, 0x00DB, 0x00DC, 0x016C, 0x015C, 0x00DF
            , 0x00E0, 0x00E1, 0x00E2,  undef, 0x00E4, 0x010B, 0x0109, 0x00E7
            , 0x00E8, 0x00E9, 0x00EA, 0x00EB, 0x00EC, 0x00ED, 0x00EE, 0x00EF
            ,  undef, 0x00F1, 0x00F2, 0x00F3, 0x00F4, 0x0121, 0x00F6, 0x00F7
            , 0x011D, 0x00F9, 0x00FA, 0x00FB, 0x00FC, 0x016D, 0x015D, 0x02D9 };

        return ext[ch - 0xA1];
    }
}

#endif // ! defined(STRF_OMIT_IMPL)

class impl_iso_8859_15
{
public:

    static STRF_HD const char* name() noexcept
    {
        return "ISO-8859-15";
    };
    static constexpr strf::char_encoding_id id = strf::eid_iso_8859_15;

    static STRF_HD bool is_valid(std::uint8_t)
    {
        return true;
    }
    static STRF_HD char32_t decode(std::uint8_t ch)
    {
        static const unsigned short ext[] = {
            /*                           */ 0x20AC, 0x00A5, 0x0160, 0x00A7,
            0x0161, 0x00A9, 0x00AA, 0x00AB, 0x00AC, 0x00AD, 0x00AE, 0x00AF,
            0x00B0, 0x00B1, 0x00B2, 0x00B3, 0x017D, 0x00B5, 0x00B6, 0x00B7,
            0x017E, 0x00B9, 0x00BA, 0x00BB, 0x0152, 0x0153, 0x0178
        };

        STRF_IF_LIKELY (ch <= 0xA3 || 0xBF <= ch)
            return ch;

        return ext[ch - 0xA4];
    }

    static STRF_HD unsigned encode(char32_t ch)
    {
        return (ch < 0xA0 || (0xBE < ch && ch < 0x100)) ? ch : encode_ext(ch);
    }

private:

    static STRF_HD unsigned encode_ext(char32_t ch);
};

#if ! defined(STRF_OMIT_IMPL)

STRF_FUNC_IMPL STRF_HD unsigned impl_iso_8859_15::encode_ext(char32_t ch)
{
    switch(ch) {
        case 0x20AC: return 0xA4;
        case 0x0160: return 0xA6;
        case 0x0161: return 0xA8;
        case 0x017D: return 0xB4;
        case 0x017E: return 0xB8;
        case 0x0152: return 0xBC;
        case 0x0153: return 0xBD;
        case 0x0178: return 0xBE;
        case 0xA4:
        case 0xA6:
        case 0xA8:
        case 0xB4:
        case 0xB8:
        case 0xBC:
        case 0xBD:
        case 0xBE:
            return 0x100;
    }
    return ch;
}

#endif // ! defined(STRF_OMIT_IMPL)

class impl_windows_1252
{
public:
    // https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WindowsBestFit/bestfit1252.txt

    static STRF_HD const char* name() noexcept
    {
        return "windows-1252";
    };
    static constexpr strf::char_encoding_id id = strf::eid_windows_1252;

    constexpr static unsigned short decode_fail = 0xFFFF;

    static STRF_HD bool is_valid(std::uint8_t)
    {
        return true;
    }

    static STRF_HD char32_t decode(std::uint8_t ch)
    {
        STRF_IF_LIKELY (ch < 0x80 || 0x9F < ch) {
            return ch;
        }
        else {
            static const unsigned short ext[] = {
                0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021,
                0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
                0X0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014,
                0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178,
            };
            return ext[ch - 0x80];
        }
    }

    static STRF_HD unsigned encode(char32_t ch)
    {
        return (ch < 0x80 || (0x9F < ch && ch < 0x100)) ? ch : encode_ext(ch);
    }

private:

    static STRF_HD unsigned encode_ext(char32_t ch);
};

#if ! defined(STRF_OMIT_IMPL)

STRF_FUNC_IMPL STRF_HD unsigned impl_windows_1252::encode_ext(char32_t ch)
{
    switch(ch) {
        case 0x81: return 0x81;
        case 0x8D: return 0x8D;
        case 0x8F: return 0x8F;
        case 0x90: return 0x90;
        case 0x9D: return 0x9D;
        case 0x20AC: return 0x80;
        case 0x201A: return 0x82;
        case 0x0192: return 0x83;
        case 0x201E: return 0x84;
        case 0x2026: return 0x85;
        case 0x2020: return 0x86;
        case 0x2021: return 0x87;
        case 0x02C6: return 0x88;
        case 0x2030: return 0x89;
        case 0x0160: return 0x8A;
        case 0x2039: return 0x8B;
        case 0x0152: return 0x8C;
        case 0x017D: return 0x8E;
        case 0x2018: return 0x91;
        case 0x2019: return 0x92;
        case 0x201C: return 0x93;
        case 0x201D: return 0x94;
        case 0x2022: return 0x95;
        case 0x2013: return 0x96;
        case 0x2014: return 0x97;
        case 0x02DC: return 0x98;
        case 0x2122: return 0x99;
        case 0x0161: return 0x9A;
        case 0x203A: return 0x9B;
        case 0x0153: return 0x9C;
        case 0x017E: return 0x9E;
        case 0x0178: return 0x9F;
    }
    return 0x100;
}

#endif // ! defined(STRF_OMIT_IMPL)


template <typename SrcCharT, typename DestCharT, class Impl>
struct single_byte_char_encoding_to_utf32
{
    static STRF_HD void transcode
        ( strf::basic_outbuff<DestCharT>& ob
        , const SrcCharT* src
        , std::size_t src_size
        , strf::invalid_seq_notifier inv_seq_notifier
        , strf::surrogate_policy surr_poli );

    static constexpr STRF_HD std::size_t transcode_size
        ( const SrcCharT* src
        , std::size_t src_size
        , strf::surrogate_policy surr_poli ) noexcept
    {
        (void) src;
        (void) surr_poli;
        return src_size;
    }

    static STRF_HD strf::transcode_f<SrcCharT, DestCharT> transcode_func() noexcept
    {
        return transcode;
    }
    static STRF_HD strf::transcode_size_f<SrcCharT> transcode_size_func() noexcept
    {
        return transcode_size;
    }
};

template <typename SrcCharT, typename DestCharT, class Impl>
STRF_HD void single_byte_char_encoding_to_utf32<SrcCharT, DestCharT, Impl>::transcode
    ( strf::basic_outbuff<DestCharT>& ob
    , const SrcCharT* src
    , std::size_t src_size
    , strf::invalid_seq_notifier inv_seq_notifier
    , strf::surrogate_policy surr_poli )
{
    (void) surr_poli;
    auto dest_it = ob.pointer();
    auto dest_end = ob.end();
    auto src_end = src + src_size;
    for (auto src_it = src; src_it < src_end; ++src_it, ++dest_it) {
        STRF_CHECK_DEST;
        char32_t ch32 = Impl::decode(static_cast<std::uint8_t>(*src_it));
        STRF_IF_LIKELY (ch32 != 0xFFFD) {
            *dest_it = static_cast<DestCharT>(ch32);
        } else  {
            *dest_it = 0xFFFD;
            if (inv_seq_notifier) {
                ob.advance_to(dest_it + 1);
                inv_seq_notifier.notify();
            }
        }
    }
    ob.advance_to(dest_it);
}

template <typename SrcCharT, typename DestCharT, class Impl>
struct utf32_to_single_byte_char_encoding
{
    static STRF_HD void transcode
        ( strf::basic_outbuff<DestCharT>& ob
        , const SrcCharT* src
        , std::size_t src_size
        , strf::invalid_seq_notifier inv_seq_notifier
        , strf::surrogate_policy surr_poli );

    static constexpr STRF_HD std::size_t transcode_size
        ( const SrcCharT* src
        , std::size_t src_size
        , strf::surrogate_policy surr_poli ) noexcept
    {
        (void) src;
        (void) surr_poli;
        return src_size;
    }
    static STRF_HD strf::transcode_f<SrcCharT, DestCharT> transcode_func() noexcept
    {
        return transcode;
    }
    static STRF_HD strf::transcode_size_f<SrcCharT> transcode_size_func() noexcept
    {
        return transcode_size;
    }
};

template <typename SrcCharT, typename DestCharT, class Impl>
STRF_HD void utf32_to_single_byte_char_encoding<SrcCharT, DestCharT, Impl>::transcode
    ( strf::basic_outbuff<DestCharT>& ob
    , const SrcCharT* src
    , std::size_t src_size
    , strf::invalid_seq_notifier inv_seq_notifier
    , strf::surrogate_policy surr_poli )
{
    (void)surr_poli;
    auto dest_it = ob.pointer();
    auto dest_end = ob.end();
    auto src_end = src + src_size;
    for(auto src_it = src; src_it != src_end; ++src_it, ++dest_it) {
        STRF_CHECK_DEST;
        auto ch2 = Impl::encode(*src_it);
        STRF_IF_LIKELY (ch2 < 0x100) {
            * dest_it = static_cast<DestCharT>(ch2);
        } else {
            * dest_it = '?';
            if (inv_seq_notifier) {
                ob.advance_to(dest_it + 1);
                inv_seq_notifier.notify();
            }
        }
    }
    ob.advance_to(dest_it);
}

template <typename SrcCharT, typename DestCharT, class Impl>
struct single_byte_char_encoding_sanitizer
{
    static STRF_HD void transcode
        ( strf::basic_outbuff<DestCharT>& ob
        , const SrcCharT* src
        , std::size_t src_size
        , strf::invalid_seq_notifier inv_seq_notifier
        , strf::surrogate_policy surr_poli );

    static constexpr STRF_HD std::size_t transcode_size
        ( const SrcCharT* src
        , std::size_t src_size
        , strf::surrogate_policy surr_poli ) noexcept
    {
        (void) src;
        (void) surr_poli;
        return src_size;
    }

    static STRF_HD strf::transcode_f<SrcCharT, DestCharT> transcode_func() noexcept
    {
        return transcode;
    }
    static STRF_HD strf::transcode_size_f<SrcCharT> transcode_size_func() noexcept
    {
        return transcode_size;
    }
};

template <typename SrcCharT, typename DestCharT, class Impl>
STRF_HD void single_byte_char_encoding_sanitizer<SrcCharT, DestCharT, Impl>::transcode
    ( strf::basic_outbuff<DestCharT>& ob
    , const SrcCharT* src
    , std::size_t src_size
    , strf::invalid_seq_notifier inv_seq_notifier
    , strf::surrogate_policy surr_poli )
{
    (void) surr_poli;
    auto dest_it = ob.pointer();
    auto dest_end = ob.end();
    auto src_end = src + src_size;
    for (auto src_it = src; src_it < src_end; ++src_it, ++dest_it) {
        STRF_CHECK_DEST;
        auto ch = static_cast<std::uint8_t>(*src_it);
        STRF_IF_LIKELY (Impl::is_valid(ch)) {
            *dest_it = static_cast<SrcCharT>(ch);
        }
        else {
            *dest_it = '?';
            STRF_IF_UNLIKELY (inv_seq_notifier) {
                ob.advance_to(dest_it);
                inv_seq_notifier.notify();
            }
        }
    }
    ob.advance_to(dest_it);
}

template <std::size_t wchar_size, typename CharT, strf::char_encoding_id>
class single_byte_char_encoding_tofrom_wchar
{
public:

    static STRF_HD strf::dynamic_transcoder<wchar_t, CharT>
    find_transcoder_from(strf::tag<wchar_t>, strf::char_encoding_id) noexcept
    {
        return {};
    }
    static STRF_HD strf::dynamic_transcoder<CharT, wchar_t>
    find_transcoder_to(strf::tag<wchar_t>, strf::char_encoding_id) noexcept
    {
        return {};
    }

protected:

    constexpr static std::nullptr_t find_transcoder_to_wchar = nullptr;
    constexpr static std::nullptr_t  find_transcoder_from_wchar = nullptr;
};

template <typename CharT, strf::char_encoding_id Id>
class single_byte_char_encoding_tofrom_wchar<4, CharT, Id>
{
public:

    static STRF_HD strf::dynamic_transcoder<wchar_t, CharT>
    find_transcoder_from(strf::tag<wchar_t>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_from_wchar(id);
    }
    static STRF_HD strf::dynamic_transcoder<CharT, wchar_t>
    find_transcoder_to(strf::tag<wchar_t>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_to_wchar(id);
    }

protected:

    static STRF_HD strf::dynamic_transcoder<wchar_t, CharT>
    find_transcoder_from_wchar(strf::char_encoding_id id) noexcept
    {
        using return_type = strf::dynamic_transcoder<wchar_t, CharT>;
        if (id == strf::eid_utf32) {
            strf::static_transcoder<wchar_t, CharT, strf::eid_utf32, Id> t;
            return return_type{t};
        }
        return {};
    }
    static STRF_HD strf::dynamic_transcoder<CharT, wchar_t>
    find_transcoder_to_wchar(strf::char_encoding_id id) noexcept
    {
        using return_type = strf::dynamic_transcoder<CharT, wchar_t>;
        if (id == strf::eid_utf32) {
            strf::static_transcoder<CharT, wchar_t, Id, strf::eid_utf32> t;
            return return_type{t};
        }
        return {};
    }
};


template <typename CharT, class Impl>
class single_byte_char_encoding
    : public strf::detail::single_byte_char_encoding_tofrom_wchar
        < sizeof(wchar_t), CharT, Impl::id >
{
    static_assert(sizeof(CharT) == 1, "Character type with this encoding");

    using wchar_stuff_ =
        strf::detail::single_byte_char_encoding_tofrom_wchar<sizeof(wchar_t), CharT, Impl::id>;
public:

    using char_type = CharT;

    static STRF_HD const char* name() noexcept
    {
        return Impl::name();
    };
    static constexpr STRF_HD strf::char_encoding_id id() noexcept
    {
        return Impl::id;
    }
    static constexpr STRF_HD char32_t replacement_char() noexcept
    {
        return U'?';
    }
    static constexpr STRF_HD std::size_t replacement_char_size() noexcept
    {
        return 1;
    }
    static STRF_HD void write_replacement_char(strf::basic_outbuff<CharT>& ob)
    {
        strf::put(ob, static_cast<CharT>('?'));
    }
    static STRF_HD std::size_t validate(char32_t ch32) noexcept
    {
        return Impl::encode(ch32) < 0x100 ? 1 : (std::size_t)-1;
    }
    static constexpr STRF_HD std::size_t encoded_char_size(char32_t) noexcept
    {
        return 1;
    }
    static STRF_HD CharT* encode_char(CharT* dest, char32_t ch);

    static STRF_HD void encode_fill
        ( strf::basic_outbuff<CharT>& ob, std::size_t count, char32_t ch );

    static STRF_HD strf::codepoints_count_result codepoints_fast_count
        ( const CharT* src, std::size_t src_size
        , std::size_t max_count ) noexcept
    {
        (void) src;
        if (max_count < src_size) {
            return {max_count, max_count};
        }
        return {src_size, src_size};
    }
    static STRF_HD strf::codepoints_count_result codepoints_robust_count
        ( const CharT* src, std::size_t src_size
        , std::size_t max_count, strf::surrogate_policy surr_poli ) noexcept
    {
        (void) src;
        (void) surr_poli;
        if (max_count < src_size) {
            return {max_count, max_count};
        }
        return {src_size, src_size};
    }
    static STRF_HD char32_t decode_char(CharT ch)
    {
        return Impl::decode(static_cast<std::uint8_t>(ch));
    }
    static STRF_HD strf::encode_char_f<CharT> encode_char_func() noexcept
    {
        return encode_char;
    }
    static STRF_HD strf::encode_fill_f<CharT> encode_fill_func() noexcept
    {
        return encode_fill;
    }
    static STRF_HD strf::validate_f validate_func() noexcept
    {
        return validate;
    }
    static STRF_HD
    strf::write_replacement_char_f<CharT> write_replacement_char_func() noexcept
    {
        return write_replacement_char;
    }
    static constexpr
    STRF_HD strf::static_transcoder<CharT, CharT, Impl::id, Impl::id>
    sanitizer() noexcept
    {
        return {};
    }
    static constexpr STRF_HD
    strf::static_transcoder<char32_t, CharT, strf::eid_utf32, Impl::id>
    from_u32() noexcept
    {
        return {};
    }
    static constexpr STRF_HD
    strf::static_transcoder<CharT, char32_t, Impl::id, strf::eid_utf32>
    to_u32() noexcept
    {
        return {};
    }

    using wchar_stuff_::find_transcoder_from;
    using wchar_stuff_::find_transcoder_to;

    static STRF_HD strf::dynamic_transcoder<char, CharT>
    find_transcoder_from(strf::tag<char>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_from_narrow<char>(id);;
    }
    static STRF_HD strf::dynamic_transcoder<CharT, char>
    find_transcoder_to(strf::tag<char>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_to_narrow<char>(id);
    }

#if defined (__cpp_char8_t)

    static STRF_HD strf::dynamic_transcoder<char8_t, CharT>
    find_transcoder_from(strf::tag<char8_t>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_from_narrow<char8_t>(id);
    }
    static STRF_HD strf::dynamic_transcoder<CharT, char8_t>
    find_transcoder_to(strf::tag<char8_t>, strf::char_encoding_id id) noexcept
    {
        return find_transcoder_to_narrow<char8_t>(id);
    }

#endif

    void STRF_HD fill_data(strf::dynamic_char_encoding_data<CharT>& data) const noexcept
    {
        data.name = name();
        data.id = id();
        data.replacement_char = replacement_char();
        data.replacement_char_size = 1;
        data.validate_func = validate;
        data.encoded_char_size_func = encoded_char_size;
        data.encode_char_func = encode_char;
        data.encode_fill_func = encode_fill;
        data.codepoints_fast_count_func = codepoints_fast_count;
        data.codepoints_robust_count_func = codepoints_robust_count;
        data.write_replacement_char_func = write_replacement_char;
        data.decode_char_func = decode_char;

        data.sanitizer = strf::dynamic_transcoder<CharT, CharT>{sanitizer()};
        data.from_u32 = strf::dynamic_transcoder<char32_t, CharT>{from_u32()};
        data.to_u32 = strf::dynamic_transcoder<CharT, char32_t>{to_u32()};

        data.find_transcoder_from_wchar = wchar_stuff_::find_transcoder_from_wchar;
        data.find_transcoder_to_wchar = wchar_stuff_::find_transcoder_to_wchar;

        data.find_transcoder_from_char16 = nullptr;
        data.find_transcoder_to_char16 = nullptr;

        data.find_transcoder_from_char = find_transcoder_from_narrow<char>;
        data.find_transcoder_to_char = find_transcoder_to_narrow<char>;

#if defined (__cpp_char8_t)
        data.find_transcoder_from_char8 = find_transcoder_from_narrow<char8_t>;
        data.find_transcoder_to_char8 = find_transcoder_to_narrow<char8_t>;

#else
        data.find_transcoder_from_char8 = nullptr;
        data.find_transcoder_to_char8 = nullptr;
#endif
    }

    static strf::dynamic_char_encoding<CharT> to_dynamic() noexcept
    {
        static const strf::dynamic_char_encoding_data<CharT> data = {
            name(), id(), replacement_char(), 1, validate, encoded_char_size,
            encode_char, encode_fill, codepoints_fast_count,
            codepoints_robust_count, write_replacement_char, decode_char,
            strf::dynamic_transcoder<CharT, CharT>{sanitizer()},
            strf::dynamic_transcoder<char32_t, CharT>{from_u32()},
            strf::dynamic_transcoder<CharT, char32_t>{to_u32()},
            wchar_stuff_::find_transcoder_from_wchar,
            wchar_stuff_::find_transcoder_to_wchar,
            nullptr,
            nullptr,
            find_transcoder_from_narrow<char>,
            find_transcoder_to_narrow<char>,
#if defined (__cpp_char8_t)
            find_transcoder_from_narrow<char8_t>,
            find_transcoder_to_narrow<char8_t>,
#else
            nullptr,
            nullptr,
#endif // defined (__cpp_char8_t)
        };
        return strf::dynamic_char_encoding<CharT>{data};
    }
    explicit operator strf::dynamic_char_encoding<CharT> () const
    {
        return to_dynamic();
    }

private:

    template <typename SrcCharT>
    static STRF_HD strf::dynamic_transcoder<SrcCharT, CharT>
    find_transcoder_from_narrow(strf::char_encoding_id id) noexcept
    {
        using transcoder_type = strf::dynamic_transcoder<SrcCharT, CharT>;
        if (id == Impl::id) {
            static_transcoder<SrcCharT, CharT, Impl::id, Impl::id> t;
            return transcoder_type{ t };
        }
        return {};
    }
    template <typename DestCharT>
    static STRF_HD strf::dynamic_transcoder<CharT, DestCharT>
    find_transcoder_to_narrow(strf::char_encoding_id id) noexcept
    {
        using transcoder_type = strf::dynamic_transcoder<CharT, DestCharT>;
        if (id == Impl::id) {
            static_transcoder<CharT, DestCharT, Impl::id, Impl::id> t;
            return transcoder_type{ t };
        }
        return {};
    }

};

template <typename CharT, class Impl>
STRF_HD CharT* single_byte_char_encoding<CharT, Impl>::encode_char
    ( CharT* dest
    , char32_t ch )
{
    auto ch2 = Impl::encode(ch);
    bool valid = (ch2 < 0x100);
    *dest = static_cast<CharT>(valid * ch2 + (!valid) * '?');
    return dest + 1;
}

template <typename CharT, class Impl>
STRF_HD void single_byte_char_encoding<CharT, Impl>::encode_fill
    ( strf::basic_outbuff<CharT>& ob, std::size_t count, char32_t ch )
{
    unsigned ch2 = Impl::encode(ch);
    STRF_IF_UNLIKELY (ch2 >= 0x100) {
        ch2 = '?';
    }
    auto ch3 = static_cast<CharT>(ch2);
    while(true) {
        std::size_t available = ob.space();
        STRF_IF_LIKELY (count <= available) {
            strf::detail::str_fill_n<CharT>(ob.pointer(), count, ch3);
            ob.advance(count);
            return;
        }
        strf::detail::str_fill_n<CharT>(ob.pointer(), available, ch3);
        ob.advance_to(ob.end());
        count -= available;
        ob.recycle();
    }
}

} // namespace detail

STRF_DEF_SINGLE_BYTE_CHARSET(ascii);
STRF_DEF_SINGLE_BYTE_CHARSET(iso_8859_1);
STRF_DEF_SINGLE_BYTE_CHARSET(iso_8859_3);
STRF_DEF_SINGLE_BYTE_CHARSET(iso_8859_15);
STRF_DEF_SINGLE_BYTE_CHARSET(windows_1252);

} // namespace strf


#endif  // STRF_DETAIL_SINGLE_BYTE_CHAR_ENCODINGS_HPP

