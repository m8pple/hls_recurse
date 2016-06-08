int f2(int x, int y);

int f1(int x, int y)
{
	if(x<=1){
		return y;
	}else{
	  return f1(x-1,x+y)+f2(x-2,x*y);
	}

}

int f2(int x, int y)
{
	if(x<=1){
		return y;
	}else{
	  return f1(x-1,x+y-1)+f2(x-2,x*y+1);
	}
}

int main()
{
  return f1(5,10)+f2(8,9);
}

