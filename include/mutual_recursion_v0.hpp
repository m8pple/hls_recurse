#ifndef mutual_recursion_v0_hpp
#define mutual_recursion_v0_hpp

#include "self_recursion_steps.hpp"

namespace v0
{
    template<class TArcSrc>
    struct CallStep
    {
        unsigned m_dstTag;
        TArcSrc m_argSrc;
    };
    
    template<unsigned TKey,unsigned TIndex,class TRest>
    struct key_value_map
    {
        
        static constexpr bool contains(unsigned key)
        {
            if(TTag==tag)
                return true;
            else
                return TRest::contains(tag);
                
        }
        
        static constexpr unsigned lookup(int tag)
        {
            if(TTag==tag)
                return true;
            else
                return TRest::contains(tag);
                
        }
    };
    
    

    template<int TFuncTag, class TRet, class TParams, class TLocals, class TBody>
    struct mutual_function;
    
    template<int TFuncTag, class TRet, class ...TParams, class ...TLocals, class TBody>
    struct mutual_function<
        TFuncTag,
        TRet,
        hls_binding_tuple<TParams...>,
        hls_binding_tuple<TLocals...>,
        TBody
    >
    {
        typedef TRet return_type;
        typedef TParams args_type;
        typedef TLocals locals_type;
        typedef hls_state_tuple<TParams...,TLocals...> state_type;
        typedef hls_state_tuple<TParams...,TLocals...> binding_type;
        typedef TBody body_type;
        
        typedef CallStack<return_type,state_type> call_stack_type;
        
        
    };
    
    template<class TFirst,class TRest>
    struct function_list
    
    
    

TRet run_functions(const TFuncList &body, TArgs ...args)
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
