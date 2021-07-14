#ifndef STRF_PRINTER_HPP
#define STRF_PRINTER_HPP

//  Copyright (C) (See commit logs on github.com/robhz786/strf)
//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <strf/outbuff.hpp>
#include <strf/width_t.hpp>
#include <strf/facets_pack.hpp>

namespace strf {

template <typename CharT>
class printer
{
public:

    using char_type = CharT;

    STRF_HD virtual ~printer()
    {
    }

    STRF_HD virtual void print_to(strf::basic_outbuff<CharT>& ob) const = 0;
};

struct string_input_tag_base
{
};

template <typename CharIn>
struct string_input_tag: string_input_tag_base
{
};

template <typename CharT>
struct is_string_of
{
    template <typename T>
    using fn = std::is_base_of<string_input_tag<CharT>, T>;
};

template <typename T>
using is_string = std::is_base_of<string_input_tag_base, T>;

template <typename CharIn>
struct tr_string_input_tag: strf::string_input_tag<CharIn>
{
};

template <typename CharIn>
struct range_separator_input_tag: strf::string_input_tag<CharIn>
{
};

template <typename CharIn>
struct is_tr_string_of
{
    template <typename T>
    using fn = std::is_same<strf::tr_string_input_tag<CharIn>, T>;
};

template <typename T>
struct is_tr_string: std::false_type
{
};

template <typename CharIn>
struct is_tr_string<strf::is_tr_string_of<CharIn>> : std::true_type
{
};

template <bool Active>
class width_preview;

template <>
class width_preview<true>
{
public:

    explicit constexpr STRF_HD width_preview(strf::width_t initial_width) noexcept
        : width_(initial_width)
    {}

    STRF_HD width_preview(const width_preview&) = delete;

    STRF_CONSTEXPR_IN_CXX14 STRF_HD void subtract_width(strf::width_t w) noexcept
    {
        if (w < width_) {
            width_ -= w;
        } else {
            width_ = 0;
        }
    }

    template <typename IntT>
    STRF_CONSTEXPR_IN_CXX14
    strf::detail::enable_if_t<std::is_integral<IntT>::value>
    STRF_HD subtract_width(IntT w) noexcept
    {
        subtract_int_(std::is_signed<IntT>{}, w);
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD void clear_remaining_width() noexcept
    {
        width_ = 0;
    }
    constexpr STRF_HD strf::width_t remaining_width() const noexcept
    {
        return width_;
    }

private:

    template <typename W>
    void STRF_HD subtract_int_(std::true_type, W w) noexcept
    {
        if (w > 0) {
            if (w <= static_cast<int>(width_.floor())) {
                width_ -= static_cast<std::uint16_t>(w);
            } else {
                width_ = 0;
            }
        }
    }

    template <typename W>
    void STRF_HD subtract_int_(std::false_type, W w) noexcept
    {
        if (w <= width_.floor()) {
            width_ -= static_cast<std::uint16_t>(w);
        } else {
            width_ = 0;
        }
    }

    strf::width_t width_;
};

template <>
class width_preview<false>
{
public:

    constexpr STRF_HD width_preview() noexcept
    {
    }

    template <typename T>
    STRF_CONSTEXPR_IN_CXX14 STRF_HD void subtract_width(T) const noexcept
    {
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD void clear_remaining_width() noexcept
    {
    }

    constexpr STRF_HD strf::width_t remaining_width() const noexcept
    {
        return 0;
    }
};

template <bool Active>
class size_preview;

template <>
class size_preview<true>
{
public:
    explicit constexpr STRF_HD size_preview(std::size_t initial_size = 0) noexcept
        : size_(initial_size)
    {
    }

    STRF_HD size_preview(const size_preview&) = delete;

    STRF_CONSTEXPR_IN_CXX14 STRF_HD void add_size(std::size_t s) noexcept
    {
        size_ += s;
    }

    constexpr STRF_HD std::size_t accumulated_size() const noexcept
    {
        return size_;
    }

private:

    std::size_t size_;
};

template <>
class size_preview<false>
{
public:

    constexpr STRF_HD size_preview() noexcept
    {
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD void add_size(std::size_t) noexcept
    {
    }

    constexpr STRF_HD std::size_t accumulated_size() const noexcept
    {
        return 0;
    }
};

enum class preview_width: bool { no = false, yes = true };
enum class preview_size : bool { no = false, yes = true };

template <strf::preview_size SizeRequired, strf::preview_width WidthRequired>
class print_preview
    : public strf::size_preview<static_cast<bool>(SizeRequired)>
    , public strf::width_preview<static_cast<bool>(WidthRequired)>
{
public:

    static constexpr bool size_required = static_cast<bool>(SizeRequired);
    static constexpr bool width_required = static_cast<bool>(WidthRequired);
    static constexpr bool nothing_required = ! size_required && ! width_required;
    static constexpr bool something_required = size_required || width_required;

    template <strf::preview_width W = WidthRequired>
    STRF_HD constexpr explicit print_preview
        ( strf::detail::enable_if_t<static_cast<bool>(W), strf::width_t> initial_width ) noexcept
        : strf::width_preview<true>{initial_width}
    {
    }

    constexpr STRF_HD print_preview() noexcept
    {
    }
};

using no_print_preview = strf::print_preview<strf::preview_size::no, strf::preview_width::no>;
using print_size_preview  = strf::print_preview<strf::preview_size::yes, strf::preview_width::no>;
using print_width_preview = strf::print_preview<strf::preview_size::no, strf::preview_width::yes>;
using print_size_and_width_preview = strf::print_preview<strf::preview_size::yes, strf::preview_width::yes>;

namespace detail {

#if defined(__cpp_fold_expressions)

template <typename CharT, typename ... Printers>
inline STRF_HD void write_args( strf::basic_outbuff<CharT>& ob
                              , const Printers& ... printers )
{
    (... , printers.print_to(ob));
}

#else // defined(__cpp_fold_expressions)

template <typename CharT>
inline STRF_HD void write_args(strf::basic_outbuff<CharT>&)
{
}

template <typename CharT, typename Printer, typename ... Printers>
inline STRF_HD void write_args
    ( strf::basic_outbuff<CharT>& ob
    , const Printer& printer
    , const Printers& ... printers )
{
    printer.print_to(ob);
    if (ob.good()) {
        write_args<CharT>(ob, printers ...);
    }
}

#endif // defined(__cpp_fold_expressions)

} // namespace detail

namespace detail{

template
    < class From
    , class To
    , template <class ...> class List
    , class ... T >
struct fmt_replace_impl2
{
    template <class U>
    using f = strf::detail::conditional_t<std::is_same<From, U>::value, To, U>;

    using type = List<f<T> ...>;
};

template <class From, class List>
struct fmt_replace_impl;

template
    < class From
    , template <class ...> class List
    , class ... T>
struct fmt_replace_impl<From, List<T ...> >
{
    template <class To>
    using type_tmpl =
        typename strf::detail::fmt_replace_impl2
            < From, To, List, T...>::type;
};

template <typename FmtA, typename FmtB, typename ValueWithFormat>
struct fmt_forward_switcher
{
    template <typename FmtAInit>
    static STRF_HD const typename FmtB::template fn<ValueWithFormat>&
    f(const FmtAInit&, const ValueWithFormat& v)
    {
        return v;
    }

    template <typename FmtAInit>
    static STRF_HD typename FmtB::template fn<ValueWithFormat>&&
    f(const FmtAInit&, ValueWithFormat&& v)
    {
        return v;
    }
};

template <typename FmtA, typename ValueWithFormat>
struct fmt_forward_switcher<FmtA, FmtA, ValueWithFormat>
{
    template <typename FmtAInit>
    static constexpr STRF_HD FmtAInit&&
    f(strf::detail::remove_reference_t<FmtAInit>& fa,  const ValueWithFormat&)
    {
        return static_cast<FmtAInit&&>(fa);
    }

    template <typename FmtAInit>
    static constexpr STRF_HD FmtAInit&&
    f(strf::detail::remove_reference_t<FmtAInit>&& fa, const ValueWithFormat&)
    {
        return static_cast<FmtAInit&&>(fa);
    }
};

} // namespace detail

template <typename List, typename From, typename To>
using fmt_replace
    = typename strf::detail::fmt_replace_impl<From, List>
    ::template type_tmpl<To>;

template <typename PrintTraits, class ... Fmts>
class value_with_formatters;

template <typename PrintTraits, class ... Fmts>
class value_with_formatters
    : public Fmts::template fn<value_with_formatters<PrintTraits, Fmts ...>> ...
{
public:
    using traits = PrintTraits;
    using value_type = typename PrintTraits::forwarded_type;

    template <typename ... OtherFmts>
    using replace_fmts = strf::value_with_formatters<PrintTraits, OtherFmts ...>;

    explicit constexpr STRF_HD value_with_formatters(const value_type& v)
        : value_(v)
    {
    }

    template <typename OtherPrintTraits>
    constexpr STRF_HD value_with_formatters
        ( const value_type& v
        , const strf::value_with_formatters<OtherPrintTraits, Fmts...>& f )
        : Fmts::template fn<value_with_formatters<PrintTraits, Fmts...>>
            ( static_cast
              < const typename Fmts
             :: template fn<value_with_formatters<OtherPrintTraits, Fmts...>>& >(f) )
        ...
        , value_(v)
    {
    }

    template <typename OtherPrintTraits>
    constexpr STRF_HD value_with_formatters
        ( const value_type& v
        , strf::value_with_formatters<OtherPrintTraits, Fmts...>&& f )
        : Fmts::template fn<value_with_formatters<PrintTraits, Fmts...>>
            ( static_cast
              < typename Fmts
             :: template fn<value_with_formatters<OtherPrintTraits, Fmts...>> &&>(f) )
        ...
        , value_(static_cast<value_type&&>(v))
    {
    }

    template <typename... F, typename... FInit>
    constexpr STRF_HD value_with_formatters
        ( const value_type& v
        , strf::tag<F...>
        , FInit&&... finit )
        : F::template fn<value_with_formatters<PrintTraits, Fmts...>>
            (std::forward<FInit>(finit))
        ...
        , value_(v)
    {
    }

    template <typename... OtherFmts>
    constexpr STRF_HD explicit value_with_formatters
        ( const strf::value_with_formatters<PrintTraits, OtherFmts...>& f )
        : Fmts::template fn<value_with_formatters<PrintTraits, Fmts...>>
            ( static_cast
              < const typename OtherFmts
             :: template fn<value_with_formatters<PrintTraits, OtherFmts ...>>& >(f) )
        ...
        , value_(f.value())
    {
    }

    template <typename ... OtherFmts>
    constexpr STRF_HD explicit value_with_formatters
        ( strf::value_with_formatters<PrintTraits, OtherFmts...>&& f )
        : Fmts::template fn<value_with_formatters<PrintTraits, Fmts...>>
            ( static_cast
              < typename OtherFmts
             :: template fn<value_with_formatters<PrintTraits, OtherFmts ...>>&& >(f) )
        ...
        , value_(static_cast<value_type&&>(f.value()))
    {
    }

    template <typename Fmt, typename FmtInit, typename ... OtherFmts>
    constexpr STRF_HD value_with_formatters
        ( const strf::value_with_formatters<PrintTraits, OtherFmts...>& f
        , strf::tag<Fmt>
        , FmtInit&& fmt_init )
        : Fmts::template fn<value_with_formatters<PrintTraits, Fmts...>>
            ( strf::detail::fmt_forward_switcher
                  < Fmt
                  , Fmts
                  , strf::value_with_formatters<PrintTraits, OtherFmts...> >
              :: template f<FmtInit>(fmt_init, f) )
            ...
        , value_(f.value())
    {
    }

    constexpr STRF_HD const value_type& value() const
    {
        return value_;
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD value_type& value()
    {
        return value_;
    }

private:

    value_type value_;
};

template <typename CharT, typename Preview, typename FPack, typename Arg, typename Printer>
struct usual_printer_input;

template< typename CharT
        , strf::preview_size PreviewSize
        , strf::preview_width PreviewWidth
        , typename FPack
        , typename Arg
        , typename Printer >
struct usual_printer_input
    < CharT, strf::print_preview<PreviewSize, PreviewWidth>, FPack, Arg, Printer >
{
    using char_type = CharT;
    using arg_type = Arg;
    using preview_type = strf::print_preview<PreviewSize, PreviewWidth>;
    using fpack_type = FPack;
    using printer_type = Printer;

    preview_type& preview;
    FPack facets;
    Arg arg;
};

template<typename T>
struct print_traits;

template<typename PrintTraits, typename... Fmts>
struct print_traits<strf::value_with_formatters<PrintTraits, Fmts...>> : PrintTraits
{
};

namespace detail {

template <typename T>
struct print_traits_finder;

} // namespace detail

template <typename T>
using print_traits_of = typename
    detail::print_traits_finder<strf::detail::remove_cvref_t<T>>
    ::traits;

struct print_traits_tag
{
private:
    static const print_traits_tag& tag_();

public:

    template < typename Arg >
    constexpr STRF_HD auto operator()(Arg&&) const -> strf::print_traits_of<Arg>
    {
        return {};
    }
};

namespace detail {

template <typename T>
struct has_print_traits_specialization
{
    template <typename U, typename = typename strf::print_traits<U>::forwarded_type>
    static std::true_type test(const U*);

    template <typename U>
    static std::false_type test(...);

    using T_ = strf::detail::remove_cvref_t<T>;
    using result = decltype(test<T_>((const T_*)0));

    constexpr static bool value = result::value;
};

struct print_traits_tag;

struct select_print_traits_specialization
{
    template <typename T>
    using select = strf::print_traits<T>;
};

struct select_print_traits_from_tag_invoke
{
    template <typename T>
    using select = decltype
        ( strf::detail::tag_invoke(strf::print_traits_tag{}, std::declval<T>() ));
};

template <typename T>
struct print_traits_finder
{
    using selector_ = strf::detail::conditional_t
        < strf::detail::has_print_traits_specialization<T>::value
        , strf::detail::select_print_traits_specialization
        , strf::detail::select_print_traits_from_tag_invoke >;

    using traits = typename selector_::template select<T>;
    using forwarded_type = typename traits::forwarded_type;
};

template <typename Traits, typename... F>
struct print_traits_finder<strf::value_with_formatters<Traits, F...>>
{
    using traits = Traits;
    using forwarded_type = strf::value_with_formatters<Traits, F...>;
};

template <typename T>
struct print_traits_finder<T&> : print_traits_finder<T>
{
};

template <typename T>
struct print_traits_finder<T&&> : print_traits_finder<T>
{
};

template <typename T>
struct print_traits_finder<const T> : print_traits_finder<T>
{
};

template <typename T>
struct print_traits_finder<volatile T> : print_traits_finder<T>
{
};

template <typename PrintTraits, typename Formatters>
struct mp_define_value_with_formatters;

template < typename PrintTraits
         , template <class...> class List
         , typename... Fmts >
struct mp_define_value_with_formatters<PrintTraits, List<Fmts...>>
{
    using type = strf::value_with_formatters<PrintTraits, Fmts...>;
};

template <typename PrintTraits>
struct extract_formatters_from_print_traits_impl
{
private:
    template <typename U, typename Fmts = typename U::formatters>
    static Fmts get_formatters_(U*);

    template <typename U>
    static strf::tag<> get_formatters_(...);

    template <typename U>
    using result_ = decltype(get_formatters_<U>((U*)0));

public:

    using type = result_<PrintTraits>;
};

template <typename PrintTraits>
using extract_formatters_from_print_traits =
    typename extract_formatters_from_print_traits_impl<PrintTraits>::type;

template <typename PrintTraits>
using default_value_with_formatter_of_print_traits = typename
    strf::detail::mp_define_value_with_formatters
        < PrintTraits
        , extract_formatters_from_print_traits<PrintTraits> >
    :: type;

template <typename T>
struct formatters_finder
{
    using traits = typename print_traits_finder<T>::traits;
    using formatters = extract_formatters_from_print_traits<traits>;
    using fmt_type = typename
        strf::detail::mp_define_value_with_formatters<traits, formatters>::type;
};

template <typename PrintTraits, typename... Fmts>
struct formatters_finder<strf::value_with_formatters<PrintTraits, Fmts...>>
{
    using traits = PrintTraits;
    using formatters = strf::tag<Fmts...>;
    using fmt_type = strf::value_with_formatters<PrintTraits, Fmts...>;
};

} // namespace detail

template <typename T>
using forwarded_printable_type = typename
    detail::print_traits_finder<strf::detail::remove_cvref_t<T>>
    ::forwarded_type;

template <typename T>
using fmt_type = typename
    detail::formatters_finder<strf::detail::remove_cvref_t<T>>
    ::fmt_type;

template <typename T>
using fmt_value_type = typename fmt_type<T>::value_type;

template <typename T>
using formatters_of = typename strf::detail::formatters_finder<T>::formatters;

inline namespace format_functions {

#if defined (STRF_NO_GLOBAL_CONSTEXPR_VARIABLE)

template <typename T>
constexpr STRF_HD fmt_type<T> fmt(T&& value)
    noexcept(noexcept(fmt_type<T>{fmt_value_type<T>{value}}))
{
    return fmt_type<T>{fmt_value_type<T>{value}};
}

#else //defined (STRF_NO_GLOBAL_CONSTEXPR_VARIABLE)

namespace detail_format_functions {

struct fmt_fn
{
    template <typename T>
    constexpr STRF_HD fmt_type<T> operator()(T&& value) const
        noexcept(noexcept(fmt_type<T>{fmt_value_type<T>{(T&&)value}}))
    {
        return fmt_type<T>{fmt_value_type<T>{(T&&)value}};
    }
};

} // namespace detail_format_functions

constexpr detail_format_functions::fmt_fn fmt {};

#endif

} // inline namespace format_functions

struct print_override_c;
struct no_print_override;

namespace detail {

template <typename T>
struct has_override_tag_helper
{
    template <typename U, typename F = typename U::override_tag>
    static STRF_HD std::true_type test_(const U*);

    template <typename U>
    static STRF_HD std::false_type test_(...);

    using result = decltype(test_<T>((T*)0));
};

template <typename T>
struct has_override_tag: has_override_tag_helper<T>::result{};

namespace mk_pr_in {

template < typename UnadaptedMaker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg >
struct can_make_printer_input_impl
{
    template <typename P>
    static STRF_HD auto test_(Preview& preview, const FPack& facets, const Arg& arg)
        -> decltype( std::declval<const P&>()
                         .make_printer_input
                             ( strf::tag<CharT>{}, preview, facets, arg )
                   , std::true_type{} );

    template <typename P>
    static STRF_HD std::false_type test_(...);

    using result = decltype
        ( test_<UnadaptedMaker>
            ( std::declval<Preview&>()
            , std::declval<FPack>()
            , std::declval<Arg>() ));
};

template < typename UnadaptedMaker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg >
using can_make_printer_input = typename
    can_make_printer_input_impl<UnadaptedMaker, CharT, Preview, FPack, Arg>
    ::result;

struct arg_adapter_rm_fmt
{
    template <typename PrintTraits, typename... Fmts>
    static constexpr STRF_HD const typename PrintTraits::forwarded_type&
    adapt_arg(const strf::value_with_formatters<PrintTraits, Fmts...>& x)
    {
        return x.value();
    }
};

template <typename To>
struct arg_adapter_cast
{
    template <typename From>
    static constexpr STRF_HD To adapt_arg(const From& x)
    {
        return static_cast<To>(x);
    }
};

template < typename PrintTraits
         , typename Maker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Vwf
         , typename DefaultVwf >
struct adapter_selector_3
{
    static_assert( ! std::is_same<Vwf, DefaultVwf>::value, "");
    using adapter_type = arg_adapter_cast<const Vwf&>;
};

template < typename PrintTraits
         , typename Maker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename DefaultVwf >
struct adapter_selector_3<PrintTraits, Maker, CharT, Preview, FPack, DefaultVwf, DefaultVwf>
{
    static constexpr bool can_pass_directly =
        can_make_printer_input<Maker, CharT, Preview, FPack, DefaultVwf>
        ::value;

    using adapter_type = typename std::conditional
        < can_pass_directly
        , arg_adapter_cast<const DefaultVwf&>
        , arg_adapter_rm_fmt >
        ::type;
};

template < typename PrintTraits
         , typename Maker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg
         , typename DefaultVwf >
struct adapter_selector_2
{
    static constexpr bool can_pass_directly =
        can_make_printer_input<Maker, CharT, Preview, FPack, Arg>
        ::value;
    static constexpr bool can_pass_as_fmt =
        can_make_printer_input<Maker, CharT, Preview, FPack, DefaultVwf>
        ::value;
    static constexpr bool shall_adapt = !can_pass_directly && can_pass_as_fmt;

    using destination_type = typename std::conditional
        < shall_adapt, DefaultVwf, typename PrintTraits::forwarded_type>
        :: type;
    using adapter_type = arg_adapter_cast<destination_type>;
};

template < typename PrintTraits
         , typename Maker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename DefaultVwf
         , typename... Fmts >
struct adapter_selector_2
    < PrintTraits, Maker, CharT, Preview, FPack
    , strf::value_with_formatters<PrintTraits, Fmts...>, DefaultVwf>
{
    using vwf = strf::value_with_formatters<PrintTraits, Fmts...>;
    using other = adapter_selector_3
        < PrintTraits, Maker, CharT, Preview, FPack, vwf, DefaultVwf>;
    using adapter_type = typename other::adapter_type;
};

template < typename PrintTraits
         , typename Maker
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg >
struct adapter_selector
{
    using vwf = default_value_with_formatter_of_print_traits<PrintTraits>;
    using other = adapter_selector_2<PrintTraits, Maker, CharT, Preview, FPack, Arg, vwf>;
    using adapter_type = typename other::adapter_type;
};

template < typename PrintTraits, typename Maker, typename CharT
         , typename Preview, typename FPack, typename Arg >
using select_adapter = typename
    adapter_selector<PrintTraits, Maker, CharT, Preview, FPack, Arg>
    ::adapter_type;

template <typename Overrider, typename OverrideTag>
struct maker_getter_overrider
{
    using return_maker_type = const Overrider&;
    using maker_type = Overrider;

    template <typename FPack>
    static constexpr STRF_HD return_maker_type get_maker(const FPack& fp)
    {
        return strf::use_facet<strf::print_override_c, OverrideTag>(fp);
    }
};

template <typename PrintTraits>
struct maker_getter_print_traits
{
    using return_maker_type = PrintTraits;
    using maker_type = PrintTraits;

    template <typename FPack>
    static constexpr STRF_HD maker_type get_maker(const FPack&)
    {
        return maker_type{};
    }
};

template < typename PrintTraits
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg
         , bool HasOverrideTag >
struct maker_getter_selector_2
{
    static_assert(HasOverrideTag, "");
    using override_tag = typename PrintTraits::override_tag;
    using overrider_ = decltype
        ( strf::use_facet<strf::print_override_c, override_tag>(*(const FPack*)0) );
    using overrider = strf::detail::remove_cvref_t<overrider_>;
    using maker_getter_type = typename std::conditional
        < std::is_same<overrider, strf::no_print_override>::value
        , maker_getter_print_traits<PrintTraits>
        , maker_getter_overrider<overrider, override_tag> >
        ::type;
};

template < typename PrintTraits
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg >
struct maker_getter_selector_2<PrintTraits, CharT, Preview, FPack, Arg, false>
{
    using maker_getter_type = maker_getter_print_traits<PrintTraits>;
};

template < typename PrintTraits
         , typename CharT
         , typename Preview
         , typename FPack
         , typename Arg >
struct maker_getter_selector
{
    using other = maker_getter_selector_2
        < PrintTraits, CharT, Preview, FPack, Arg
        , has_override_tag<PrintTraits>::value >;
    using maker_getter_type = typename other::maker_getter_type;
};

template < typename PrintTraits, typename CharT, typename Preview
         , typename FPack, typename Arg >
using select_maker_getter = typename maker_getter_selector
    <PrintTraits, CharT, Preview, FPack, Arg>
    :: maker_getter_type;

template <typename CharT, typename Preview, typename FPack, typename Arg>
struct selector
{
    using traits = strf::print_traits_of<Arg>;
    using maker_getter_type = select_maker_getter<traits, CharT, Preview, FPack, Arg>;
    using maker_type = typename maker_getter_type::maker_type;
    using adapter_type = select_adapter<traits, maker_type, CharT, Preview, FPack, Arg>;
};

template <typename CharT, typename Preview, typename FPack, typename Arg>
struct selector_no_override
{
    using traits = strf::print_traits_of<Arg>;
    using maker_getter_type = maker_getter_print_traits<traits>;
    using adapter_type = select_adapter<traits, traits, CharT, Preview, FPack, Arg>;
};

template < typename CharT, typename Preview, typename FPack, typename Arg
         , typename Selector = selector<CharT, Preview, FPack, Arg> >
struct helper: Selector::maker_getter_type, Selector::adapter_type
{
};

template < typename CharT, typename Preview, typename FPack, typename Arg
         , typename Selector = selector_no_override<CharT, Preview, FPack, Arg> >
struct helper_no_override: Selector::maker_getter_type, Selector::adapter_type
{
};

} // namespace mk_pr_in
} // namespace detail

template < typename CharT
         , typename Preview
         , typename FPack
         , typename Arg
         , typename Helper
             = strf::detail::mk_pr_in::helper_no_override<CharT, Preview, FPack, Arg>
         , typename Maker = typename Helper::maker_type
         , typename ChTag = strf::tag<CharT> >
constexpr STRF_HD auto make_default_printer_input(Preview& p, const FPack& fp, const Arg& arg)
    noexcept(noexcept(Maker::make_printer_input(ChTag{}, p, fp, Helper::adapt_arg(arg))))
    -> decltype(Maker::make_printer_input(ChTag{}, p, fp, Helper::adapt_arg(arg)))
{
    return Maker::make_printer_input(ChTag{}, p, fp, Helper::adapt_arg(arg));
}

template < typename CharT
         , typename Preview
         , typename FPack
         , typename Arg
         , typename Helper = strf::detail::mk_pr_in::helper<CharT, Preview, FPack, Arg>
         , typename Maker = typename Helper::maker_type
         , typename ChTag = strf::tag<CharT> >
constexpr STRF_HD auto make_printer_input(Preview& p, const FPack& fp, const Arg& arg)
    -> decltype(((const Maker*)0)->make_printer_input(ChTag{}, p, fp, Helper::adapt_arg(arg)))
{
    return Helper::get_maker(fp)
        .make_printer_input(strf::tag<CharT>{}, p, fp, Helper::adapt_arg(arg));
}

struct no_print_override
{
    using category = print_override_c;
    template <typename CharT, typename Preview, typename FPack, typename Arg>
    constexpr static STRF_HD auto make_printer_input
        ( strf::tag<CharT>
        , Preview& preview
        , const FPack& facets
        , Arg&& arg )
        noexcept(noexcept(strf::make_default_printer_input<CharT>(preview, facets, arg)))
        -> decltype(strf::make_default_printer_input<CharT>(preview, facets, arg))
    {
        return strf::make_default_printer_input<CharT>(preview, facets, arg);
    }
};

struct print_override_c
{
    static constexpr bool constrainable = true;

    constexpr static STRF_HD no_print_override get_default() noexcept
    {
        return {};
    }
};

#if defined(STRF_HAS_VARIABLE_TEMPLATES)

template <typename T>
constexpr bool is_overridable
    = strf::detail::has_override_tag<strf::print_traits_of<T>>::value;

#endif // defined(STRF_HAS_VARIABLE_TEMPLATES)

template <typename T>
using override_tag = typename strf::print_traits_of<T>::override_tag;

template <typename CharT, typename Preview, typename FPack, typename Arg>
using printer_input_type = decltype
    ( strf::make_printer_input<CharT>( std::declval<Preview&>()
                                     , std::declval<const FPack&>()
                                     , std::declval<Arg>() ) );

template <typename CharT, typename Preview, typename FPack, typename Arg>
using printer_type = typename printer_input_type<CharT, Preview, FPack, Arg>::printer_type;

template < typename CharT
         , strf::preview_size SizeRequired
         , strf::preview_width WidthRequired
         , typename... FPE >
STRF_HD void preview( strf::print_preview<SizeRequired, WidthRequired>&
                    , const strf::facets_pack<FPE...> &)
{
}

template < typename CharT
         , typename... FPE
         , typename Arg
         , typename... Args >
STRF_HD STRF_CONSTEXPR_IN_CXX14 void preview
    ( strf::print_preview<strf::preview_size::no, strf::preview_width::no>
    , const strf::facets_pack<FPE...>&
    , const Arg&
    , const Args&... ) noexcept
{
}

namespace detail {

template < typename CharT, typename... FPE >
STRF_HD STRF_CONSTEXPR_IN_CXX14 void preview_only_width
    ( strf::print_preview<strf::preview_size::no, strf::preview_width::yes>&
    , const strf::facets_pack<FPE...>& ) noexcept
{
}

template < typename CharT
         , typename... FPE
         , typename Arg
         , typename... OtherArgs >
STRF_HD void preview_only_width
    ( strf::print_preview<strf::preview_size::no, strf::preview_width::yes>& pp
    , const strf::facets_pack<FPE...>& facets
    , const Arg& arg
    , const OtherArgs&... other_args )
{
    using preview_type = strf::print_preview<strf::preview_size::no, strf::preview_width::yes>;

    (void) strf::printer_type<CharT, preview_type, strf::facets_pack<FPE...>, Arg>
        ( strf::make_printer_input<CharT>(pp, facets, arg) );

    if (pp.remaining_width() > 0) {
        strf::detail::preview_only_width<CharT>(pp, facets, other_args...);
    }
}

} // namespace detail

template <typename CharT, typename... FPE, typename... Args>
STRF_HD void preview
    ( strf::print_preview<strf::preview_size::no, strf::preview_width::yes>& pp
    , const strf::facets_pack<FPE...>& facets
    , const Args&... args )
{
    if (pp.remaining_width() > 0) {
        strf::detail::preview_only_width<CharT>(pp, facets, args...);
    }
}

namespace detail {

template <typename... Args>
STRF_HD STRF_CONSTEXPR_IN_CXX14 void do_nothing_with(const Args...) noexcept
{
    // workaround for the lack of support for fold expressions
}

} // namespace detail

template < typename CharT
         , strf::preview_width WidthRequired
         , typename... FPE
         , typename... Args >
STRF_HD void preview
    ( strf::print_preview<strf::preview_size::yes, WidthRequired>& pp
    , const strf::facets_pack<FPE...>& facets
    , const Args&... args )
{
    using preview_type = strf::print_preview<strf::preview_size::yes, WidthRequired>;
    strf::detail::do_nothing_with
        ( strf::printer_type<CharT, preview_type, strf::facets_pack<FPE...>, Args>
          ( strf::make_printer_input<CharT>(pp, facets, args) ) ... );
}

template <typename> struct is_char: public std::false_type {};

#if defined(__cpp_char8_t)
template <> struct is_char<char8_t>: public std::true_type {};
#endif
template <> struct is_char<char>: public std::true_type {};
template <> struct is_char<char16_t>: public std::true_type {};
template <> struct is_char<char32_t>: public std::true_type {};
template <> struct is_char<wchar_t>: public std::true_type {};

enum class showsign {negative_only = 0, positive_also = '+', fill_instead_of_positive = ' '};

template <bool HasAlignment>
struct alignment_formatter_q;

enum class text_alignment {left, right, center};

struct alignment_format
{
    constexpr STRF_HD alignment_format
        ( char32_t fill_ = U' '
        , strf::width_t width_ = 0
        , strf::text_alignment alignment_ = strf::text_alignment::right ) noexcept
        : fill(fill_)
        , width(width_)
        , alignment(alignment_)
    {
    }
    constexpr alignment_format(const alignment_format&) = default;

    char32_t fill = U' ';
    strf::width_t width = 0;
    strf::text_alignment alignment = strf::text_alignment::right;

};

struct default_alignment_format
{
    static constexpr char32_t fill = U' ';
    static constexpr strf::width_t width = 0;
    static constexpr strf::text_alignment alignment = strf::text_alignment::right;

    constexpr operator strf::alignment_format () const noexcept
    {
        return {};
    }
};

template <class T, bool HasAlignment>
class alignment_formatter_fn;

template <class T>
class alignment_formatter_fn<T, true>
{
    STRF_HD T&& move_self_downcast_()
    {
        T* d =  static_cast<T*>(this);
        return static_cast<T&&>(*d);
    }

public:

    constexpr STRF_HD alignment_formatter_fn() noexcept
    {
    }

    constexpr STRF_HD explicit alignment_formatter_fn
        ( strf::alignment_format data) noexcept
        : data_(data)
    {
    }

    template <typename U, bool B>
    constexpr STRF_HD explicit alignment_formatter_fn
        ( const strf::alignment_formatter_fn<U, B>& u ) noexcept
        : data_(u.get_alignment_format())
    {
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& operator<(strf::width_t width) && noexcept
    {
        data_.alignment = strf::text_alignment::left;
        data_.width = width;
        return move_self_downcast_();
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& operator>(strf::width_t width) && noexcept
    {
        data_.alignment = strf::text_alignment::right;
        data_.width = width;
        return move_self_downcast_();
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& operator^(strf::width_t width) && noexcept
    {
        data_.alignment = strf::text_alignment::center;
        data_.width = width;
        return move_self_downcast_();
    }
    template < typename CharT >
    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& fill(CharT ch) && noexcept
    {
        static_assert( strf::is_char<CharT>::value // issue #19
                     , "Refusing non-char argument to set the fill character, "
                       "since one may pass 0 instead of '0' by accident." );
        data_.fill = ch;
        return move_self_downcast_();
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& set_alignment_format(strf::alignment_format data) && noexcept
    {
        data_ = data;
        return move_self_downcast_();
    }
    constexpr STRF_HD strf::width_t width() const noexcept
    {
        return data_.width;
    }
    constexpr STRF_HD strf::text_alignment alignment() const noexcept
    {
        return data_.alignment;
    }
    constexpr STRF_HD char32_t fill() const noexcept
    {
        return data_.fill;
    }

    constexpr STRF_HD alignment_format get_alignment_format() const noexcept
    {
        return data_;
    }

private:

    strf::alignment_format data_;
};

template <class T>
class alignment_formatter_fn<T, false>
{
    using derived_type = T;
    using adapted_derived_type = strf::fmt_replace
            < T
            , strf::alignment_formatter_q<false>
            , strf::alignment_formatter_q<true> >;

public:

    constexpr STRF_HD alignment_formatter_fn() noexcept
    {
    }

    template <typename U>
    constexpr STRF_HD explicit alignment_formatter_fn(const alignment_formatter_fn<U, false>&) noexcept
    {
    }

    constexpr STRF_HD adapted_derived_type operator<(strf::width_t width) const noexcept
    {
        return adapted_derived_type
            { self_downcast_()
            , strf::tag<alignment_formatter_q<true>>{}
            , strf::alignment_format{U' ', width, strf::text_alignment::left} };
    }
    constexpr STRF_HD adapted_derived_type operator>(strf::width_t width) const noexcept
    {
        return adapted_derived_type
            { self_downcast_()
            , strf::tag<alignment_formatter_q<true>>{}
            , strf::alignment_format{U' ', width, strf::text_alignment::right} };
    }
    constexpr STRF_HD adapted_derived_type operator^(strf::width_t width) const noexcept
    {
        return adapted_derived_type
            { self_downcast_()
            , strf::tag<alignment_formatter_q<true>>{}
            , strf::alignment_format{U' ', width, strf::text_alignment::center} };
    }
    template <typename CharT>
    constexpr STRF_HD adapted_derived_type fill(CharT ch) const noexcept
    {
        static_assert( strf::is_char<CharT>::value // issue #19
                     , "Refusing non-char argument to set the fill character, "
                       "since one may pass 0 instead of '0' by accident." );
        return adapted_derived_type
            { self_downcast_()
            , strf::tag<alignment_formatter_q<true>>{}
            , strf::alignment_format{static_cast<char32_t>(ch)} };
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD
    T&& set_alignment_format(strf::default_alignment_format) && noexcept
    {
        return move_self_downcast_();
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&  set_alignment_format(strf::default_alignment_format) & noexcept
    {
        return self_downcast_();
    }
    STRF_CONSTEXPR_IN_CXX14 STRF_HD const T& set_alignment_format(strf::default_alignment_format) const & noexcept
    {
        return self_downcast_();
    }
    constexpr STRF_HD adapted_derived_type set_alignment_format(strf::alignment_format data) const & noexcept
    {
        return adapted_derived_type
            { self_downcast_()
            , strf::tag<strf::alignment_formatter_q<true>>{}
            , data };
    }
    constexpr static STRF_HD strf::default_alignment_format get_alignment_format() noexcept
    {
        return {};
    }
    constexpr STRF_HD strf::width_t width() const noexcept
    {
        return 0;//get_alignment_format().width;
    }
    constexpr STRF_HD strf::text_alignment alignment() const noexcept
    {
        return get_alignment_format().alignment;
    }
    constexpr STRF_HD char32_t fill() const noexcept
    {
        return get_alignment_format().fill;
    }

private:

    STRF_HD constexpr const T& self_downcast_() const
    {
        //const T* base_ptr = static_cast<const T*>(this);
        return *static_cast<const T*>(this);
    }
    STRF_HD STRF_CONSTEXPR_IN_CXX14 T& self_downcast_()
    {
        T* base_ptr = static_cast<T*>(this);
        return *base_ptr;
    }
    STRF_HD STRF_CONSTEXPR_IN_CXX14 T&& move_self_downcast_()
    {
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
};

template <bool HasAlignment>
struct alignment_formatter_q
{
    template <class T>
    using fn = strf::alignment_formatter_fn<T, HasAlignment>;
};

using dynamic_alignment_formatter = strf::alignment_formatter_q<true>;
using alignment_formatter = strf::alignment_formatter_q<false>;


template <class T>
class quantity_formatter_fn
{
public:

    constexpr STRF_HD quantity_formatter_fn(std::size_t count) noexcept
        : count_(count)
    {
    }

    constexpr STRF_HD quantity_formatter_fn() noexcept
    {
    }

    template <typename U>
    constexpr STRF_HD explicit quantity_formatter_fn(const quantity_formatter_fn<U>& u) noexcept
        : count_(u.count())
    {
    }

    STRF_CONSTEXPR_IN_CXX14 STRF_HD T&& multi(std::size_t count) && noexcept
    {
        count_ = count;
        T* base_ptr = static_cast<T*>(this);
        return static_cast<T&&>(*base_ptr);
    }
    constexpr STRF_HD std::size_t count() const noexcept
    {
        return count_;
    }

private:

    std::size_t count_ = 1;
};

struct quantity_formatter
{
    template <class T>
    using fn = strf::quantity_formatter_fn<T>;
};


inline namespace format_functions {

#if defined (STRF_NO_GLOBAL_CONSTEXPR_VARIABLE)

template <typename T>
constexpr STRF_HD auto hex(T&& value)
    noexcept(noexcept(strf::fmt(value).hex()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).hex())>
{
    return strf::fmt(value).hex();
}

template <typename T>
constexpr STRF_HD auto dec(T&& value)
    noexcept(noexcept(strf::fmt(value).dec()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).dec())>
{
    return strf::fmt(value).dec();
}

template <typename T>
constexpr STRF_HD auto oct(T&& value)
    noexcept(noexcept(strf::fmt(value).oct()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).oct())>
{
    return strf::fmt(value).oct();
}

template <typename T>
constexpr STRF_HD auto bin(T&& value)
    noexcept(noexcept(strf::fmt(value).bin()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).bin())>
{
    return strf::fmt(value).bin();
}

template <typename T>
constexpr STRF_HD auto fixed(T&& value)
    noexcept(noexcept(strf::fmt(value).fixed()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fixed())>
{
    return strf::fmt(value).fixed();
}

template <typename T>
    constexpr STRF_HD auto fixed(T&& value, unsigned precision)
    noexcept(noexcept(strf::fmt(value).fixed().p(precision)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fixed().p(precision))>
{
    return strf::fmt(value).fixed().p(precision);
}

template <typename T>
constexpr STRF_HD auto sci(T&& value)
    noexcept(noexcept(strf::fmt(value).sci()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sci())>
{
    return strf::fmt(value).sci();
}

template <typename T>
constexpr STRF_HD auto sci(T&& value, unsigned precision)
    noexcept(noexcept(strf::fmt(value).sci().p(precision)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sci().p(precision))>
{
    return strf::fmt(value).sci().p(precision);
}

template <typename T>
constexpr STRF_HD auto gen(T&& value)
    noexcept(noexcept(strf::fmt(value).gen()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).gen())>
{
    return strf::fmt(value).gen();
}

template <typename T>
constexpr STRF_HD auto gen(T&& value, unsigned precision)
    noexcept(noexcept(strf::fmt(value).gen().p(precision)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).gen().p(precision))>
{
    return strf::fmt(value).gen().p(precision);
}

template <typename T, typename C>
constexpr STRF_HD auto multi(T&& value, C&& count)
    noexcept(noexcept(strf::fmt(value).multi(count)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).multi(count))>
{
    return strf::fmt(value).multi(count);
}

template <typename T>
constexpr STRF_HD auto conv(T&& value)
    noexcept(noexcept(strf::fmt(value).convert_charset()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).convert_charset())>
{
    return strf::fmt(value).convert_charset();
}

template <typename T, typename Charset>
    constexpr STRF_HD auto conv(T&& value, Charset&& charset)
    noexcept(noexcept(strf::fmt(value).convert_from_charset(charset)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).convert_from_charset(charset))>
{
    return strf::fmt(value).convert_from_charset(charset);
}

template <typename T>
constexpr STRF_HD auto sani(T&& value)
    noexcept(noexcept(strf::fmt(value).sanitize_charset()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sanitize_charset())>
{
    return strf::fmt(value).sanitize_charset();
}

template <typename T, typename Charset>
    constexpr STRF_HD auto sani(T&& value, Charset&& charset)
    noexcept(noexcept(strf::fmt(value).sanitize_from_charset(charset)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sanitize_from_charset(charset))>
{
    return strf::fmt(value).sanitize_from_charset(charset);
}

template <typename T>
constexpr STRF_HD auto right(T&& value, strf::width_t width)
    noexcept(noexcept(strf::fmt(value) > width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value) > width)>
{
    return strf::fmt(value) > width;
}

template <typename T, typename CharT>
constexpr STRF_HD auto right(T&& value, strf::width_t width, CharT fill)
    noexcept(noexcept(strf::fmt(value).fill(fill) > width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) > width)>
{
    return strf::fmt(value).fill(fill) > width;
}

template <typename T>
constexpr STRF_HD auto left(T&& value, strf::width_t width)
    noexcept(noexcept(strf::fmt(value) < width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value) < width)>
{
    return strf::fmt(value) < width;
}

template <typename T, typename CharT>
constexpr STRF_HD auto left(T&& value, strf::width_t width, CharT fill)
    noexcept(noexcept(strf::fmt(value).fill(fill) < width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) < width)>
{
    return strf::fmt(value).fill(fill) < width;
}

template <typename T>
constexpr STRF_HD auto center(T&& value, strf::width_t width)
    noexcept(noexcept(strf::fmt(value) ^ width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value) ^ width)>
{
    return strf::fmt(value) ^ width;
}

template <typename T, typename CharT>
constexpr STRF_HD auto center(T&& value, strf::width_t width, CharT fill)
    noexcept(noexcept(strf::fmt(value).fill(fill) ^ width))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) ^ width)>
{
    return strf::fmt(value).fill(fill) ^ width;
}

template <typename T>
constexpr STRF_HD auto pad0(T&& value, decltype(strf::fmt(value).pad0width()) width)
    noexcept(noexcept(strf::fmt(value).pad0(width)))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).pad0(width))>
{
    return strf::fmt(value).pad0(width);
}

template <typename T>
constexpr STRF_HD auto punct(T&& value)
    noexcept(noexcept(strf::fmt(value).punct()))
    -> strf::detail::remove_reference_t<decltype(strf::fmt(value).punct())>
{
    return strf::fmt(value).punct();
}

#else  // defined (STRF_NO_GLOBAL_CONSTEXPR_VARIABLE)

namespace detail_format_functions {

struct hex_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).hex()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).hex())>
    {
        return strf::fmt(value).hex();
    }
};

struct dec_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).dec()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).dec())>
    {
        return strf::fmt(value).dec();
    }
};

struct oct_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).oct()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).oct())>
    {
        return strf::fmt(value).oct();
    }
};

struct bin_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).bin()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).bin())>
    {
        return strf::fmt(value).bin();
    }
};

struct fixed_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).fixed()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fixed())>
    {
        return strf::fmt(value).fixed();
    }
    template <typename T>
        constexpr STRF_HD auto operator()(T&& value, unsigned precision) const
        noexcept(noexcept(strf::fmt(value).fixed().p(precision)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fixed().p(precision))>
    {
        return strf::fmt(value).fixed().p(precision);
    }
};

struct sci_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).sci()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sci())>
    {
        return strf::fmt(value).sci();
    }
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value, unsigned precision) const
        noexcept(noexcept(strf::fmt(value).sci().p(precision)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sci().p(precision))>
    {
        return strf::fmt(value).sci().p(precision);
    }
};

struct gen_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).gen()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).gen())>
    {
        return strf::fmt(value).gen();
    }
    template <typename T>
        constexpr STRF_HD auto operator()(T&& value, unsigned precision) const
        noexcept(noexcept(strf::fmt(value).gen().p(precision)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).gen().p(precision))>
    {
        return strf::fmt(value).gen().p(precision);
    }
};

struct multi_fn {
    template <typename T, typename C>
    constexpr STRF_HD auto operator()(T&& value, C&& count) const
        noexcept(noexcept(strf::fmt(value).multi(count)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).multi(count))>
    {
        return strf::fmt(value).multi(count);
    }
};

struct conv_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).convert_charset()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).convert_charset())>
    {
        return strf::fmt(value).convert_charset();
    }
    template <typename T, typename Charset>
        constexpr STRF_HD auto operator()(T&& value, Charset&& charset) const
        noexcept(noexcept(strf::fmt(value).convert_from_charset(charset)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).convert_from_charset(charset))>
    {
        return strf::fmt(value).convert_from_charset(charset);
    }
};

struct sani_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value) const
        noexcept(noexcept(strf::fmt(value).sanitize_charset()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sanitize_charset())>
    {
        return strf::fmt(value).sanitize_charset();
    }
    template <typename T, typename Charset>
    constexpr STRF_HD auto operator()(T&& value, Charset&& charset) const
        noexcept(noexcept(strf::fmt(value).sanitize_from_charset(charset)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).sanitize_from_charset(charset))>
    {
        return strf::fmt(value).sanitize_from_charset(charset);
    }
};

struct right_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width) const
        noexcept(noexcept(strf::fmt(value) > width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value) > width)>
    {
        return strf::fmt(value) > width;
    }
    template <typename T, typename CharT>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width, CharT fill) const
        noexcept(noexcept(strf::fmt(value).fill(fill) > width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) > width)>
    {
        return strf::fmt(value).fill(fill) > width;
    }
};

struct left_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width) const
        noexcept(noexcept(strf::fmt(value) < width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value) < width)>
    {
        return strf::fmt(value) < width;
    }
    template <typename T, typename CharT>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width, CharT fill) const
        noexcept(noexcept(strf::fmt(value).fill(fill) < width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) < width)>
    {
        return strf::fmt(value).fill(fill) < width;
    }
};

struct center_fn {
    template <typename T>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width) const
        noexcept(noexcept(strf::fmt(value) ^ width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value) ^ width)>
    {
        return strf::fmt(value) ^ width;
    }
    template <typename T, typename CharT>
    constexpr STRF_HD auto operator()(T&& value, strf::width_t width, CharT fill) const
        noexcept(noexcept(strf::fmt(value).fill(fill) ^ width))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).fill(fill) ^ width)>
    {
        return strf::fmt(value).fill(fill) ^ width;
    }
};

struct pad0_fn {
    template <typename T, typename W>
    constexpr STRF_HD auto operator() (T&& value, W width) const
        noexcept(noexcept(strf::fmt(value).pad0(width)))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).pad0(width))>
    {
        return strf::fmt(value).pad0(width);
    }
};

struct punct_fn {
    template <typename T>
    constexpr STRF_HD auto operator()
        ( T&& value ) const
        noexcept(noexcept(strf::fmt(value).punct()))
        -> strf::detail::remove_reference_t<decltype(strf::fmt(value).punct())>
    {
        return strf::fmt(value).punct();
    }
};

} // namespace detail_format_functions

constexpr strf::detail_format_functions::hex_fn    hex {};
constexpr strf::detail_format_functions::dec_fn    dec {};
constexpr strf::detail_format_functions::oct_fn    oct {};
constexpr strf::detail_format_functions::bin_fn    bin {};
constexpr strf::detail_format_functions::fixed_fn  fixed {};
constexpr strf::detail_format_functions::sci_fn    sci {};
constexpr strf::detail_format_functions::gen_fn    gen {};
constexpr strf::detail_format_functions::multi_fn  multi {};
constexpr strf::detail_format_functions::conv_fn   conv {};
constexpr strf::detail_format_functions::sani_fn   sani {};
constexpr strf::detail_format_functions::right_fn  right {};
constexpr strf::detail_format_functions::left_fn   left {};
constexpr strf::detail_format_functions::center_fn center {};
constexpr strf::detail_format_functions::pad0_fn   pad0 {};
constexpr strf::detail_format_functions::punct_fn  punct {};

#endif // defined (STRF_NO_GLOBAL_CONSTEXPR_VARIABLE)

} // inline namespace format_functions

} // namespace strf

#endif // STRF_PRINTER_HPP
