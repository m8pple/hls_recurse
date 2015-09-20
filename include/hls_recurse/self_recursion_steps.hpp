#ifndef self_recursion_steps_hpp
#define self_recursion_steps_hpp

/* This is stuff that is shared between
    the various self-recursion front-ends,
    and provides the basic recursion steps.
   */

#include "hls_recurse/state_machine_steps.hpp"
#include "hls_recurse/state_machine_builder.hpp"

#include <stdio.h>

#ifndef __SYNTHESIS__
#include <iostream>
#endif

#include "hls_recurse/hls_tuple.hpp"

namespace hls_recurse{

template<class TTuple>
struct generic_context_t{
    unsigned first;
    TTuple second;

    HLS_INLINE_STEP generic_context_t()
    {}

    HLS_INLINE_STEP generic_context_t(unsigned _first, const TTuple &_second)
        : first(_first)
        , second(_second)
    {}

#ifndef __SYNTHESIS__
    void dump(std::ostream &dst) const
    {
        dst<<"Ctxt<sId="<<first<<",locals=";
        second.dump(dst);
        dst<<">";
    }

#endif
};


template<class TRet>
struct return_holder
{
    TRet m_value;

    HLS_INLINE_STEP TRet get()
    {
        return m_value;
    }

    HLS_INLINE_STEP void set(const TRet &val)
    {
        m_value=val;
    }
};

template<>
struct return_holder<void>
{
	HLS_INLINE_STEP void get()
    {}
};

template<class TArgSrc>
struct RecurseStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr==TBase);

        auto args=m_argSrc();

        if(TTraits & InheritedTrait_ReturnPosition){
            // Tail recursion position, we simply overwrite the current context,
            // we don't need to put anything on the stack

            // This should be statically visible to the compiler and fully inlined,
            // so either this path or the next will be in the code
            //printf("Tail recursion\n");
        }else{
            //printf("True recursion\n");
            ctxt.PushContext(TBase+1);
        }
        ctxt.SetContext(args);
        return 0; // Back to the beginning of the function
    }
};

template<class TArgSrc>
struct RecurseAndReturnStep
    : Step
{
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseAndReturnStep(const TArgSrc &args)
        : m_argSrc(args)
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr==TBase);

        auto args=m_argSrc();

        // Recurse and return is explicitly tail recursion, so
        // we simply overwrite the current context, and we don't
        // need to put anything on the stack
        ctxt.SetContext(args);

        return 0; // Back to the beginning of the function
    }
};

template<class TRetDst, class TArgSrc>
struct RecurseWithResultStep
    : Step
{
    TRetDst &m_retDst;
    TArgSrc m_argSrc;

    HLS_INLINE_STEP RecurseWithResultStep(TRetDst &retDst, const TArgSrc &args)
        : m_retDst(retDst)
        , m_argSrc(args)
    {}

    enum{ total_state_count = 2 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert( (curr==TBase) || (curr==TBase+1) );

        if(curr==TBase){
            // Kick of the recursive call
            auto args=m_argSrc();
            ctxt.PushContext(TBase+1); // We will resume with the assignment from return
            ctxt.SetContext(args);
            return 0; // Back to the beginning of the function
        }else{
            // Assign the return value to the correct place
            m_retDst=ctxt.GetReturnHolder().get();
            return TBase+2; // And carry on
        }
    }
};

namespace detail{
    template<class TF>
    struct LiftSingleStep<RecurseStep<TF>  >
    {
        typedef RecurseStep<TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };

    template<class TF>
    struct LiftSingleStep<RecurseAndReturnStep<TF>  >
    {
        typedef RecurseAndReturnStep<TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };

    template<class TR, class TF>
    struct LiftSingleStep<RecurseWithResultStep<TR,TF>  >
    {
        typedef RecurseWithResultStep<TR,TF> type;

        HLS_INLINE_STEP static type lift(const type &step)
        { return step; }
    };
};

template<class TArgSrc>
HLS_INLINE_STEP RecurseStep<TArgSrc> Recurse(const TArgSrc &src)
{ return RecurseStep<TArgSrc>(src); }

template<class TArgSrc>
HLS_INLINE_STEP RecurseAndReturnStep<TArgSrc> RecurseAndReturn(const TArgSrc &src)
{ return RecurseAndReturnStep<TArgSrc>(src); }

template<class TRetDst, class TArgSrc>
HLS_INLINE_STEP RecurseWithResultStep<TRetDst, TArgSrc> RecurseWithResult(TRetDst &dst, const TArgSrc &src)
{ return RecurseWithResultStep<TRetDst,TArgSrc>(dst,src); }

}; // hls_recurse

#endif
