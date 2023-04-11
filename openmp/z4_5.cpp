#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
using namespace std;
int main(int argc, char * argv[])
{
	int tc=strtol(argv[1],NULL,10);
	int n=1000000;
	printf("Number: ");
	//std::cin>>n;
	scanf("%d",&n);
	int prost[n-1];
	for(int i=2;i<=n;i++)
	{
		prost[i]=i;
		
	}
	int trank;
	int i;
	int j;
	double s = omp_get_wtime();
	#pragma omp parallel for num_threads(tc) private(i,j) schedule(runtime)
	for(i=2;i<=n;i++)
	{
		for(j=i+1;j<=n;j++)
		{
			if(j%i==0)
				prost[j]=0;
		}
	}
	s=omp_get_wtime()-s;
	printf("Time %lfms\nPrime numbers are",s*1000);
		
	return 0;
}
