#ifndef state_machine_builder_hpp
#define state_machine_builder_hpp


#include "hls_recurse/state_machine_steps.hpp"

namespace hls_recurse{

namespace detail
{
    template<class TF>
    struct LiftSingleStep
    {
        typedef FunctorStep<TF> type;
        
        HLS_INLINE_STEP static type lift(const TF &step)
        { return type(step); }
    };
    
    template<class TF>
    struct LiftSingleStep<FunctorStep<TF>  >
    {
        typedef FunctorStep<TF> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<class TA,class TB>
    struct LiftSingleStep<SequenceStep<TA,TB>  >
    {
        typedef SequenceStep<TA,TB> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<class TA,class TB>
    struct LiftSingleStep<IfStep<TA,TB>  >
    {
        typedef IfStep<TA,TB> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<class TA,class TB,class TC>
    struct LiftSingleStep<IfElseStep<TA,TB,TC>  >
    {
        typedef IfElseStep<TA,TB,TC> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<class TA,class TB>
    struct LiftSingleStep<WhileStep<TA,TB>  >
    {
        typedef WhileStep<TA,TB> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<>
    struct LiftSingleStep<BreakStep>
    {
        typedef BreakStep type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<>
    struct LiftSingleStep<ReturnStep>
    {
        typedef ReturnStep type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    template<class TRet>
    struct LiftSingleStep<ReturnWithResultStep<TRet> >
    {
        typedef ReturnWithResultStep<TRet> type;
        
        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
    
    
    
    template<class ...TArgs>
    struct LiftSequence;
    
    template<class TFirst,class TSecond, class ...TRest>
    struct LiftSequence<TFirst,TSecond,TRest...>
    {
        typedef SequenceStep<
            typename LiftSingleStep<TFirst>::type,
            typename LiftSequence<TSecond,TRest...>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TFirst &first, const TSecond &second, const TRest &...rest)
        {
            return type(
                LiftSingleStep<TFirst>::lift(first),
                LiftSequence<TSecond, TRest...>::lift(second, rest...)
            );
        }
    };
    
    template<class TFirst,class TSecond>
    struct LiftSequence<TFirst,TSecond>
    {
        typedef SequenceStep<
            typename LiftSingleStep<TFirst>::type,
            typename LiftSingleStep<TSecond>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TFirst &first, const TSecond &second)
        {
            return type(
                LiftSingleStep<TFirst>::lift(first),
                LiftSingleStep<TSecond>::lift(second)
            );
        }
    };
    
    template<class TFirst>
    struct LiftSequence<TFirst>
    {
        typedef typename LiftSingleStep<TFirst>::type type;
        
        HLS_INLINE_STEP static type lift(const TFirst &first)
        {
            return LiftSingleStep<TFirst>::lift(first);
        }
    };
    
    
    template<class TCondFunc,class ...TBody>
    struct LiftIf
    {
        typedef IfStep<
            TCondFunc,
            typename LiftSequence<TBody...>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TCondFunc &func, const TBody & ...t)
        {
            return type(
                func,
                LiftSequence<TBody...>::lift(t...)
            );
        }
    };
    
    template<class TCondFunc,class TTrue,class TFalse>
    struct LiftIfElse
    {
        typedef IfElseStep<
            TCondFunc,
            typename LiftSingleStep<TTrue>::type,
            typename LiftSingleStep<TFalse>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TCondFunc &func, const TTrue &t, const TFalse &f)
        {
            return type(
                func,
                LiftSingleStep<TTrue>::lift(t),
                LiftSingleStep<TFalse>::lift(f)
            );
        }
    };
    
    template<class TCondFunc,class ...TBody>
    struct LiftWhile
    {
        typedef WhileStep<
            TCondFunc,
            typename LiftSequence<TBody...>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TCondFunc &func, const TBody  &...t)
        {
            return type(
                func,
                LiftSequence<TBody...>::lift(t...)
            );
        }
    };
    
    template<class TInitFunc, class TCondFunc,class TStepFunc,class ...TBody>
    struct LiftFor
    {
        typedef ForStep<
            TInitFunc,
            TCondFunc,
            TStepFunc,
            typename LiftSequence<TBody...>::type
        > type;
        
        HLS_INLINE_STEP static type lift(const TInitFunc &init, const TCondFunc &cond, const TStepFunc &step, const TBody  &...t)
        {
            return type(
                init,
                cond,
                step,
                LiftSequence<TBody...>::lift(t...)
            );
        }
    };
};

HLS_INLINE_STEP ReturnStep Return()
{ return ReturnStep(); }

template<class TResultSrc>
HLS_INLINE_STEP ReturnWithResultStep<TResultSrc> Return(const TResultSrc &src)
{ return ReturnWithResultStep<TResultSrc>(src); }

HLS_INLINE_STEP BreakStep Break()
{ return BreakStep(); }

HLS_INLINE_STEP PassStep Pass()
{ return PassStep(); }

template<class ...TArgs>
HLS_INLINE_STEP typename detail::LiftSequence<TArgs...>::type Sequence(const TArgs ...args)
{
    return detail::LiftSequence<TArgs...>::lift(args...);
}

template<class TCondFunc,class ...TBodySteps>
HLS_INLINE_STEP typename detail::LiftIf<TCondFunc,TBodySteps...>::type If(const TCondFunc &cond, const TBodySteps &...body)
{
    return detail::LiftIf<TCondFunc,TBodySteps...>::lift(cond, body...);
}   

template<class TCondFunc,class TTrueStep,class TFalseStep>
HLS_INLINE_STEP typename detail::LiftIfElse<TCondFunc,TTrueStep,TFalseStep>::type IfElse(const TCondFunc &cond, const TTrueStep &tBody, const TFalseStep &fBody)
{
    return detail::LiftIfElse<TCondFunc,TTrueStep,TFalseStep>::lift(cond, tBody, fBody);
}   

template<class TCondFunc,class ...TBodySteps>
HLS_INLINE_STEP typename detail::LiftWhile<TCondFunc,TBodySteps...>::type While(const TCondFunc &cond, const TBodySteps &...body)
{
    return detail::LiftWhile<TCondFunc,TBodySteps...>::lift(cond, body...);
}   

template<class TInitFunc, class TCondFunc, class TStepFunc, class ...TBodySteps>
HLS_INLINE_STEP typename detail::LiftFor<TInitFunc,TCondFunc,TStepFunc,TBodySteps...>::type For(
    const TInitFunc &init, const TCondFunc &cond, const TStepFunc &step, const TBodySteps &...body
){
    return detail::LiftFor<TInitFunc,TCondFunc,TStepFunc,TBodySteps...>::lift(init,cond,step,body...);
}   

}; // hls_recurse

#endif
