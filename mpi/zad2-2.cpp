# include <stdio.h>
# include <string.h>
# include <mpi.h>
const int MAX_STRING = 100;
using namespace std;
int main (void){
	int n=0;
	int local_n;
	int local_a=0;
	int local_b=0;
	int sum=0;
	int local_sum=0;
	int csize;
	int prank;
	MPI_Init ( NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&csize);
	MPI_Comm_rank(MPI_COMM_WORLD,&prank);
	
	if(prank==0)
	{
		while(n<csize)
		{
			cout<<"N=";
			cin>>n;

		}
		
	}
	MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

	local_n=n/csize;
	local_a=prank*local_n+1;
	
	if(n%csize!=0)
		if(prank==csize-1)
		{
			local_n=n-(csize-1)*local_n;
		}
	local_b=local_a+local_n-1;

	for(int i=local_a;i<=local_b;i++)
	{
		local_sum+=i;
	}
	cout<<prank<<","<<local_sum<<",local_sum"<<endl;
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Reduce(&local_sum,&sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	if(prank==0)
		cout<<"Sum of "<<n<<" numbers is "<<sum<<endl;
	MPI_Finalize();
	return 0;
}
