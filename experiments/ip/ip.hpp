

template<class T,int D>
struct stack_t
{
private:
    T stack0;
    T stack1;
    T stack[D];
    unsigned sp;
public:
    stack_t()
        : sp(0)
    {}

    bool is_empty() const
    { return sp==0; }
    
    int count() const
    { return sp; }
    
    T peek0() const
    {
        assert(!is_empty());
        return stack0;
    }
    
    T peek1() const
    {
        assert(sp>1);
        return stack1;
    }

    T swap(T x)
    {
        assert(sp>0);
        T res=stack0;
        stack0=x;
        return res;
    }
    
    void push(T x)
    {
        assert(sp+1<D);
        stack[sp]=stack1;
        stack1=stack0;
        stack0=x;
        sp++
    }
    
    T pop()
    {
        assert(sp>0);
        sp--;
        T res=stack0;
        stack0=stack1;
        stack1=stack[sp];
        return res;
    }
};

enum opcode_t{
    opcode_dup,      // (n) [s0 s1 ...] -> [s[n] s0 s1 ...]     : Copy a value from n down the stack to the top
    opcode_read_mem, // [s0 ...] -> [mem[s0] ...]   : Read a value from external address space
    opcode_write_mem,// [s0 s1 ...] -> [s1...]      : Do mem[s0]=s1
    opcode_add,      // [s0 s1 ...] -> [s1+s0 ...]
    opcode_sub,      // [s0 s1 ...] -> [s1-s0 ...]
    opcode_jump,     // [...] -> [...],  pc=pc+...
    opcode_bnz,      // [s0 ...] -> [...], if(s0){ pc=pc+... }
    opcode_bz,       // [s0 ...] -> [...], if(s0==0){ pc=pc+... }
    
    opcode_max
};

template<opcode_t Opcode, int I0=0>
struct instr
{
    typedef const Opcode opcode;
    typedef const int constant=I0;
};

template<class ...IList>
struct instr_list
{};
    
template<class ...TA, class ...TB>
struct instr_list_cat
{
    typedef instr_list<TA...,TB...> type;
};


struct VarExpr
{
    static const int v_id = ...;
    
    typedef instr_list<
        instr<opcode_const,v_id>
        instr<opcode_read>
    > assembly;
};

template<class TA,class TB>
struct AddExpr
{
    typedef TA a_t;
    typedef TB b_t;
    
    typedef instr_list<
        a_t::assembly...,
        ...b_t::assembly,
        instr_add
    > assembly;
};

struct MulExpr
{
    
};

template<class TDst, class TSrc>
struct AssignStat
{
    typedef TDst dst_t;
    typedef TSrc src_t;
    
    typedef ilist<
        src_t::assembly,
        instr_const<dst_t::v_id>,
        instr_write
    > assembly;
};

template<class TCond,class TStat>
struct IfStat
{
    const int jump_dist=TStat::assembly::length;
    
    typedef ilist<
        TCond::assembly,
        instr_const<jump_dist>
        instr_bnz,
        TStat::assembly
    > assembly;
};

struct WhileStat
{
    typedef ilist<
        TCond::assembly,
        instr_const<body_dist>+1,
        instr_bnz,
        TBody::assembly,
        instr_jmp<-(cond_dist+2+body_dist+1)>
    > assembly;
};


struct machine_spec_t
{
    static const int opcode_width = 8;
    static const int immediate_width = 16;
    
    typedef uint32_t instr_t;
    typedef int16_t data_t;
    typedef uint16_t addr_t;
    
    static unsigned get_opcode(instr_t x)
    { return (x>>immediate_width) & mask<opcode_width>::value; }
    
    static data_t get_imm_data(instr_t x)
    { return (int16_t)(uint16_t)(x&0xFFFF); }
    
    static int16_t run(
        const instr_t *pInstrMem,
        data_t *pDataMem,
        addr_t pcInit
        int nInputs,
        const *pInputs,
        int nOutputs,
        const *pOutputs
    )
    {
        stack_t<data_t,1024> dstack;
        stack_t<instr_t,64> istack;
        
        bool running=true;
        addr_t pc=pcInit;
        
        for(int i=0; i<nInputs; i++){
            dstack.push(pInputs[i]);
        }
        
        while(running){
            ////////////////////////////////////
            // Fetch
            instr_t instr=pImem[pc];
            
            /////////////////////////////////////
            // Instruction logic
            
            switch (get_opcode(instr)){
            case opcode_dup:
                dstack.push(dstack.peek());
                pc=pc+1;
                break;
            case opcode_constant:
                dstack.push(get_imm_data(instr));
                pc=pc+1;
                break;
            ///////////////////////////////////////
            // ALU stuff
            case opcode_add:
                dstack.push( dstack.peek1()+dstack.peek2() );
                pc=pc+1;
                break;
            case opcode_sub:
                dstack.push( dstack.peek1()-dstack.peek2() );
                pc=pc+1;
                break;
            case opcode_and:
                dstack.push( dstack.peek1()&dstack.peek2() );
                pc=pc+1;
                break;
            case opcode_or:
                dstack.push( dstack.peek1()|dstack.peek2() );
                pc=pc+1;
                break;
            case opcode_xor:
                dstack.push( dstack.peek1()^dstack.peek2() );
                pc=pc+1;
                break;
            //////////////////////////////////////
            // Memory stuff
            case opcode_read_mem:
                dstack.swap( pMem[ dstack.peek0()] );
                pc=pc+1;
                break;
            case opcode_write_mem:
                pMem[ dstack.peek0() ]=dstack.peek1();
                pc=pc+1;
                break;
            //////////////////////////////////////
            // Branches
            case opcode_branch_not_zero:
                if( dstack.peek0() ){
                    pc=pc+1+get_imm_delta(instr);
                }
                dstack.pop();
                break;
            case opcode_bz:
                if( dstack.peek0() != 0 ){
                    pc=pc+1+get_imm_delta(instr);
                }
                dstack.pop();
                break;
            case opcode_jmp:
                pc=pc+1+get_imm_delta(instr);
                break;
            //////////////////////////////////////
            // Functions
            case opcode_call:
                istack.push(pc+1);
                pc=get_imm_absolute(instr);
                break;
            case opcode_return:
                if(istack.is_empty()){
                    running=false;
                    break;
                }else{
                    pc=istack.pop();
                    break;
                }
            /////////////////////////////////////
            // Error
            default:
                assert(0);
            }
        }
        
        assert(dstack.count()==nOutputs);
        
        for(int i=0; i<nOutputs; i++){
            pOutputs[i]=dstack.pop();
        }
    }
}
