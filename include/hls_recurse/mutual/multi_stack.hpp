#ifndef hls_recurse_mutual_multi_stack_hpp
#define hls_recurse_mutual_multi_stack_hpp

#include "hls_recurse/mutual/function_def.hpp"

namespace hls_recurse
{
    
namespace mutual
{
    
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

template<class TFunctionDefList>
class multi_stack;

/*hls_recurse::mutual::multi_stack<
    hls_recurse::mutual::function_def_list<
        hls_recurse::mutual::function_def<
            10u,
            void,
            hls_recurse::hls_binding_tuple<int, int>,
            hls_recurse::hls_binding_tuple<int>,
            hls_recurse::FunctorStep<main()::<lambda()> >
        >
    >
>*/

// This adds a data stack to the list, holding context for one function
template<class THead,class ...TRest>
class multi_stack<function_def_list<THead,TRest...> >
{
public:
    static const unsigned function_id = THead::function_id;
    typedef THead function_def;

    typedef function_def_list<THead,TRest...> flist;
    
    typedef typename THead::total_state_type state_tuple_t;

    typedef multi_stack<typename flist::tail_type> tail_type;
private:
    state_tuple_t *m_stackStorage;
    unsigned m_stackPtr;
    
    const THead &m_func;

    return_holder<typename THead::return_type> m_return;
    
    tail_type &m_tail;
    
public:
    multi_stack(state_tuple_t *stackStorage, const THead &headFunc, multi_stack<typename flist::tail_type> &tailStack)
        : m_stackStorage(stackStorage)
        , m_stackPtr(0)
        , m_func(headFunc)
        , m_tail(tailStack)
    {}
        
    HLS_INLINE_STEP tail_type &Tail()
    { return m_tail; }
        
    HLS_INLINE_STEP bool IsEmpty() const
    { return m_stackPtr==0; }
    
    HLS_INLINE_STEP bool IsFull() const
    { return false; } // TODO
    
    HLS_INLINE_STEP void PopData()
    {
        assert(!IsEmpty());
        
        m_func.SetState(m_stackStorage[--m_stackPtr]);
    }
    
    HLS_INLINE_STEP void PushData(const state_tuple_t &args)
    {
        assert(!IsFull());
        m_stackStorage[m_stackPtr++] = m_func.GetState();
        m_func.SetArguments(args);
    }
};

// The final part is the control stack, which is shared amongst
// all functions
template<>
class multi_stack<function_def_list<>>
{
private:
    unsigned *m_stackStorage;
    unsigned m_stackPtr;
public:    
    HLS_INLINE_STEP bool IsEmpty() const
    { return m_stackPtr==0; }
    
    HLS_INLINE_STEP bool IsFull() const
    { return false; } // TODO
    
    HLS_INLINE_STEP unsigned PopControl()
    {
        assert(!IsEmpty());
        return m_stackStorage[--m_stackPtr];
    }
    
    HLS_INLINE_STEP void PushControl(unsigned s)
    {
        assert(!IsFull());
        m_stackStorage[m_stackPtr++]=s;
    }
};

template<unsigned TSelId, class TStack>
struct multi_stack_by_id
{
    // Pushes the given args onto the data stack of TSelId and sets (TSelId,s) as the
    // top of the control stack
    template<class TArgs>
    HLS_INLINE_STEP static void PushDataAndControl(TStack &stack, unsigned s, const TArgs &args)
    {
        multi_stack_by_id<TSelId,typename TStack::tail_type>::PushDataAndControl<TSelId>(stack.Tail(), s, args);
    }
    
    void PushControl(TStack &stack, unsigned s)
    { stack.Tail().PushControl(s); }
    
    std::pair<unsigned,unsigned> PopControl(TStack &stack)
    {
        return stack.Tail().PopControl();
    }
};

template<class TStack>
struct multi_stack_by_id<TStack::function_def::function_id, TStack>
{
    template<class TArgs>
    HLS_INLINE_STEP static void PushDataAndControl(TStack &stack, unsigned s, const TArgs &args)
    {
        stack.Push(args);
        stack.tail.PushControl(s);
    }
    
    void PushControl(TStack &stack, unsigned s)
    { stack.PushControl(s); }
};

template<unsigned TSelId>
struct multi_stack_by_id<TSelId,function_def_list<>>
{
    typedef multi_stack<function_def_list<>> stack_t;
    
    template<class TArgs>
    HLS_INLINE_STEP static void PushDataAndControl(stack_t &stack, unsigned s, const TArgs &args)
    {
        static_assert(!std::is_same<TArgs,TArgs>::value, "Attempt to push for an unknown function id.");
    }
    
    void PushControl(stack_t &stack, unsigned s)
    { stack.Push(TSelId,s); }

};

}; // mutual
}; // hls_recurse

#endif
