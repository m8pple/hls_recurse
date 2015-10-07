#ifndef state_machine_steps_hpp
#define state_machine_steps_hpp

#include "hls_recurse/utility.hpp"

namespace hls_recurse
{

/* There is no particular reason for this number, it just reflects
    my assumptions. It is also useful to make sure that things
    don't overflow if my increasingly poor decision to pack
    bit-fields into states continues unabated. */
const unsigned max_total_state_count = 1024;


/* These are special codes which should be propagated back
    without inner->outer adjustment.
*/
enum{
    //! A return statement has executed, and propagates back to the function level (outer manager)
    StateCode_Return         = 0x1000,

    //! A break statemetn has executed, and propagates out to the enclosing loop.
    /*! I guess if it hits the outer part then its a run-time error, though it should be compile-time. */
    StateCode_Break          = 0x2000,

    // Something has gone wrong at run-time. This should never actually happen
    // if the compile-time stuff works correctly
    StateCode_RunTimeError          = 0x4000,

    StateCode_SpecialMask    = StateCode_Return | StateCode_Break | StateCode_RunTimeError
};


enum traits_t{
    // If this trait is true, then this is the end of a sequence, so following
    // this there will be some kind of join e.g. last statement in a loop or if
    // body, or a return
    InheritedTrait_SequenceEnd = 0x1,

    // If this trait is true, then after this step is an implicit return.
    // This automatically implies InheritedTrait_SequenceEnd
    InheritedTrait_ReturnPosition = 0x2,

    // There is an enclosing loop, so Break is valid
    InheritedTrait_WithinLoop = 0x4
};



#if 0
struct ConceptStep
{
    enum{ total_state_count = ... };

    /* If this is true, then this step will always run all
        the way through, so it will start at state 0, and finish with state total_state_count.
        Things which can cause interrupts are returns, breaks, recurse, ...
    */
    enum{ no_interrupts = ... };

    //! Step by a single state, returning the next state to execute
    /*! Context is related to function calls and should not be needed
        by basic steps. However, it should be passed on to inner steps */
    template<unsigned TBase, traits_t TTraits, class TContext>
    unsigned step(unsigned curr, TContext &ctxt) const;

};
#endif

struct Step
{};


template<class TFunc>
struct FunctorStep
    : Step
{
    TFunc m_func;

    HLS_INLINE_STEP FunctorStep(const TFunc &func)
        : m_func(func)
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 1 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &) const
    {
        assert(curr==TBase);
        m_func();
        return curr+1;
    }
};


struct PassStep
    : Step
{

    	HLS_INLINE_STEP PassStep()

    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 1 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &) const
    {
        assert(curr==TBase);
        return curr+1;
    }
};

struct BreakStep // ba-boom, tsh!
    : Step
{
    HLS_INLINE_STEP BreakStep()
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &) const
    {
        static_assert(TTraits&InheritedTrait_WithinLoop, "It appears Break is being called while not in an outer loop.");
        static_assert(TTraits&InheritedTrait_SequenceEnd, "It appears Break is followed by more statements which will not be executed.");

        assert(curr==TBase);
        return StateCode_Break;
    }
};

struct ReturnStep
    : Step
{
    HLS_INLINE_STEP ReturnStep()
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &) const
    {
        static_assert(TTraits&InheritedTrait_SequenceEnd, "It appears Return is followed by more statements which will not be executed.");

        assert(curr==TBase);
        return StateCode_Return;
    }
};

template<class TRetSrc>
struct ReturnWithResultStep
    : Step
{
    TRetSrc m_retSrc;

    HLS_INLINE_STEP ReturnWithResultStep(const TRetSrc &retSrc)
        : m_retSrc(retSrc)
    {}

    enum{ total_state_count = 1 };

    enum{ no_interrupts = 0 };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        static_assert(TTraits&InheritedTrait_SequenceEnd, "It appears Return is followed by more statements which will not be executed.");

        assert(curr==TBase);
        ctxt.GetReturnHolder().set(m_retSrc());
        return StateCode_Return;
    }
};



template<class THeadStep, class TTailStep>
struct SequenceStep
    : Step
{
    THeadStep m_headStep;
    TTailStep m_tailStep;

    HLS_INLINE_STEP SequenceStep(const THeadStep &headStep, const TTailStep &tailStep)
        : m_headStep(headStep)
        , m_tailStep(tailStep)
    {}

    enum{ total_state_count = THeadStep::total_state_count + TTailStep::total_state_count };

    enum{ no_interrupts = THeadStep::no_interrupts && TTailStep::no_interrupts };

    static_assert( total_state_count < max_total_state_count, "Sanity check on total local states" );

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned state, TContext &ctxt) const
    {
        assert(state >= TBase);
        assert(state < TBase+total_state_count);

        const traits_t HeadTraits=(traits_t)(TTraits & ~(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition));  // Cannot be the sequence end or implicit return

#ifdef HLS_ENABLE_SHORT_CUT
        const bool SHORT_CUT=true;
#else
        const bool SHORT_CUT=false;
#endif
        if(SHORT_CUT && (THeadStep::no_interrupts && TTailStep::no_interrupts)){
            //fprintf(stderr, "Seq/NN : no -> no\n");

            // The steps can't be interrupted, we must start in first state, finish in last
            assert(state==TBase);
            unsigned st=m_headStep.template step<TBase,HeadTraits,TContext>(TBase, ctxt);
            assert(st==TBase+THeadStep::total_state_count);
            st=m_tailStep.template step<TBase+THeadStep::total_state_count,TTraits,TContext>(TBase+THeadStep::total_state_count, ctxt);
            assert(st==TBase+total_state_count);
            //fprintf(stderr, "state = %u -> state = %u\n", state, st);
            return TBase+total_state_count;
        }else if(SHORT_CUT && THeadStep::no_interrupts){
            // We must either be at the start of the first step, or somewhere unknown in the second step
            //fprintf(stderr, "Seq/NY : no -> yes\n");
            assert((state==TBase) || (state>=TBase+THeadStep::total_state_count));
            if(state==TBase){ // Slightly more hardware friendly
                unsigned s=m_headStep.template step<TBase,HeadTraits,TContext>(TBase, ctxt);
                assert(s==TBase+THeadStep::total_state_count);
                state=TBase+THeadStep::total_state_count;
            }
            unsigned st=m_tailStep.template step<TBase+THeadStep::total_state_count,TTraits,TContext>(state, ctxt);
            //fprintf(stderr, "state = %u -> state = %u\n", state, st);
            return st;
        }else if(SHORT_CUT && TTailStep::no_interrupts){
            //fprintf(stderr, "Seq/YN : yes -> no\n");
            // We must either be somewhere unknown in the first step, or at the start of the second step
            assert((state<TBase+THeadStep::total_state_count) || (state==TBase+THeadStep::total_state_count));
            if(state!=TBase+THeadStep::total_state_count){
                // Somewhere in the first step
                unsigned st=m_headStep.template step<TBase,HeadTraits,TContext>(state, ctxt);
                if(st!=TBase+THeadStep::total_state_count)
                    return st;
            }
            // Start of the second step
            m_tailStep.template step<TBase+THeadStep::total_state_count,TTraits,TContext>(TBase+THeadStep::total_state_count, ctxt);
            return TBase+total_state_count;
        }else{
            ///fprintf(stderr, "Seq/YY : yes -> yes\n");
            if(state < TBase+THeadStep::total_state_count){
                return m_headStep.template step<TBase,HeadTraits,TContext>(state, ctxt);
            }else{
                return m_tailStep.template step<TBase+THeadStep::total_state_count,TTraits,TContext>(state, ctxt);
            }
        }
    }
};

template<class TCondFunc, class TTrueStep>
struct IfStep
    : Step
{
    TCondFunc m_condFunc;
    TTrueStep m_trueStep;

    HLS_INLINE_STEP IfStep(const TCondFunc &condFunc, const TTrueStep &trueStep)
        : m_condFunc(condFunc)
        , m_trueStep(trueStep)
    {}

    enum{ total_state_count = 1+TTrueStep::total_state_count  };

    #ifdef HLS_ENABLE_SHORT_CUT
            static const bool SHORT_CUT=true;
    #else
            static const bool SHORT_CUT=false;
    #endif

    enum{ no_interrupts = SHORT_CUT && TTrueStep::no_interrupts };

    static_assert( total_state_count < max_total_state_count, "Sanity check on total local states" );

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr >= TBase);
        assert(curr < TBase+total_state_count);

        const traits_t BodyTraits=(traits_t)((TTraits
                | InheritedTrait_SequenceEnd) // The true body is the final thing within its control structure
                // All other flags remain in place
            );

        if(SHORT_CUT){
            if(TTrueStep::no_interrupts){
                assert(curr==TBase);
                if(m_condFunc()){
                    unsigned s=m_trueStep.template step<TBase+1,BodyTraits,TContext>(TBase+1, ctxt);
                    assert(s==TBase+total_state_count);
                }
                return TBase+total_state_count;
            }else{
                if(curr==TBase){
                    if(!m_condFunc()){
                        return TBase+1+TTrueStep::total_state_count;
                    }
                    curr=TBase+1;
                }
                return m_trueStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
            }
        }else{
            if(curr==TBase){
                bool cond=m_condFunc();
                if(cond){
                    return TBase+1;
                }else{
                    return TBase+1+TTrueStep::total_state_count;
                }
            }else{
                return m_trueStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
            }
        }
    }
};

template<class TCondFunc, class TTrueStep, class TFalseStep>
struct IfElseStep
    : Step
{
    TCondFunc m_condFunc;
    TTrueStep m_trueStep;
    TFalseStep m_falseStep;

    HLS_INLINE_STEP IfElseStep(const TCondFunc &condFunc, const TTrueStep &trueStep, const TFalseStep &falseStep)
        : m_condFunc(condFunc)
        , m_trueStep(trueStep)
        , m_falseStep(falseStep)
    {}

    enum{ total_state_count = 1+TTrueStep::total_state_count + TFalseStep::total_state_count  };

    #ifdef HLS_ENABLE_SHORT_CUT
        static const bool SHORT_CUT=true;
#else
        static const bool SHORT_CUT=false;
#endif

    enum{ no_interrupts = SHORT_CUT &&(TTrueStep::no_interrupts && TFalseStep::no_interrupts) };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr >= TBase);
        assert(curr < TBase+total_state_count);

        const traits_t BodyTraits=(traits_t)((TTraits
            | InheritedTrait_SequenceEnd) // The true and false bodies are the final thing within its control structure
            // All other flags remain in place
        );


        if(SHORT_CUT && TTrueStep::no_interrupts && TFalseStep::no_interrupts){
            assert(curr==TBase);
            if(m_condFunc()){
                unsigned tmp=m_trueStep.template step<TBase+1,BodyTraits,TContext>(TBase+1, ctxt);
                assert(tmp==TBase+1+TTrueStep::total_state_count);
            }else{
                unsigned tmp=m_falseStep.template step<TBase+1+TTrueStep::total_state_count,BodyTraits,TContext>(TBase+1+TTrueStep::total_state_count, ctxt);
                assert(tmp==TBase+total_state_count);
            }
            return TBase+total_state_count;
        }else if(SHORT_CUT){
            bool doTrue=false;

            if(curr==TBase){
                if(m_condFunc()){
                    doTrue=true;
                    curr=TBase+1;
                }else{
                    curr=TBase+1+TTrueStep::total_state_count;
                }
            }else{
                if(curr < TBase+1+TTrueStep::total_state_count){
                    doTrue=true;
                }
            }

            if(doTrue){
                unsigned tmp=m_trueStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
                if(TTrueStep::no_interrupts){
                    assert(tmp==TBase+1+TTrueStep::total_state_count);
                    return TBase+total_state_count;
                }else{
                    if(tmp == TBase+1+TTrueStep::total_state_count){
                        return TBase+total_state_count;
                    }else{
                        return tmp;
                    }
                }
            }else{
                unsigned tmp=m_falseStep.template step<TBase+1+TTrueStep::total_state_count,BodyTraits,TContext>(curr, ctxt);
                if(TFalseStep::no_interrupts){
                    assert(tmp==TBase+total_state_count);
                    return TBase+total_state_count;
                }else{
                    return tmp;
                }
            }

        }else{
            if(curr==TBase){
                bool cond=m_condFunc();
                if(cond){
                    return TBase+1;
                }else{
                    return TBase+1+TTrueStep::total_state_count;
                }
            }else if(curr < TBase+1+TTrueStep::total_state_count){
                unsigned tmp=m_trueStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
                if(tmp == TBase+1+TTrueStep::total_state_count){
                    return TBase+total_state_count;
                }else{
                    return tmp;
                }
            }else{
                return m_falseStep.template step<TBase+1+TTrueStep::total_state_count,BodyTraits,TContext>(curr, ctxt);
            }
        }
    }
};

template<class TCondFunc, class TBodyStep>
struct WhileStep
    : Step
{
    const TCondFunc m_condFunc;
    const TBodyStep m_bodyStep;

    HLS_INLINE_STEP WhileStep(const TCondFunc &condFunc, const TBodyStep &bodyStep)
        : m_condFunc(condFunc)
        , m_bodyStep(bodyStep)
    {}

#ifdef HLS_ENABLE_SHORT_CUT
        static const bool SHORT_CUT=false;
#else
        static const bool SHORT_CUT=false;
#endif

    enum{ total_state_count = 1 + TBodyStep::total_state_count };

    // We always use interrupts right now
    enum{ no_interrupts = SHORT_CUT && TBodyStep::no_interrupts };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr >= TBase);
        assert(curr < TBase+total_state_count);

        const traits_t BodyTraits=(traits_t)(((TTraits
                    | InheritedTrait_WithinLoop) // The body is obviously within a loop
                    | InheritedTrait_SequenceEnd) // The body is the final thing within its control structure
                    & ~InheritedTrait_ReturnPosition // The while itself can be within return position, but not the body
                );

        if(SHORT_CUT){
            if(TBodyStep::no_interrupts){
                // rather unlikely, but still...
                while(m_condFunc()){
                    // No breaks in the body
                    unsigned tmp=m_bodyStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
                    assert(tmp==TBase+total_state_count);
                }
                return TBase+total_state_count;
            }else{
                // Leave this as straight-line code, and let outer level
                // keep the loop.
                if(curr==TBase){
                    if(!m_condFunc()){
                        return TBase+total_state_count;
                    }
                    curr=curr+1;
                }
                unsigned tmp=m_bodyStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);
                if(tmp==StateCode_Break)
                    return TBase+total_state_count;
                if(tmp==TBase+total_state_count)
                    return TBase;
                return tmp; // continue with body
            }
        }else{
            if(curr==TBase){
                bool cond=m_condFunc();
                if(cond){
                    return TBase+1; // first statement of body
                }else{
                    return TBase+total_state_count; // skip past loop body out of loop
                }
            }else{
                unsigned tmp=m_bodyStep.template step<TBase+1,BodyTraits,TContext>(curr, ctxt);

                // We are the person who actually deals with this code
                if(tmp==StateCode_Break)
                    return TBase+total_state_count; // skip past loop body out of loop

                // Check whether we should return to loop condition
                if(tmp==TBase+total_state_count)
                    return TBase;

                return tmp; // continue with body
            }
        }
    }
};


template<class TInitFunc, class TCondFunc, class TStepFunc, class TBodyStep>
struct ForStep
    : Step
{
    /*
        for(init(); cond(); step()){
            body()
        }

        =

        init();
        while(cond()){
            body();
            step();
        }

    */

    const TInitFunc m_initFunc;
    const TCondFunc m_condFunc;
    const TStepFunc m_stepFunc;
    const TBodyStep m_bodyStep;

    HLS_INLINE_STEP ForStep(
        const TInitFunc &initFunc,
        const TCondFunc &condFunc,
        const TStepFunc &stepFunc,
        const TBodyStep &bodyStep
    )
        : m_initFunc(initFunc)
        , m_condFunc(condFunc)
        , m_stepFunc(stepFunc)
        , m_bodyStep(bodyStep)
    {}

    enum{ total_state_count = 3 + TBodyStep::total_state_count };

    #ifdef HLS_ENABLE_SHORT_CUT
        static const bool SHORT_CUT=false;
#else
        static const bool SHORT_CUT=false;
#endif

    enum{ no_interrupts = SHORT_CUT && TBodyStep::no_interrupts };

    template<unsigned TBase, traits_t TTraits, class TContext>
    HLS_INLINE_STEP unsigned step(unsigned curr, TContext &ctxt) const
    {
        assert(curr >= TBase);
        assert(curr < TBase+total_state_count);

        const traits_t BodyTraits=(traits_t)(((TTraits
                    | InheritedTrait_WithinLoop) // The body is obviously within a loop
                    | InheritedTrait_SequenceEnd) // The body is the final thing within its control structure
                    & ~InheritedTrait_ReturnPosition // The for itself can be within return position, but not the body
                );

        if(SHORT_CUT){
            if(TBodyStep::no_interrupts){

                m_initFunc();
                while(m_condFunc()){
                    unsigned tmp=m_bodyStep.template step<TBase+2,BodyTraits,TContext>(curr, ctxt);
                    assert(tmp==TBase+2+TBodyStep::total_state_count);
                    m_stepFunc();
                }
                return TBase+total_state_count;
            }else{
                if(curr==TBase){
                    m_initFunc();
                    curr++;
                }
                if(curr==TBase+1){
                    if(!m_condFunc())
                        return TBase+total_state_count;
                    curr++;
                }
                unsigned tmp=m_bodyStep.template step<TBase+2,BodyTraits,TContext>(curr, ctxt);
                if(tmp==StateCode_Break){
                    return TBase+total_state_count;
                }
                if(tmp==TBase+2+TBodyStep::total_state_count){
                    m_stepFunc();
                    return TBase+1;
                }
                return tmp;
            }
        }else{
            if(curr==TBase){
                m_initFunc();
                return TBase+1;
            }else if(curr==TBase+1){
                bool cond=m_condFunc();
                if(cond){
                    return TBase+2; // first statement of body
                }else{
                    return TBase+total_state_count; // skip past loop body out of loop
                }
            }else if(curr<TBase+2+TBodyStep::total_state_count){
                unsigned tmp=m_bodyStep.template step<TBase+2,BodyTraits,TContext>(curr, ctxt);

                // We are the person who actually deals with this code
                if(tmp==StateCode_Break)
                    return TBase+total_state_count; // skip past loop body out of loop

                return tmp; // continue with body, or onto step, or something like return
            }else{ // TBase+2+TBodyStep::total_state_count
                // Loop step

                m_stepFunc();

                return TBase+1; // Back to loop step
            }
        }
    }
};

/*! Runs a state machine that does not contain any function
    related parts. i.e. it should not call return, or recurse,
    or anything like that.
*/
template<class TBody>
void run_state_machine(
    const TBody &body
){
    struct {} ctxt; // The context does nothing

    unsigned next=0;
    while(next < body.total_state_count){
        next=body.template step<0,InheritedTrait_ReturnPosition>(next,ctxt);
    }
    assert(next==body.total_state_count);
}

}; // hls_recurse

#endif
