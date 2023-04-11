#define __CL_ENABLE_EXCEPTIONS
#include "matrix.hpp"
#include "cl_util.hpp"
#include <omp.h>
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#define NUM 1000
#define NUMOFVAR NUM
#define NUMOFSLACK NUM
#define ROWSIZE (NUMOFSLACK+1)
#define COLSIZE (NUMOFSLACK+NUMOFVAR+1)
//#define FOR_PERIOD 1
using namespace std;
float var[2*NUMOFVAR];
float optim[2];
int count[2]={0,0};
void print_status_msg(const matrix::mat&, const matrix::mat&, const matrix::mat&, const matrix::mat&, double);
float wv[ROWSIZE*COLSIZE];
double start1,start2, elapsed1,elapsed2;
void print(float wv[ROWSIZE*COLSIZE])
{
    for(int j=0;j<ROWSIZE;j++)
        {
            for(int i=0;i<COLSIZE;i++)
            {
                cout<<wv[j*COLSIZE+i]<<" ";
            }
            cout<<endl;
        }
        cout<<endl<<endl<<endl;
}
bool checkOptimality(float wv[ROWSIZE*COLSIZE])
{
    for(int i=0;i<COLSIZE-1;i++)
    {
        if(wv[(ROWSIZE-1)*COLSIZE+i]<0)
            return false;
    }
    return true;
}
bool isUnbounded(float wv[ROWSIZE*COLSIZE],int pivotCol)
{
    for(int j=0;j<ROWSIZE-1;j++)
    {
        if(wv[j*COLSIZE+pivotCol]>0)
            return false;
    }
    return true;
}
void makeMatrix(float wv[ROWSIZE*COLSIZE])
{
	for(int j=0;j<ROWSIZE; j++)
	{
		for(int i =0;i<COLSIZE;i++)
		{
			wv[j*COLSIZE+i]=0;
		}
	}
	fstream myFile;
    myFile.open("../simplex_input_generator/baza1000.txt",ios::in); //open in read mode
	if(myFile.is_open())
    {
        for(int j = 0; j < ROWSIZE; j++)
        {
            for(int i = 0; i< NUMOFVAR; i++)
            {
              myFile >> wv[j*COLSIZE+i];
            }
        }
		for(int j = 0;j< NUMOFSLACK;j++)
		{
			myFile >> wv[j*COLSIZE+COLSIZE-1];
		}
    }
    myFile.close();
    for(int j=0;j<ROWSIZE-1;j++)
    {
		
	wv[j*COLSIZE+NUMOFVAR+j]=1;
		
    }
}
int findPivotCol(float wv[ROWSIZE*COLSIZE])
{
     float minnegval=wv[(ROWSIZE-1)*COLSIZE+0];
       int loc=0;
        for(int i=1;i<COLSIZE-1;i++)
        {
            if(wv[(ROWSIZE-1)*COLSIZE+i]<minnegval)
            {
                minnegval=wv[(ROWSIZE-1)*COLSIZE+i];
                loc=i;
            }
        }
        return loc;
}
int findPivotRow(float wv[ROWSIZE*COLSIZE],int pivotCol)
{
    float rat[ROWSIZE-1];
    for(int j=0;j<ROWSIZE-1;j++)
        {
            if(wv[j*COLSIZE+pivotCol]>0)
            {
                rat[j]=wv[j*COLSIZE+COLSIZE-1]/wv[j*COLSIZE+pivotCol];
            }
            else
            {
                rat[j]=0;
            }
        }

        double minpozval=99999999999;
        int loc=0;
        for(int j=0;j<ROWSIZE-1;j++)
        {
            if(rat[j]>0)
            {
                if(rat[j]<minpozval)
                {
                    minpozval=rat[j];
                    loc=j;
                }
            }
        }
        return loc;
}

void solutions(float wv[ROWSIZE*COLSIZE],int a)
{
    for(int i=0;i<NUMOFVAR; i++)  //every basic column has the values, get it form B array
     {
        int count0 = 0;
        int index = 0;
        for(int j=0; j<ROWSIZE-1; j++)
        {
            if(wv[j*COLSIZE+i]==0.0)
            {
                count0 = count0+1;
            }
            else if(wv[j*COLSIZE+i]==1)
            {
                index = j;
            }


        }

        if(count0 == ROWSIZE - 2 )
        {
            cout<<"variable"<<i+1<<": "<<wv[index*COLSIZE+COLSIZE-1]<<endl;  //every basic column has the values, get it form B array
            var[a*NUMOFVAR+i]=wv[index*COLSIZE+COLSIZE-1];
        }
        else
        {
            cout<<"variable"<<i+1<<": "<<0<<endl;
            var[a*NUMOFVAR+i]=0;
        }
    }

    cout<<""<<endl;
    cout<<endl<<"Optimal solution is "<<wv[(ROWSIZE-1)*COLSIZE+COLSIZE-1]<<endl;
    optim[a]=wv[(ROWSIZE-1)*COLSIZE+COLSIZE-1];
}
void doPivoting(float wv[ROWSIZE*COLSIZE],int pivotRow,int pivotCol,float pivot)
{
    float newRow[COLSIZE];
    float pivotColVal[ROWSIZE];
    //ds=omp_get_wtime();
    for(int i=0;i<COLSIZE;i++)
        {
            newRow[i]=wv[pivotRow*COLSIZE+i]/pivot;
        }
	//sdiv=sdiv+omp_get_wtime()-ds;
        for(int j=0;j<ROWSIZE;j++)
        {
            pivotColVal[j]=wv[j*COLSIZE+pivotCol];
        }
	//dds=omp_get_wtime();
	//#pragma omp parallel for num_threads(tc) schedule(runtime)
        for(int j=0;j<ROWSIZE;j++)
        {
            if(j==pivotRow)
            {
                for(int i=0;i<COLSIZE;i++)
                {
                    wv[j*COLSIZE+i]=newRow[i];
                }
            }
            else
            {
                for(int i=0;i<COLSIZE;i++)
                {
                    wv[j*COLSIZE+i]=wv[j*COLSIZE+i]-newRow[i]*pivotColVal[j];
                }
            }
        }
       //spivot=spivot+omp_get_wtime()-dds;
}
void simplexCalculate(float wv[ROWSIZE*COLSIZE])
{
	
    //float minnegval;
    //float minpozval;
    //int loc;
    int pivotRow;
    int pivotCol;
    bool unbounded=false;
    float pivot;

    //float solVar[NUMOFVAR];
    #ifdef FOR_PERIOD
    	for(int i =0;i<FOR_PERIOD;i++)
    #else
    while(!checkOptimality(wv))
    #endif
    {	
    //start1 = omp_get_wtime();
    	count[0]++;
        pivotCol=findPivotCol(wv);

        if(isUnbounded(wv,pivotCol))
        {
            unbounded=true;
            break;
        }


        pivotRow=findPivotRow(wv,pivotCol);

        pivot=wv[pivotRow*COLSIZE+pivotCol];

    	
        doPivoting(wv,pivotRow,pivotCol,pivot);
       // print(wv);
	//elapsed1  += omp_get_wtime() - start1;

    }
    //Writing results
    if(unbounded)
    {
        cout<<"Unbounded"<<endl;
    }
    else
    {
        //print(wv);

        solutions(wv,0);

    }
}
int main(int argc, char *argv[])
{
	//const int winum = 16; // Number of workitems per work group
	//int N = 1024;
	

	//vector<float> h_a(N * N), h_b(N * N), h_c(N * N);
	cl::Buffer b_newRow, b_pivotColVal, b_wv,b_opti_unb_pr,b_rat;//,b_unbounded,b_pivotRow;

	try
	{
	  cl_util::simple_env env;
	  env.parse_args(argc, argv);

	  vector<cl::Device>& devices = env.devices;
	  cl::Device& device = env.device;

	  cout << "Using OpenCL device: " << env.get_info() << "\n";

	  cl::Context& context = env.get_context();
	  cl::CommandQueue& queue = env.get_queue();



	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- Sequential - host CPU                                  --" << endl;
	  cout << "------------------------------------------------------------" << endl;

	  start1 = omp_get_wtime();

	  makeMatrix(wv);
	  //print(wv);
	  
	  simplexCalculate(wv);
	 elapsed1  = omp_get_wtime() - start1;
	  cout<<"Time elapsed = "<<elapsed1<<endl;
	  

	  /*
	   * Init buffers
	   */

	  b_newRow = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * COLSIZE);
    	b_pivotColVal = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * ROWSIZE);
    b_wv = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * ROWSIZE*COLSIZE);
    
	b_opti_unb_pr = cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(int)*3);
	//b_rat =cl::Buffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * (ROWSIZE-1));
	//b_unbounded = cl::Buffer(context,CL_MEM_READ_WRITE, sizeof(int)*1);
	//b_pivotRow = cl::Buffer(context,CL_MEM_READ_WRITE,sizeof(int)*1);
	 

	  /*
	   * OpenCL row on work item
	   */
	
	  cl::Program prog_pivot(context, cl_util::load_prog("pivot.cl"), true);
	  
	  cl::make_kernel< int, int, cl::Buffer, cl::Buffer, cl::Buffer,cl::Buffer> pivoting(prog_pivot, "pivot");
	  
	  cl::Program prog_simplex_calc(context, cl_util::load_prog("simplex_calc.cl"), true);
	 
	  cl::make_kernel< int, int, cl::Buffer, cl::Buffer, cl::Buffer,cl::Buffer> simplex_calc(prog_simplex_calc, "simplex_calc");
	  
	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- OpenCL row on work item                                --" << endl;
	  cout << "------------------------------------------------------------" << endl;

	  //c.zero();

	 start2 = omp_get_wtime();
	 makeMatrix(wv);
	
	int opti_unb_pr[3]={0,0,0};
    	
	queue.enqueueWriteBuffer(b_wv,CL_TRUE,0,sizeof(float) * ROWSIZE*COLSIZE,wv);
    #ifdef FOR_PERIOD
    	for(int i =0;i<FOR_PERIOD;i++)
    #else
    while(1)
    #endif
    {	
    	//start2 = omp_get_wtime();
    	count[1]++;
    	simplex_calc(cl::EnqueueArgs(queue, cl::NDRange(1)), ROWSIZE, COLSIZE, b_newRow, b_pivotColVal, b_wv,b_opti_unb_pr);
    	queue.finish();
    	cl::copy(queue, b_opti_unb_pr, opti_unb_pr, opti_unb_pr+2);
    	if(opti_unb_pr[0] || opti_unb_pr[1])
    		break;
    	pivoting(cl::EnqueueArgs(queue, cl::NDRange(ROWSIZE)),ROWSIZE, COLSIZE, b_newRow, b_pivotColVal, b_wv,b_opti_unb_pr);
	queue.finish();
	//elapsed2  += omp_get_wtime() - start2;
    }
    //float solVar[NUMOFVAR];
    cl::copy(queue, b_wv, wv, wv+(ROWSIZE-1)*COLSIZE+(COLSIZE));
    int unbounded=opti_unb_pr[1];
    //
    //Write results
    if(unbounded)
    {
        cout<<"Unbounded"<<endl;
    }
    else
    {
        //print(wv);

        solutions(wv,1);

    }
	elapsed2  = omp_get_wtime() - start2;
	cout << "------------------------------------------------------------" << endl;
	cout << "-- Results                                                --" << endl;
	cout << "------------------------------------------------------------" << endl;
	cout<<"Matrix size:"<<NUMOFVAR<<"x"<<NUMOFSLACK<< endl;
	int equal=1;
	float error=0.001;
	float rel_error=(1.0-optim[1]/optim[0]);
	int var_mismatch_count=0;
	if(abs(rel_error)>error)
		equal=0;
	cout<<"Optimal solutions - seq:"<<optim[0]<<", gpu:"<<optim[1]<<endl;
	cout<<"Relative error:"<<rel_error*100<<"%"<<", error margin = "<<error*100<<"%"<<endl;
	
	/*for(int v=0;v<NUMOFVAR;v++)
	{	//cout<<var[v]<<","<<var[NUMOFVAR+v]<<endl;
		if(abs(1-var[NUMOFVAR+v]/var[v])>0.001)
			var_mismatch_count++;
		{	//cout<<var[v]<<","<<var[NUMOFVAR+v]<<endl; 
		//equal=0;
		}
	}*/
	if(equal)
		cout<<"--Results match--"<<endl;			
	else
		cout<<"Results don't match"<<endl;
	//cout<<"Number of matched variables: "<<NUMOFVAR-var_mismatch_count<<" out of "<<NUMOFVAR<<endl;
	cout<<"Iterations - seq:"<<count[0]<<", gpu:"<<count[1]<<endl;
	cout<<"Time elapsed on gpu = "<<elapsed2<<"s, sequential time = "<<elapsed1<<"s"<<endl; 		cout<<"speedup = seq_time/gpu_time = "<<elapsed1/elapsed2<<endl;
	  
    }
	catch (cl::Error err)
    {
		cout << "Exception" << endl;
		cerr << "ERROR: " << err.what() << endl;
		if (err.err() == CL_BUILD_PROGRAM_FAILURE)
		{
			cout << cl_util::get_build_log("mul3.cl");
		}
    }

	return 0;
}

void print_status_msg(
	const matrix::mat& a,
	const matrix::mat& b,
	const matrix::mat& c0,
	const matrix::mat& c1,
	double elapsed)
{
	if (c0 == c1)
	{
		cout << "Matrices are equal." << endl;
		cout << "Elapsed time : " << elapsed << endl;
		cout << "MFLOPS       : " << ((double)get_mul_ops(a, b, elapsed)) / (1000000.0f * elapsed);
		cout << endl;
	}
	else
	{
		cout << "ERROR: Matrices are not equal!" << endl;
		cout << "-----------------------" << endl;
		cout << c0 << endl;
		cout << "-----------------------" << endl;
		cout << c1 << endl;
	}
}
