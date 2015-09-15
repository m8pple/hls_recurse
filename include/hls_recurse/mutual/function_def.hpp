#ifndef hls_recurse_mutual_function_def_hpp
#define hls_recurse_mutual_function_def_hpp

#include "hls_recurse/hls_tuple.hpp"
#include "hls_recurse/state_machine_steps.hpp"

#include <stdio.h>

namespace hls_recurse
{
namespace mutual
{
    
namespace detail
{
    /* Used as a signaller that something wasn't found in a templated list.
        The chances of having a templatised list this big is
        desperately unlikely, would cause memory exhaustion at
        compilation, and would be disallowed by compiler recursion
        limits.
        Plus I'm too lazy to do a proper sentinel.
        The reason for not using unsigned max is because we want to
        add numbers to it while searching lists.
    */
    const unsigned MAX_INDEX = 1<<24;
};

template<unsigned TId, class TRetType, class TArgTuple, class TLocalTuple, class TBody>
struct function_def
{
    static_assert(TId==-1, "TArgTuple and TLocalTuple must both be binding tuples.");
};
    
template<unsigned TId, class TRetType, class ...TArgTypes, class ...TLocalTypes, class TBody>
class function_def<TId, TRetType, hls_binding_tuple<TArgTypes...>, hls_binding_tuple<TLocalTypes...>, TBody>
{   
public:
    static const unsigned function_id = TId;
    
    // We have one extra slot to handle implicit return
    static const unsigned total_state_count = 1+TBody::total_state_count;
    
    typedef TRetType signature_type(TArgTypes...);
    
    typedef TRetType return_type;
    
    typedef hls_state_tuple<TArgTypes...> input_state_type;
    typedef hls_binding_tuple<TArgTypes...> input_binding_type;
    
    typedef hls_state_tuple<TLocalTypes...> locals_state_type;
    typedef hls_binding_tuple<TLocalTypes...> locals_binding_type;
    
    typedef hls_state_tuple<TArgTypes...,TLocalTypes...> total_state_type;
    typedef hls_binding_tuple<TArgTypes...,TLocalTypes...> total_binding_type;
    
    function_def(
        const hls_binding_tuple<TArgTypes...> &arguments,
        const hls_binding_tuple<TLocalTypes...> &locals,
        const TBody &body
    )
        : m_arguments(arguments)
        , m_locals(locals)
        , m_state(concatenate_binding_tuples(arguments,locals))
        , m_body(body)
    {}
    
private:
    input_binding_type m_arguments;
    locals_binding_type m_locals;
    total_binding_type m_state;
    const TBody m_body;
        
public:
    HLS_INLINE_STEP total_state_type GetState() const
    { return total_state_type(m_state); }
    
    HLS_INLINE_STEP void SetState(const total_state_type &s)
    { m_state=s; }
    
    HLS_INLINE_STEP void SetArguments(const input_state_type &s)
    {
        m_arguments=s;
        m_locals=locals_state_type();
    }

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr>=TBase);
        assert(curr<TBase+total_state_count);
        if(curr<TBase+m_body.total_state_count){
            return m_body.template step<TBase,TTraits,TContext>(curr, ctxt);
        }else{
            // If the function does not have void return type it should
            // explicitly return something
            assert( (std::is_same<TRetType,void>::value) );
            printf("Implicit return\n");
            return StateCode_Return;
        }
    }
};

template<unsigned TId, class TRetType, class ...TArgTypes, class ...TLocalTypes, class TBody>
function_def<TId,TRetType,hls_binding_tuple<TArgTypes...>, hls_binding_tuple<TLocalTypes...>, TBody>
    FunctionDef(
        const hls_binding_tuple<TArgTypes...> &args,
        const hls_binding_tuple<TLocalTypes...> &locals,
        const TBody &body
    )
{
    return function_def<TId,TRetType,hls_binding_tuple<TArgTypes...>, hls_binding_tuple<TLocalTypes...>, TBody>(
        args,
        locals,
        body
    );
}


template<class ...TFunctions>
struct function_def_list;

template<>
struct function_def_list<>
{
    static const unsigned total_state_count = 0;
    
    template<unsigned TSelId>
    struct by_id
    {
        static const int relative_index = detail::MAX_INDEX;
                
    };
    
    template<unsigned TBase, traits_t TTraits, class TContext>
    unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(0);
        return 0;
    }
};

template<class TFirstFunction, class ...TFunctions>
struct function_def_list<TFirstFunction,TFunctions...>
{
    // The condition is just to make sure it only fails if expanded
    static_assert(!std::is_same<TFirstFunction,TFirstFunction>::value, "First argument must be of type function_def.");
};

namespace detail
{
    template<unsigned TSelId, class TList>
    struct function_def_by_id
    {
        static const unsigned relative_index = 1 + function_def_by_id<TSelId,typename TList::tail_type>::relative_index;
        
        static const unsigned relative_start_state = TList::head_type::total_state_count + function_def_by_id<TSelId,typename TList::tail_type>::relative_start_state;
        
        typedef typename function_def_by_id<TSelId,typename TList::tail_type>::function_def_type function_def_type;
    };
    
    template<class TList>
    struct function_def_by_id<TList::head_type::function_id, TList>
    {
        static const unsigned relative_index = 0;
        
        static const unsigned relative_start_state = 0;
        
        typedef typename TList::head_type function_def_type;
    };
    
    struct NO_FUNCTION_WITH_GIVEN_ID_FOUND;
    
    template<unsigned TSelId>
    struct function_def_by_id<TSelId, function_def_list<> >
    {
        static const unsigned relative_index = detail::MAX_INDEX;
        
        static const unsigned relative_start_state = detail::MAX_INDEX;
        
        typedef NO_FUNCTION_WITH_GIVEN_ID_FOUND function_def_type;
    };  
};


template<unsigned TId, class TRetType, class ...TArgTypes, class ...TLocalTypes, class TBody, class ...TFunctions>
struct function_def_list<
    function_def<
        TId,
        TRetType,
        hls_binding_tuple<TArgTypes...>,
        hls_binding_tuple<TLocalTypes...>,
        TBody
    >,
    TFunctions...
>
{
    typedef function_def<
        TId,
        TRetType,
        hls_binding_tuple<TArgTypes...>,
        hls_binding_tuple<TLocalTypes...>,
        TBody
    > function_def_type;
    
    typedef function_def_type head_type;
    typedef function_def_list<TFunctions...> tail_type;
    
    template<unsigned TSelId>
    static constexpr unsigned IndexOf()
    {
        return detail::function_def_by_id<TSelId,function_def_list>::relative_index;
    }
    
    template<unsigned TSelId>
    static constexpr bool IsValidId()
    { return IndexOf<TSelId>() < detail::MAX_INDEX; }
    
    template<unsigned TSelId>
    static constexpr unsigned StartStateOf()
    {
        static_assert(IsValidId<TSelId>(), "Attempt to get start state of unknown function.");
        return detail::function_def_by_id<TSelId,function_def_list>::relative_start_state;
    }
    
    
    static_assert(detail::function_def_by_id<TId,function_def_list>::relative_index < detail::MAX_INDEX, "Function def list contains the same id twice.");
    
    const head_type m_head;
    tail_type m_tail;
    
    function_def_list(const function_def_type &head, const TFunctions &... tail)
        : m_head(head)
        , m_tail(tail...)
    {}
        
    //////////////////////////////////////
    // Step concept
        
    static const unsigned total_state_count = TBody::total_state_count + tail_type::total_state_count;    
           
    static_assert( total_state_count < max_total_state_count, "Total number of states exceeds current (arbitrary) limit on states in a program." );    
        
    enum{ no_interrupts = 0 };
    
    //! Step by a single state, returning the next state to execute
    /*! Context is related to function calls and should not be needed
        by basic steps. However, it should be passed on to inner steps */
    template<unsigned TBase, traits_t TTraits, class TContext>
    unsigned step(unsigned curr, TContext &ctxt) const
    {
        static_assert( (TTraits & InheritedTrait_WithinLoop)==0, "Function list cannot be within a loop.");
        static_assert( (TTraits & InheritedTrait_SequenceEnd)!=0, "Function list must be sequence end.");
        static_assert( (TTraits & InheritedTrait_ReturnPosition)!=0, "Function list must be in return position.");
        
        
        assert(curr>=TBase);
        if(curr < TBase+total_state_count){
            return m_head.template step<TBase, TTraits, TContext>(curr, ctxt);
        }else{
            return m_tail.template step<TBase+total_state_count, TTraits, TContext>(curr,ctxt);
        }
    }
};

template<class ...TFunctions>
function_def_list<TFunctions...> Functions(
    const TFunctions &...functions
){
    return function_def_list<TFunctions...>(functions...);
}


}; // mutual
}; // hls_recurse

#endif
