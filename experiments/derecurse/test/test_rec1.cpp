int g(int x)
{
  int acc=1;
  while(x>0){
    x=x*acc;
    --x;
  }
  return acc;
}


int f(int x, int y)
{
	if(x<=1){
	  return g(y);
	}else{
	  return f1(x-1,x+y)+f1(x-2,x*y);
	}

}

int main()
{
  return f1(5,10);
}

