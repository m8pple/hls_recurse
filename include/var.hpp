#ifndef var_hpp
#define var_hpp

#include <string>
#include <string.h>

namespace hls_recurse
{

template<class TImpl,class T>
class VarExpr
{
private:
    const TImpl&me() const
    {
        return static_cast<const TImpl&>(*this);
    }    
    
public:
    operator const TImpl&() const
    {
        return me();
    }
    
    T operator()() const
    {
        return me().eval();
    }
};

template<class T>
class Const
    : public VarExpr<Const<T>,T>
{
private:
    T m_val;
public:
    typedef T type;

    Const(T val)
        : m_val(val)
    {}
    
    Const &operator=(const Const &) = delete;

    type eval() const
    { return m_val; }
};

template<class T>
class Var
    : public VarExpr<Var<T>,T>
{
private:
    T &m_val;
public:
    typedef T type;

    Var() = delete;
    Var &operator=(const Var &) = delete;

    Var(T &val)
        : m_val(val)
    {}
    
    type eval() const
    { return  m_val; }

    T &get()
    { return  m_val; }
};

template<class T>
Const<T> Val(T x)
{ return Const<T>(x); }


template<class A, class B>
class VarAdd
    : public VarExpr<VarAdd<A,B>,decltype(typename A::type()+typename B::type())>
{
private:
    const A &a;
    const B &b;
public:
    typedef decltype(a.eval()+b.eval()) type;

    VarAdd(const A &_a, const B &_b)
        : a(_a)
        , b(_b)
    {}
    
    type eval() const
    {
        return a.eval()+b.eval();
    }
};


template<class A,class B>
VarAdd<A,B> operator+(const VarExpr<A,typename A::type> &a, const VarExpr<B,typename B::type> &b)
{
    return VarAdd<A,B>(a,b);
}

template<class A, class B>
class VarSub
    : public VarExpr<VarSub<A,B>,decltype(typename A::type()-typename B::type()) >
{
private:
    const A &a;
    const B &b;
public:
    typedef decltype(a.eval()-b.eval()) type;

    VarSub(const A &_a, const B &_b)
        : a(_a)
        , b(_b)
    {}
    
    type eval() const
    {
        return a.eval()-b.eval();
    }
};

template<class A,class B>
VarSub<A,B> operator-(const VarExpr<A,typename A::type> &a, const VarExpr<B,typename B::type> &b)
{
    return VarSub<A,B>(a,b);
}

template<class A, class B>
class VarMul
    : public VarExpr<VarMul<A,B>,decltype(A::type()*B::type()) >
{
private:
    const A &a;
    const B &b;
public:
    typedef decltype(a.eval()*b.eval()) type;

    VarMul(const A &_a, const B &_b)
        : a(_a)
        , b(_b)
    {}
    
    type eval() const
    { return a.eval()*b.eval(); }
};

template<class A,class B>
VarMul<A,B> operator*(const VarExpr<A,typename A::type> &a, const VarExpr<B,typename B::type> &b)
{
    return VarMul<A,B>(a,b);
}

template<class A, class B>
class VarLessThan
    : public VarExpr<VarLessThan<A,B>,bool>
{
private:
    const A &a;
    const B &b;
public:
    typedef bool type;

    VarLessThan(const A &_a, const B &_b)
        : a(_a)
        , b(_b)
    {}
    
    type eval() const
    { return a.eval()<b.eval(); }
};

template<class A,class B>
VarLessThan<A,B> operator<(const VarExpr<A,typename A::type> &a, const VarExpr<B,typename B::type> &b)
{
    return VarLessThan<A,B>(a,b);
}

template<class A, class B>
class VarLessThanEqual
    : public VarExpr<VarLessThanEqual<A,B>,bool>
{
private:
    const A &a;
    const B &b;
public:
    typedef bool type;

    VarLessThanEqual(const A &_a, const B &_b)
        : a(_a)
        , b(_b)
    {}
    
    type eval() const
    { return a.eval()<=b.eval(); }
};

template<class A,class B>
VarLessThanEqual<A,B> operator<=(const VarExpr<A,typename A::type> &a, const VarExpr<B,typename B::type> &b)
{
    return VarLessThanEqual<A,B>((const A&)a,(const B &)b);
}

template<class A,class B>
VarLessThanEqual<A,Const<B>> operator<=(const VarExpr<A,typename A::type> &a, const B &b)
{
    return VarLessThanEqual<A,Const<B>>((const A&)a,Val(b));
}

}; // hls_recurse

#endif
