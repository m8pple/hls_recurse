


class Lock
{
    volatile bool m_Q[2];
    volatile int m_turn;

    template<int ME>
    void enter()
    {
        m_Q[ME]=true;
        m_turn=ME;
        do{
            ap_wait();
        }while( !m_Q[THEM] || m_turn==THEM);
    }

    template<int ME>
    void leave()
    {
        m_Q[ME]=false;
    }

};


template<class T, unsigned LOG2N>
class Queue
{
public:
    static const unsigned N=LOG2N;

private:
    volatile bool m_leftReq;
    volatile bool m_rightReq;

    volatile unsigned m_leftIndex;
    volatile unsigned m_rightIndex;

    volatile T m_ram[N];
public:
    bool isEmpty()
    {
        return m_leftIndex == m_rightIndex;
    }

    bool isAlmostEmpty()
    {
        return (m_leftIndex+1)&MASK) == m_rightIndex;
    }

    bool isFull()
    {
        return m_leftIndex == ((m_rightIndex+1)&MASK);
    }

    bool isAlmostFull()
    {
        return m_leftIndex == ((m_rightIndex+2)&MASK);
    }

    bool try_push_left(const T &x)
    {
        if(isFull(){
            return false;
        }else{
            m_leftBlockWrite=isAlmostFull();
        }

        ap_wait();

        if(isFull()){
            return false;
        }else{
            m_pRam[m_leftIndex]=x;
            m_leftIndex--;
            m_leftBlockWrite=false;
        }
    }

    bool try_push_right(const T &x)
    {
        if(isFull() || m_leftBlockWrite){
            return false;
        }else{
            m_pRam[m_rightIndex]=x;
            m_rightIndex++;
        }
    }

    bool try_pop_left(T &x)
    {
        if(isEmpty(){
            return false;
        }else{
            m_leftBlockRead=isAlmostEmpty();
        }

        ap_wait();

        if(isEmpty()){
            return false;
        }else{
            x=m_pRam[m_leftIndex];
            m_leftIndex++;
            m_leftBlockRead=false;
        }
    }

    bool try_pop_right(T &x)
    {
        if(isEmpty() || m_letBlockRead){
            return false;
        }else{
            x=m_pRam[m_rightIndex];
            m_rightIndex--;
        }
    }

    Queue()
        :
        , m_leftBlockRead(false)
        , m_leftBlockWrite(false)
        , m_leftIndex(0)
        , m_rightIndex(0)
        , m_ram[N]
    {
    }
};

template<class T>
class SplitQueue
{
private:
    Queue<T,LOG2N> m_clientQueues[2];
    volatile bool m_regReq;
    volatile bool m_regValid;
    volatile T m_reg;
public:
    void run()
    {
        while(1){
            if(m_regValid){

            }

            if( (nLeft==0) && (nRight==0) ){
                if(!m_regValid){
                    m_reqReq=true;
                }else{
                    m_clientQueues[0].push(m_reg);
                    m_regValid=false;
                }
            }else if( (nLeft>0) && (nRight==0) ){
                if(!m_regValid){

                }
            }

            if( nLeft > nRight > 0 && !m_regValid ){

            }
        }
    }

    template<class TWorker>
    void dispatch(TWorker worker)
    {
        worker(m_clientQueues[0]);
        worker(m_clientQueues[1]);
        run();
    }

};

