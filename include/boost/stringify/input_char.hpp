#ifndef BOOST_STRINGIFY_INPUT_CHAR_HPP_INCLUDED
#define BOOST_STRINGIFY_INPUT_CHAR_HPP_INCLUDED

//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/stringify/input_char32.hpp>
#include <boost/stringify/type_traits.hpp>
#include <boost/stringify/custom_width_calculator.hpp>

namespace boost {
namespace stringify {
namespace detail {

template <typename CharT, typename Output, typename FTuple>
class char_stringifier
{

public:

    using input_type = CharT ;
    using char_type = CharT;
    using output_type = Output ;
    using ftuple_type = FTuple ;
    
    char_stringifier(const FTuple& fmt, CharT _character) noexcept
        : m_fmt(fmt)
        , m_char(_character)
    {
    }

    virtual std::size_t length() const
    {
        return 1;
    }
    
    void write(Output& out) const
    {
        out.put(m_char);
    }

    int remaining_width(int w) const
    {
        auto calc = boost::stringify::get_width_calculator<input_type>(m_fmt);
        return w - calc.width_of(m_char);
    }

    
private:
   
    const FTuple& m_fmt; 
    CharT m_char;
};

template <typename CharIn>
struct char_input_traits
{
    
private:

    template <typename CharOut>
    struct helper
    {
        static_assert(sizeof(CharIn) == sizeof(CharOut), "");
        
        template <typename Output, typename FTuple>
        using stringifier
        = boost::stringify::detail::char_stringifier
            <CharOut, Output, FTuple>;
    };
    
public:
    
    template <typename CharT, typename Output, typename FTuple>
    using stringifier
    = typename helper<CharT>::template stringifier<Output, FTuple>;
};

} //namepace detail


boost::stringify::detail::char_input_traits<char>
boost_stringify_input_traits_of(char);

boost::stringify::detail::char_input_traits<char16_t>
boost_stringify_input_traits_of(char16_t);

boost::stringify::detail::char_input_traits<wchar_t>
boost_stringify_input_traits_of(wchar_t);


} // namespace stringify
} // namespace boost

#endif



