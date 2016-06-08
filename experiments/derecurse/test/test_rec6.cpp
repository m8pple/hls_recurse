int f(int x, int y)
{
  while(x > 10){
    x=x-1;
    y=f(x,y+1);
  }

  return y;
}

int main()
{
  return f(5,10)+f(3,4);
}

