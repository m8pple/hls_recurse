#ifndef hls_tuple_hpp
#define hls_tuple_hpp

#include "hls_recurse/utility.hpp"

#ifndef __SYNTHESIS__
#include <iostream>
#endif

namespace hls_recurse
{

struct hls_make_tag {};

template<class ...TParts>
struct hls_state_tuple;

template<class ...TParts>
struct hls_binding_tuple;


namespace detail
{

    template<class T>
    struct is_state_tuple
    { static const int value = 0; };

    template<class ...T>
    struct is_state_tuple<hls_state_tuple<T...>>
    { static const int value = 1; };


    template<class T>
    struct is_binding_tuple
    { static const int value = 0; };

    template<class ...T>
    struct is_binding_tuple<hls_binding_tuple<T...>>
    { static const int value = 1; };
};


template<>
struct hls_state_tuple<>
{
public:
    HLS_INLINE_STEP hls_state_tuple() = default;

    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<> &)
    {}

#ifndef __SYNTHESIS__
    void dump(std::ostream &dst) const
    {}
#endif
};


template<class TFirst, class ...TRest>
struct hls_state_tuple<TFirst,TRest...>
{
    static_assert(!detail::is_state_tuple<TFirst>::value, "A state tuple cannot contain another state tuple.");
    static_assert(!detail::is_binding_tuple<TFirst>::value, "A state tuple cannot contain a binding tuple.");
    static_assert(!std::is_reference<TFirst>::value, "A state tuple cannot contain a reference.");

public:
    TFirst first;
    hls_state_tuple<TRest...> rest;


    // Don't really need the explicit default construction, just making
    // clear that is what happens
    HLS_INLINE_STEP hls_state_tuple() = default;

    HLS_INLINE_STEP hls_state_tuple(const hls_state_tuple &) = default;

    // Construction from binding tuple
    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<TFirst,TRest...> &x)
        : first(x.first)
        , rest(x.rest)
    {}


    //! Used for exact construction by make_hls_state_tuple
    HLS_INLINE_STEP hls_state_tuple(hls_make_tag, const TFirst &_first, const hls_state_tuple<TRest...> &_rest)
        : first(_first)
        , rest(_rest)
    {}

    //! Create a tuple from a parameter pack
    HLS_INLINE_STEP hls_state_tuple(const TFirst &_first, const TRest &..._rest)
        : first(_first)
        , rest(_rest...)
    {}




    //! Create a tuple from head and tail, then default all the remaining parameters
    HLS_INLINE_STEP hls_state_tuple(const TFirst &_first, const hls_state_tuple<> &_rest)
        : first(_first)
        , rest(_rest)
    {}



    // Allow for slight mismatches, as long as they are copy constructible
    template<class TFirstAlt, class TRestHead, class ...TRestAlt>
    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<TFirstAlt,TRestHead,TRestAlt...> &x)
        : first(x.first)
        , rest(x.rest)
    {}

    // Allow for missing parts, and rely on default construction
    template<class TFirstAlt>
    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<TFirstAlt> &x)
        : first(x.first)
        , rest()
    {}

    // Complete default destruction from empty tuple
    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<> &x)
        : first()
        , rest()
    {}

    HLS_INLINE_STEP void operator=(const hls_binding_tuple<TFirst,TRest...> &src)
    {
        first=src.first;
        rest=src.rest;
    }

    // Allow for slight mismatches, as long as they are assignable
    template<class TFirstAlt, class TRestHead, class ...TRestAlt>
    HLS_INLINE_STEP void operator=(const hls_binding_tuple<TFirstAlt,TRestHead,TRestAlt...> &src)
    {
        first=src.first;
        rest=src.rest;
    }

    // Allow for missing remainder
    template<class TFirstAlt>
    HLS_INLINE_STEP void operator=(const hls_binding_tuple<TFirstAlt> &src)
    {
        first=src.first;
        rest=hls_state_tuple<TRest...>(); // Default constructor
    }

    // Allow for missing everything
    HLS_INLINE_STEP void operator=(const hls_binding_tuple<> &src)
    {
        first=TFirst();
        rest=hls_state_tuple<TRest...>(); // Default constructor
    }

#ifndef __SYNTHESIS__
    void dump(std::ostream &dst) const
    {
        dst<<first;
        dst<<",";
        rest.dump(dst);
    }
#endif
};

template<class ...TArgs>
HLS_INLINE_STEP hls_state_tuple<TArgs...> make_hls_state_tuple(const TArgs &...args);

template<class TFirst, class ...TRest>
HLS_INLINE_STEP hls_state_tuple<TFirst,TRest...> make_hls_state_tuple(const TFirst &first, const TRest &...rest)
{
    return hls_state_tuple<TFirst,TRest...>(hls_make_tag(), first, make_hls_state_tuple(rest...));
}

template<>
HLS_INLINE_STEP hls_state_tuple<> make_hls_state_tuple<>()
{
    return hls_state_tuple<>();
}

namespace detail
{

    template<unsigned i,class TFirst, class ...TRest>
    class getter
    {
    public:
      typedef typename getter<i-1,TRest...>::result_t result_t;

      static const result_t &get(const hls_state_tuple<TFirst,TRest...> &args)
      {
        return getter<i-1,TRest...>::get(args.rest);
      }

      static result_t &get(hls_state_tuple<TFirst,TRest...> &args)
      {
        return getter<i-1,TRest...>::get(args.rest);
      }
    };

    template<class TFirst, class ...TRest>
    class getter<0,TFirst,TRest...>
    {
    public:
      typedef TFirst result_t;

      static const result_t &get(const hls_state_tuple<TFirst,TRest...> &args)
      {
        return args.first;
      }

      static result_t &get(hls_state_tuple<TFirst,TRest...> &args)
      {
        return args.first;
      }
    };

};

template<unsigned i,class ...TArgs>
const typename detail::getter<i,TArgs...>::result_t &get(const hls_state_tuple<TArgs...> &args)
{
  return detail::getter<i,TArgs...>::get(args);
}

template<unsigned i,class ...TArgs>
typename detail::getter<i,TArgs...>::result_t &get(hls_state_tuple<TArgs...> &args)
{
  return detail::getter<i,TArgs...>::get(args);
}


template<>
struct hls_binding_tuple<>
{
public:
    HLS_INLINE_STEP hls_binding_tuple()
    {}

    HLS_INLINE_STEP void operator=(const hls_state_tuple<> &)
    {}

#ifndef __SYNTHESIS__
    void dump(std::ostream &dst) const
    {}
#endif
};

template<class TFirst, class ...TRest>
struct hls_binding_tuple<TFirst,TRest...>
{
public:
    TFirst &first;
    hls_binding_tuple<TRest...> rest;


    HLS_INLINE_STEP hls_binding_tuple(TFirst &_first, TRest &..._rest)
        : first(_first)
        , rest(_rest...)
    {}

    HLS_INLINE_STEP hls_binding_tuple(TFirst &_first, const hls_binding_tuple<TRest...> &_rest)
        : first(_first)
        , rest(_rest)
    {}

    HLS_INLINE_STEP hls_binding_tuple(hls_make_tag, TFirst &_first, const hls_binding_tuple<TRest...> &_rest)
        : first(_first)
        , rest(_rest)
    {}

    HLS_INLINE_STEP void operator=(const hls_state_tuple<TFirst,TRest...> &src)
    {
        first=src.first;
        rest=src.rest;
    }

    // Allow for slight mismatches, as long as they are convertible
    template<class TFirstAlt, class TRestHead, class ...TRestAlt>
    HLS_INLINE_STEP void operator=(const hls_state_tuple<TFirstAlt,TRestHead,TRestAlt...> &src)
    {
        first=src.first;
        rest=src.rest;
    }

    template<class TFirstAlt>
    HLS_INLINE_STEP void operator=(const hls_state_tuple<TFirstAlt> &src)
    {
        first=src.first;
        rest=hls_state_tuple<TRest...>();
    }

    HLS_INLINE_STEP void operator=(const hls_state_tuple<> &src)
    {
        first=TFirst();
        rest=hls_state_tuple<TRest...>();
    }

#ifndef __SYNTHESIS__
    void dump(std::ostream &dst) const
    {
        dst<<first<<",";
        rest.dump(dst);
    }
#endif
};


template<class TFirst,class TSecond>
struct hls_binding_tuple_cat_type;

template<class TFirst,class ...TSecondParts>
struct hls_binding_tuple_cat_type<TFirst,hls_binding_tuple<TSecondParts...> >
{
    typedef hls_binding_tuple<TFirst,TSecondParts...> type;
};


HLS_INLINE_STEP hls_binding_tuple<> make_hls_binding_tuple()
{
    return hls_binding_tuple<>();
}


template<class TFirst, class ...TRest>
HLS_INLINE_STEP hls_binding_tuple<TFirst,TRest...> make_hls_binding_tuple(TFirst &first, TRest &...rest)
{
    hls_make_tag tag;
    return hls_binding_tuple<TFirst,TRest...>(tag, first, make_hls_binding_tuple(rest...));
}

template<class TFirst, class ...TRest>
HLS_INLINE_STEP hls_binding_tuple<TFirst,TRest...> make_hls_binding_tuple(TFirst &first, const hls_binding_tuple<TRest...> &_rest)
{
    return hls_binding_tuple<TFirst,TRest...>(first, _rest);
}

template<class ...TB>
HLS_INLINE_STEP hls_binding_tuple<TB...> concatenate_binding_tuples(const hls_binding_tuple<> &a, const hls_binding_tuple<TB...> &b)
{
    return b;
}

template<class ...TA, class ...TB>
HLS_INLINE_STEP hls_binding_tuple<TA...,TB...> concatenate_binding_tuples(const hls_binding_tuple<TA...> &a, const hls_binding_tuple<TB...> &b)
{
    return make_hls_binding_tuple(a.first,concatenate_binding_tuples(a.rest, b));
}


}; // HLSrec

#endif
