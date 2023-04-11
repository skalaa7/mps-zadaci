#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <omp.h>
using namespace std;
int main(int argc, char * argv[])
{
	int tc=strtol(argv[1],NULL,10);
	int n=10;
	long int sum =0;
	printf("Number: ");
	scanf("%d",&n);
	int local_n;
	int local_a;
	int local_b;
	long int local_sum;
	int trank;
	int i;
	double s=omp_get_wtime();
	#pragma omp parallel num_threads(tc) private(local_n,local_a,local_b,local_sum,i,trank)
	{
		trank=omp_get_thread_num();
		local_sum=0;
		local_n=n/tc;
		local_a=trank*local_n+1;
		if(n%tc!=0)
			if(trank==tc-1)
			{
				local_n=n-(tc-1)*local_n;
			}
		local_b=local_a+local_n-1;
		for(i=local_a;i<=local_b;i++)
		{
			local_sum+=i;
		}
		#pragma omp barrier

		#pragma omp atomic
			sum+=local_sum;
	}
	
	s=omp_get_wtime()-s;
	printf("Sum of %d numbers is %ld, time=%lfms\n",n,sum,s*1000);
	
	return 0;
}
