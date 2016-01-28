#ifndef state_machine_mutual_recursion_v0_hpp
#define state_machine_mutual_recursion_v0_hpp

#include "hls_recurse/mutual_recursion_steps.hpp"

namespace hls_recurse
{

namespace v0
{
    template<uint32_t FuncId, class TRet, class ...TArgs, class ...TLocals>
    struct recursive_decl<FuncId,TRet(TArgs...),TLocals...>
    {
        static const uint32_t func_id = FuncId;
        
        typedef TRet(TArgs...) prototype_t;
        
        typedef TRet retval_t;
        
        typedef hls_tuple<TArgs...> parameters_t;
        typedef hls_binding_tuple<TArgs...> parameters_binding_t;
        
        typedef hls_tuple<TLocals...> locals_t;
        typedef hls_binding_tuple<TLocals...> locals_binding_t;
        
        typedef hls_tuple<TArgs...,TLocals...> state_value_t;
        typedef hls_tuple<TArgs... &,TLocals...&> state_binding_t;
        
        const int params_size = ...;
        const int state_size = ...;
        
        static void UnpackState(state_binding_t &dest, const uint32_t *pSource);
        static void PackState(uint32_t *pDest, const state_binding_t &dest);
    };
    
    
    struct f1_proto : recursive_proto<0x12345, int(int,int), int, int> {};
    
    template<class TDef, class TDecl>
    struct recursive
    {
    protected:
        recursive(
            TDecl::parameters_binding_t params,
            TDecl::locals_binding_t locals
        )
            : m_params(params)
            , m_locals(locals)
        {}
        
            
    };
    
    struct f1
        : recursive<f1, f1_proto>
    {
        int p1, p2;
        int s1, s2;
        
        f1()
            : recursive<f1, f1_proto>(
                Parameters(p1,p2),
                Locals(s1,s2)
            )
        {}
            
        template<class TVisitor>
        void visit(TVisitor visitor)
        {
            visitor(
                While( [&](){ return s1 < s2; } ,
                    [&](){
                        s1 = p1 + p2;
                        p2 = p2 * 5;
                    }
                )
            );
        {}
        
    };
    
    
    template<class TDecl>
    struct TaskManager
    {
        enum task_state
        {
            task_state_free,
            task_state_allocated,
            task_state_ready,
            task_state_executing,
            task_state_blocked
        };
        
        struct task_context_t
        {
            task_id_t taskId;
            task_state taskState;
            ref_count_t refCount;
            state_id_t currentState;
            typename TDecl::retval_t retVal;
            typename TDecl::state_t stateTuple;
        };
        
        task_context_t contexts[MAX_CONTEXTS];
        
        queue<task_id_t,MAX_CONTEXTS> freeContexts;
        
        task_id_t AllocateTask()
        {
            return freeContexts.pop();
        }
        
        void AddRef(task_id_t id, ref_count_t increment)
        {
            lock_t lock(m_mutex);
            
            task_context_t &ctxt=m_contexts[id];
            assert(ctxt.taskState==task_state_executing);
            ctxt.refCount++;
        }
        
        void Release(task_id_t id)
        {
            bool ready;
            {
                lock_t lock(m_mutex);
                
                task_context_t &ctxt=m_contexts[id];
                assert(ctxt.refCount>0);
                ctxt.refCount--;
                ready=ctxt.refCount==0;
            }
            if(ready){
                AddToReady(id);
            }
        }
        
    };
    
    
    template<class TFunction>
    struct pool_worker
    {
    private:
        
    public:
        pool_worker()
        {}
            
        template<class TStack>
        void run(TStack &stack)
        {
            unsigned s=TFunction::length;
            
            while(1){
                if( (s==TFunction::length) || (s==StateCode_Return) ){
                    
                    if(!stack.Pop(s, m_ctxt))
                        break;
                }
                
                // s is now valid
                const traits_t Traits = (traits_t)(InheritedTrait_SequenceEnd|InheritedTrait_ReturnPosition);
                s=body.template step<0,Traits,call_stack_t>(s, call_stack);
            }
        }
    };
    
    template<class TFunctions...>
    struct worker_pool
    {
    public:
        template<class TDecl>
        typename TDecl::retval_t call( const TDecl::parameters_t &params )
        {
            
        }
    };
}

}; // hls_recurse

#endif
