#ifndef mutual_recursion_v0_hpp
#define mutual_recursion_v0_hpp

#include "hls_recurse/self_recursion_steps.hpp"

namespace v0
{

template<unsigned TId, class TArgSrc>
struct CallStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP CallStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };
    
    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr==TBase);

        auto args=m_argSrc();

        // TODO: Tail recursion
        
        return ctxt.Call<TId>(args);
    }
};

template<unsigned SN>
    class selector
    {
        template<class TStateTuple>
        void SetContext(const TStateTuple &tuple)
        {
            m_parent.tail.template SetContext<SN>(tuple);
        }
        
        typedef decltype(m_parent.tail.template GetReturnValueById<SN>()) return_type;
        
        return_type GetReturnValueById()
        {
            return m_parent.tail.template GetReturnValueById<SN>();
        }
    };
    
    template<>
    class selector<N>
    {
        template<class TStateTuple>
        void SetContextById(const TStateTuple &tuple)
        {
            m_parent.SetContext(tuple);
        }
        
        typedef decltype(m_parent.GetReturnValue()) return_type;
        
        return_type GetReturnValueById()
        {
            return m_parent.tail.template GetReturnValue();
        }
    };





    

template<unsigned N, class THeadBody,class ...TTailBodies>
class multi_stack
{
    // Id of the body associated with this stack
    static const unsigned function_id = ...;
    static const unsigned function_state_count = body_t::total_state_count;
    
    typedef ... state_t;
    typedef ... binding_t;
    typedef ... body_t;
    
    multi_stack<TTail> tail;
    
    binding_t m_context;
    
    /*! Return the starting state for given function id, 
        or 0xFFFFFFFFul if it doesn't exist */
    static constexpr unsigned GetFunctionStartState(unsigned id)
    {
        if(id==function_id){
            return 0;
        }else{
            return function_state_count+tail_t::GetFunctionStartState(id);
        }
    }
    
    
    template<unsigned TTargetId,class TArgs>
    unsigned Call(
};    


TRet run_functions(const TFuncList &body, unsigned initId, TInitArgs ...args)
{
    const unsigned N = ...; // Number of functions
    
    multi_stack.template SetContext<N-1>(make_hls_state_tuple(args...));

    unsigned sFunc=N-1; // Last function is active
	unsigned s=0;
	while(1){
        func=multi_stack.RunToReturnOrCall(func);
	}

    return call_stack.template GetReturnHolder<N-1>().get();
}

#error "None of this works, just sketching out ideas."


#endif
