# include <stdio.h>
# include <string.h>
# include <mpi.h>
const int MAX_STRING = 100;
using namespace std;
int main (void){
	int message[6];
	int load;
	int csize;
	int prank;
	MPI_Init ( NULL,NULL);
	MPI_Comm_size(MPI_COMM_WORLD,&csize);
	MPI_Comm_rank(MPI_COMM_WORLD,&prank);

	srand(prank+1);
	for(int i=0;i<csize;i++)
	{
		load=10*prank+rand()%10;
		MPI_Send(&load,1,MPI_INT,i,0,MPI_COMM_WORLD);
	}
	for(int i=0;i<csize;i++)
	{
		MPI_Recv(message+i,1,MPI_INT,i,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}
	
	
	cout<<"Process "<<prank<<" recieved: ";
	for(int i=0;i<csize;i++)
	{
		cout<<message[i]<<" ";
	}
	cout<<endl;
	MPI_Finalize ();
	return 0;
}
