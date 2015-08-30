#ifndef hls_tuple_hpp
#define hls_tuple_hpp


#include <iostream>

/*
    This file should not exist. It works fine with std::tuple for
    both values and references, but the frontends are too far
    downlevel to include tuple.
    UPDATE: now both front-ends can deal with tuple, but this is
    a legacy thing. It still allows certain things like assigning
    from a lower to higher arity tuple.

*/

struct hls_make_tag {};

template<class ...TParts>
struct hls_state_tuple;

template<class ...TParts>
struct hls_binding_tuple;


template<>
struct hls_state_tuple<>
{
public:
    HLS_INLINE_STEP hls_state_tuple()
    {}

    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<> &)
    {}

    std::ostream &dump(std::ostream &dst)
    {
        return dst;
    }
};

template<class TFirst, class ...TRest>
struct hls_state_tuple<TFirst,TRest...>
{
public:
    TFirst first;
    hls_state_tuple<TRest...> rest;


    // Don't really need the explicit default construction, just making
    // clear that is what happens
    HLS_INLINE_STEP hls_state_tuple()
        : first() // Default construct
        , rest()  // Default construct
    {}

    HLS_INLINE_STEP hls_state_tuple(const TFirst &_first, const TRest &..._rest)
        : first(_first)
        , rest(_rest...)
    {}
        
    HLS_INLINE_STEP hls_state_tuple(const TFirst &_first, const hls_state_tuple<> &_rest)
        : first(_first)
        , rest(_rest)
    {}

    HLS_INLINE_STEP hls_state_tuple(hls_make_tag, const TFirst &_first, const hls_state_tuple<TRest...> &_rest)
        : first(_first)
        , rest(_rest)
    {}

    HLS_INLINE_STEP hls_state_tuple(const hls_binding_tuple<TFirst,TRest...> &x)
        : first(x.first)
        , rest(x.rest)
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
    
    std::ostream &dump(std::ostream &dst)
    {
        dst<<first<<",";
        return rest.dump(dst);
    }
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

template<class TFirst,class TSecond>
struct hls_state_tuple_cat_type;

template<class TFirst,class ...TSecondParts>
struct hls_state_tuple_cat_type<TFirst,hls_state_tuple<TSecondParts...> >
{
    typedef hls_state_tuple<TFirst,TSecondParts...> type;
};


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

template<unsigned i,class ...TArgs>
const typename getter<i,TArgs...>::result_t &get(const hls_state_tuple<TArgs...> &args)
{
  return getter<i,TArgs...>::get(args);
}

template<unsigned i,class ...TArgs>
typename getter<i,TArgs...>::result_t &get(hls_state_tuple<TArgs...> &args)
{
  return getter<i,TArgs...>::get(args);
}


template<>
struct hls_binding_tuple<>
{
public:
    HLS_INLINE_STEP hls_binding_tuple()
    {}

    HLS_INLINE_STEP void operator=(const hls_state_tuple<> &)
    {}
       
    std::ostream &dump(std::ostream &dst)
    {
        return dst;
    }
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
    
    std::ostream &dump(std::ostream &dst)
    {
        dst<<first<<",";
        return rest.dump(dst);
    }
};


template<class TFirst,class TSecond>
struct hls_binding_tuple_cat_type;

template<class TFirst,class ...TSecondParts>
struct hls_binding_tuple_cat_type<TFirst,hls_binding_tuple<TSecondParts...> >
{
    typedef hls_binding_tuple<TFirst,TSecondParts...> type;
};




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

HLS_INLINE_STEP hls_binding_tuple<> make_hls_binding_tuple()
{
    return hls_binding_tuple<>();
}

#endif
