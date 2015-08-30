#ifndef var_extensions_hpp
#define var_extensions_hpp



#include "hls_recurse/state_machine_self_recusion.hpp"

#include "hls_recurse/var.hpp"

namespace hls_recurse
{

template<class ...TVars>
struct var_list_to_tuple;


template<>
struct var_list_to_tuple<>
{
    typedef hls_state_tuple<> type;
    
    type operator()() const
    {
        return type();
    }
};
    
template<class TFirstVar,class ...TRestVars>
struct var_list_to_tuple<TFirstVar,TRestVars...>
{
    typedef typename hls_state_tuple_cat_type<typename TFirstVar::type,typename var_list_to_tuple<TRestVars...>::type >::type type;
    
    const TFirstVar &head;
    var_list_to_tuple<TRestVars...> tail;
    
    var_list_to_tuple(const TFirstVar &_head, const TRestVars &..._tail)
        : head(_head)
        , tail(_tail...)
    {}
    
    type operator()() const
    {
        auto hv=head.eval();
        auto tv=tail();
        return type(hls_make_tag(),hv,tv);
    }
};



template<class ...T>
HLS_INLINE_STEP RecurseStep<var_list_to_tuple<T...> > Recurse(const T &...src)
{ return RecurseStep<var_list_to_tuple<T...>>(var_list_to_tuple<T...>(src...)); }


/*
template<class TArgSrc>
HLS_INLINE_STEP RecurseAndReturnStep<TArgSrc> RecurseAndReturn(const TArgSrc &src)
{ return RecurseAndReturnStep<TArgSrc>(src); }
*/


template<class TRetDst, class ...T>
HLS_INLINE_STEP RecurseWithResultStep<TRetDst, var_list_to_tuple<T...>> RecurseWithResult(TRetDst &dst, const T &...src)
{ return RecurseWithResultStep<TRetDst,var_list_to_tuple<T...>>(dst, var_list_to_tuple<T...>(src...)); }

}; // hls_recurse

#endif
