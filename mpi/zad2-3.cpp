# include <stdio.h>
# include <string.h>
# include <mpi.h>
const int MAX_SIZE = 100;
using namespace std;
void printv(int *vector,int size,int prank)
{
	cout<<"Process "<<prank<<": ";
	for(int i=0;i<size;i++)
	{
		cout<<", "<<vector[i];
	}
	cout<<endl;
}
int main (void){
	int n=0;
	int seed=1;
	int local_n;
	int v1[MAX_SIZE];
	int v2[MAX_SIZE];
	int local_a[MAX_SIZE];
	int local_b[MAX_SIZE];
	int sum=0;
	int local_sum=0;
	int csize;
	int prank;
	MPI_Init ( NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&csize);
	MPI_Comm_rank(MPI_COMM_WORLD,&prank);
	
	if(prank==0)
	{
		cout<<"Seed=";
		cin>>seed;
		while(n<csize)
		{
			cout<<"N=";
			cin>>n;

		}
		for(int i=0;i<n;i++)
		{
			srand(seed+i+1);
			v1[i]=rand()%10;
			srand(seed+i+2);
			v2[i]=rand()%10;
		}
		cout<<"v1 ";
		printv(v1,n,prank);
		cout<<"v2 ";
		printv(v2,n,prank);
		
	}
	MPI_Bcast(&n,1,MPI_INT,0,MPI_COMM_WORLD);

	local_n=n/csize;
	MPI_Scatter(v1,local_n,MPI_INT,local_a,local_n,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Scatter(v2,local_n,MPI_INT,local_b,local_n,MPI_INT,0,MPI_COMM_WORLD);

	for(int i=0;i<local_n;i++)
	{
		local_sum+=local_a[i]*local_b[i];
	}
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Reduce(&local_sum,&sum,1,MPI_INT,MPI_SUM,0,MPI_COMM_WORLD);
	if(prank==0)
	{
		if(n%csize!=0)
		{
			for(int i =csize*local_n;i<n;i++)
			{
				sum+=v1[i]*v2[i];
			}
		}
		cout<<"Dot product is "<<sum<<endl;
	}
	MPI_Finalize();
	return 0;
}
