int g(int x)
{
  if(x<=0){
    return 1;
  }else{
    return x*g(x-1)+g(x-2);
  }
  
}

int h(int x)
{
  int acc=0;
  while(x%7){
    acc= acc+ g(x-1);
    x--;
  }
  return acc;
}

int f(int x, int y)
{
	if(x<=1){
	  return h(y);
	}else{
	  return f(x-1,x+y)+f(x-2,x*y);
	}

}

int main()
{
  return f(5,10)+f(3,4);
}

