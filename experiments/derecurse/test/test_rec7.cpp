int f(int x)
{
  while(x>0){
    x=f(x-2);
  }
  return x;
}

int main()
{
  return f(5);
}

