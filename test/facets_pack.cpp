//  Distributed under the Boost Software License, Version 1.0.
//  (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include "test_utils.hpp"

template <int N> struct fcategory;

struct ctor_log
{
    int cp_count = 0;
    int mv_count = 0;
};

enum facet_conf
{
    enable_copy_and_move  ,
    enable_copy           ,
    enable_only_move      ,
    disable_copy_and_move ,
};

class facet_base
{
public:

    constexpr STRF_HD facet_base(int v, ctor_log* log = nullptr)
        : value(v)
        , log_(log)
    {
    }

    constexpr STRF_HD facet_base(const facet_base& f)
        : value(f.value)
        , log_(f.log_)
    {
        if(log_)
        {
            ++ log_->cp_count;
        }
    }

    constexpr STRF_HD facet_base(facet_base&& f)
        : value(f.value)
        , log_(f.log_)
    {
        if(log_)
        {
            ++ log_->mv_count;
        }
    }

    int value;

private:

    ctor_log* log_;
};

template <int N, facet_conf Conf = facet_conf::enable_copy_and_move>
struct facet;

template <int N>
struct facet<N, facet_conf::enable_copy_and_move> : public facet_base
{
    constexpr STRF_HD facet(int v, ctor_log* log = nullptr)
        : facet_base(v, log)
    {
    }
    constexpr facet(const facet& f) = default;
    constexpr facet(facet&& f) = default;

    using category = fcategory<N>;
};

template <int N>
struct facet<N, facet_conf::enable_copy> : public facet_base
{
    constexpr STRF_HD facet(int v, ctor_log* log = nullptr)
        : facet_base(v, log)
    {
    }
    constexpr facet(const facet& f) = default;

    using category = fcategory<N>;
};

template <int N>
struct facet<N, facet_conf::enable_only_move> : public facet_base
{
    constexpr STRF_HD facet(int v, ctor_log* log = nullptr)
        : facet_base(v, log)
    {
    }
    constexpr facet(const facet& f) = delete;
    constexpr facet(facet&& f) = default;

    using category = fcategory<N>;
};

template <int N>
struct facet<N, facet_conf::disable_copy_and_move> : public facet_base
{
    constexpr STRF_HD facet(int v, ctor_log* log = nullptr)
        : facet_base(v, log)
    {
    }
    constexpr facet(const facet& f) = delete;
    constexpr facet(facet&& f) = delete;

    using category = fcategory<N>;
};

template <int N> struct fcategory
{
    constexpr static bool constrainable = true;

    constexpr static STRF_HD facet<N> get_default() noexcept
    {
        return facet<N>{-1};
    }
};


class class_x{};

class class_xa: public class_x {};
class class_xb: public class_x {public: int i;};
class class_c {};

template <typename T>
using derives_from_x = std::is_base_of<class_x, T>;

template <typename T>
using is_64 = std::integral_constant<bool, sizeof(T) == 8>;

static void STRF_TEST_FUNC basic_tests()
{
    auto f1_10 = facet<1>{10};
    auto f2_20 = facet<2>{20};
    auto f2_21 = facet<2>{21};
    auto f2_22 = facet<2>{22};
    auto f3_30 = facet<3>{30};
    struct dummy_type{};

    {
        auto fp = strf::pack(
            f1_10,
            f2_20,
            f2_21,
            strf::constrain<std::is_integral>(f2_22),
            f3_30
        );

        decltype(auto) f1i = strf::get_facet<fcategory<1>, int>(fp);
        decltype(auto) f2d = strf::get_facet<fcategory<2>, double>(fp);
        decltype(auto) f2i = strf::get_facet<fcategory<2>, int>(fp);
        decltype(auto) f3i = strf::get_facet<fcategory<3>, int>(fp);

        static_assert(std::is_same<decltype(f1i), const facet<1>&>::value, "wrong type");
        static_assert(std::is_same<decltype(f2d), const facet<2>&>::value, "wrong type");
        static_assert(std::is_same<decltype(f2i), const facet<2>&>::value, "wrong type");
        static_assert(std::is_same<decltype(f3i), const facet<3>&>::value, "wrong type");

        TEST_EQ(f1i.value, 10);
        TEST_EQ(f2d.value, 21);
        TEST_EQ(f2i.value, 22);
        TEST_EQ(f3i.value, 30);
    }

    {   // constrain<Filter1>(constrain<Filter2>(facet))

        auto f2_20_empty = strf::constrain<std::is_empty>(f2_20);
        auto f2_20_empty_and_derives_from_x
            = strf::constrain<derives_from_x>(f2_20_empty);

        auto fp = strf::pack
            (
                strf::constrain<std::is_empty>(f2_21),
                strf::constrain<derives_from_x>(f2_22),
                f2_20_empty_and_derives_from_x
            );

        decltype(auto) xf2_20 = strf::get_facet<fcategory<2>, class_xa>(fp);
        decltype(auto) xf2_22 = strf::get_facet<fcategory<2>, class_xb>(fp);
        decltype(auto) xf2_21 = strf::get_facet<fcategory<2>, class_c>(fp);
        static_assert(std::is_same<decltype(xf2_20), const facet<2>&>::value, "wrong type");
        static_assert(std::is_same<decltype(xf2_21), const facet<2>&>::value, "wrong type");
        static_assert(std::is_same<decltype(xf2_22), const facet<2>&>::value, "wrong type");
        TEST_EQ(xf2_20.value, 20);
        TEST_EQ(xf2_21.value, 21);
        TEST_EQ(xf2_22.value, 22);
    }
}


static void STRF_TEST_FUNC test_constrained_fpe()
{
    { // check constexpr

        constexpr facet<0> f {10};
        constexpr auto c = strf::constrain<is_64>(f);
        constexpr auto c2 = c;
        constexpr auto c3 = std::move(c2);

        constexpr auto fp = pack(c3);
        constexpr decltype(fp) fp2{fp};
        constexpr decltype(fp) fp3{std::move(fp2)};

        static_assert(strf::get_facet<fcategory<0>, double>(fp3).value == 10, " ");
        static_assert(strf::get_facet<fcategory<0>, float>(fp3).value == -1, " ");

    }
    {   // constrain a facet copy
        ctor_log log;
        facet<0> f{10, &log};
        auto c = strf::constrain<is_64>(f);
        auto c2 = strf::constrain<std::is_integral>(c);
        auto c3 = strf::constrain<std::is_signed>(c2);
        auto fp = pack(c3);
        decltype(fp) fp2{fp};
        const auto fp3 = pack(fp2);

        TEST_EQ(log.cp_count, 6);
        TEST_EQ(log.mv_count, 0);
        TEST_EQ(10, (strf::get_facet<fcategory<0>, std::int64_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::int32_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::uint64_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, double>(fp3).value));
    }
    {   // construct the constrained facet from rvalue reference;

        ctor_log log;
        facet<0, facet_conf::enable_only_move> f{10, &log};

        auto c = strf::constrain<is_64>(std::move(f));
        TEST_EQ(log.cp_count, 0);

        auto c2 = strf::constrain<std::is_integral>(std::move(c));
        TEST_EQ(log.cp_count, 0);

        auto fp = pack(std::move(c2));
        TEST_EQ(log.cp_count, 0);

        auto c3 = strf::constrain<std::is_signed>(std::move(fp));
        TEST_EQ(log.cp_count, 0);

        auto fp2 = pack(std::move(c3));
        TEST_EQ(log.cp_count, 0);

        auto fp3 = pack(std::move(fp2));
        TEST_EQ(log.cp_count, 0);

        TEST_EQ(10, (strf::get_facet<fcategory<0>, std::int64_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::int32_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, double>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::uint64_t>(fp3).value));
    }

    {   // construct the constrained facet from rvalue reference;
        // when move constructor is deleted

        ctor_log log;
        facet<0, enable_copy> f{10, &log};

        auto c = strf::constrain<is_64>(std::move(f));
        auto c2 = strf::constrain<std::is_integral>(std::move(c));
        auto fp = pack(std::move(c2));
        auto c3 = strf::constrain<std::is_signed>(std::move(fp));
        auto fp2 = pack(std::move(c3));
        auto fp3 = pack(std::move(fp2));
        TEST_EQ(log.cp_count, 6);

        TEST_EQ(10, (strf::get_facet<fcategory<0>, std::int64_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::int32_t>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, double>(fp3).value));
        TEST_EQ(-1, (strf::get_facet<fcategory<0>, std::uint64_t>(fp3).value));
    }
    {   // constrain a facets_pack

        auto fp = strf::pack
            ( strf::constrain<std::is_signed>(facet<1>(201))
            , facet<2>(202)
            , facet<3>(203) );
        auto fp2 = strf::pack
            ( facet<1>(101)
            , strf::constrain<is_64>(fp)
            , strf::constrain<std::is_integral>(facet<1>(301)) );


        TEST_EQ(101, (strf::get_facet<fcategory<1>, float>(fp2).value));
        TEST_EQ(201, (strf::get_facet<fcategory<1>, double>(fp2).value));
        TEST_EQ(301, (strf::get_facet<fcategory<1>, int>(fp2).value));
    }

    {
        TEST_TRUE(!(std::is_copy_constructible<facet<0, enable_only_move>>::value));
        TEST_TRUE(!(std::is_move_constructible<facet<0, disable_copy_and_move>>::value));

        TEST_TRUE((std::is_copy_constructible<facet<0, enable_copy>>::value));
        TEST_TRUE((std::is_move_constructible<facet<0, enable_copy>>::value));
        TEST_TRUE((std::is_copy_constructible<facet<0, enable_copy_and_move>>::value));
        TEST_TRUE((std::is_move_constructible<facet<0, enable_copy_and_move>>::value));
    }
}

inline void STRF_TEST_FUNC compilation_tests()
{
    bool test1 = ! std::is_copy_constructible
        <strf::constrained_fpe<is_64, facet<0, enable_only_move>>>
        ::value;

    bool test2 = std::is_copy_constructible
        <strf::constrained_fpe<is_64, const facet<0, enable_only_move>& >>
        ::value;

    bool test3 = ! std::is_copy_constructible
        <strf::facets_pack<facet<0, enable_only_move>>>
        ::value;

    bool test4 = std::is_copy_constructible
        <strf::facets_pack<const facet<0, enable_only_move>& >>
        ::value;

    bool test5 =  ! std::is_copy_constructible
        < strf::facets_pack
             < strf::constrained_fpe
                 < is_64, facet<0, enable_only_move> >>>
        :: value;

    bool test6 = std::is_constructible
        < strf::facets_pack<facet<0, disable_copy_and_move>>, int >
        ::value;

    bool test7 = ! std::is_move_constructible
        < strf::facets_pack<facet<0, disable_copy_and_move>> >
        ::value;

    bool test8 = std::is_same
        < strf::facets_pack<facet<0>>
        , decltype(strf::pack(facet<0>{0})) >
        ::value;

    TEST_TRUE(test1);
    TEST_TRUE(test2);
    TEST_TRUE(test3);
    TEST_TRUE(test4);
    TEST_TRUE(test5);
    TEST_TRUE(test6);
    TEST_TRUE(test7);
    TEST_TRUE(test8);
}

void STRF_TEST_FUNC test_facets_pack()
{
    basic_tests();
    test_constrained_fpe();
    compilation_tests();
}
