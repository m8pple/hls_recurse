#ifndef complex_hpp
#define complex_hpp

#include <stdint.h>

#if 0

struct complex_t
{
    int32_t re;
    int32_t im;

  static const int shift = 16;
  static const int64_t bias = 1<<(shift-1);

    complex_t()
    {}

private:
    complex_t(int32_t _re, int32_t _im)
        : re(_re)
        , im(_im)
    {}

    friend complex_t operator+(const complex_t &a, const complex_t &b);
    friend complex_t operator-(const complex_t &a, const complex_t &b);
    friend complex_t operator*(const complex_t &a, const complex_t &b);
public:

  static complex_t from_int(int _re, int _im=0)
  {
    return complex_t(_re<<shift, _im<<shift);
  }

  static complex_t from_float(float _re, float _im=0.0f)
  {
    return complex_t((int32_t)(_re*(1<<shift)), (int32_t)(_im*(1<<shift)));
  }

  complex_t &operator=(const complex_t &o)
  {
    re=o.re;
    im=o.im;
    return *this;
  }

  float re_float()
  { return re * (1.0f/(1<<shift)); }

  float im_float()
  { return im * (1.0f/(1<<shift)); }

  int re_int()
  { return (re+ bias)>>shift; }

  int im_int()
  { return (im+ bias)>>shift; }
};

complex_t operator+(const complex_t &a, const complex_t &b)
{
    return complex_t(a.re+b.re, a.im+b.im);
}

complex_t operator-(const complex_t &a, const complex_t &b)
{
    return complex_t(a.re-b.re, a.im-b.im);
}

complex_t operator*(const complex_t &aa, const complex_t &bb)
{
    int64_t a=aa.re, b=aa.im, c=bb.re, d=bb.im;
    return complex_t(
		     (int32_t)((a*c-b*d+complex_t::bias)>>(complex_t::shift)),
		     (int32_t)((b*c+a*d+complex_t::bias)>>(complex_t::shift))
    );
}


#else

struct complex_t
{
   float re;
    float im;

    complex_t()
    {}

    complex_t(float _re, float _im)
        : re(_re)
        , im(_im)
    {}

  static complex_t from_int(int _re, int _im=0)
  {
    return complex_t(_re, _im);
  }

  static complex_t from_float(float _re, float _im=0.0f)
  {
    return complex_t(_re, _im);
  }

  complex_t &operator=(const complex_t &o)
  {
    re=o.re;
    im=o.im;
    return *this;
  }

  float re_float()
  { return re ; }

  float im_float()
  { return im ; }

  int re_int()
  { return (int)re; }

  int im_int()
  { return (int)im; }
};

complex_t operator+(const complex_t &a, const complex_t &b)
{
    return complex_t(a.re+b.re, a.im+b.im);
}

complex_t operator-(const complex_t &a, const complex_t &b)
{
    return complex_t(a.re-b.re, a.im-b.im);
}

complex_t operator*(const complex_t &aa, const complex_t &bb)
{
    float a=aa.re, b=aa.im, c=bb.re, d=bb.im;
    return complex_t(
		     (a*c-b*d),
		     (b*c+a*d)
    );
}

#endif

#endif
