#define __CL_ENABLE_EXCEPTIONS
#include "matrix.hpp"
#include "cl_util.hpp"
#include <omp.h>
#include <CL/cl.hpp>
#include <iostream>
#include <fstream>
#define NUM 1600
#define NUMOFVAR NUM
#define NUMOFSLACK NUM
#define ROWSIZE (NUMOFSLACK+1)
#define COLSIZE (NUMOFSLACK+NUMOFVAR+1)
//#define PERIOD 1
using namespace std;
float var[2*NUMOFVAR];
float optim[2];
int count[2]={0,0};
double start, elapsed1,elapsed2;
void print_status_msg(const matrix::mat&, const matrix::mat&, const matrix::mat&, const matrix::mat&, double);
float wv[ROWSIZE*COLSIZE];
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
    myFile.open("../simplex_input_generator/baza1600.txt",ios::in); //open in read mode
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

        float minpozval=99999999;
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
	//start = omp_get_wtime();
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
        //elapsed1  += omp_get_wtime() - start;
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
    #ifdef PERIOD
    	for(int i=0;i<PERIOD;i++)
    #else
    	while(!checkOptimality(wv))
    #endif
    {
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
	cl::Buffer b_newRow, b_pivotColVal, b_wv;

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

	  start = omp_get_wtime();

	  makeMatrix(wv);
	  //print(wv);
	  simplexCalculate(wv);
	  elapsed1  = omp_get_wtime() - start;
	  cout<<"Time elapsed = "<<elapsed1<<endl;
	  

	  /*
	   * Init buffers
	   */

	  b_newRow = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * COLSIZE);
    	b_pivotColVal = cl::Buffer(context, CL_MEM_READ_ONLY, sizeof(float) * ROWSIZE);
    b_wv = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(float) * ROWSIZE*COLSIZE);

	 

	  /*
	   * OpenCL row on work item
	   */

	  cl::Program prog(context, cl_util::load_prog("pivot.cl"), true);
	  cl::make_kernel<int, int, int, cl::Buffer, cl::Buffer, cl::Buffer> pivoting(prog, "pivot");

	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- OpenCL row on work item                                --" << endl;
	  cout << "------------------------------------------------------------" << endl;

	  //c.zero();

	 start = omp_get_wtime();
	 makeMatrix(wv);
	//  mul1(cl::EnqueueArgs(queue, cl::NDRange(c.get_row())),pivot, d1, d2, d_a, d_b, d_c);

	//  queue.finish();
	int pivotRow;
    int pivotCol;
    bool unbounded=false;
    float pivot;

    //float solVar[NUMOFVAR];

    #ifdef PERIOD
    	for(int i=0;i<PERIOD;i++)
    #else
    	while(!checkOptimality(wv))
    #endif
    {
    	count[1]++;
        pivotCol=findPivotCol(wv);

        if(isUnbounded(wv,pivotCol))
        {
            unbounded=true;
            break;
        }


        pivotRow=findPivotRow(wv,pivotCol);

        pivot=wv[pivotRow*COLSIZE+pivotCol];

    	
        //doPivoting(wv,pivotRow,pivotCol,pivot,queue);
	float newRow[COLSIZE];
	float pivotColVal[ROWSIZE];
    for(int i=0;i<COLSIZE;i++)
        {
            newRow[i]=wv[pivotRow*COLSIZE+i]/pivot;
        }

        for(int j=0;j<ROWSIZE;j++)
        {
            pivotColVal[j]=wv[j*COLSIZE+pivotCol];
        }
	////gpu part
	//print(wv);
	
	queue.enqueueWriteBuffer(b_newRow,CL_TRUE,0,sizeof(float) * COLSIZE,newRow);
	queue.enqueueWriteBuffer(b_pivotColVal,CL_TRUE,0,sizeof(float) * ROWSIZE,pivotColVal);
	queue.enqueueWriteBuffer(b_wv,CL_TRUE,0,sizeof(float) * ROWSIZE*COLSIZE,wv);
	//start = omp_get_wtime();
	pivoting(cl::EnqueueArgs(queue, cl::NDRange(ROWSIZE)),
	   	   pivotRow, ROWSIZE, COLSIZE, b_newRow, b_pivotColVal, b_wv);
	//elapsed2  += omp_get_wtime() - start;
	queue.finish();
	
	cl::copy(queue, b_wv, wv, wv+(ROWSIZE-1)*COLSIZE+(COLSIZE));
	
	
	//print(wv);
	//while(1);
    }
    
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
	elapsed2  = omp_get_wtime() - start;
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
	  /*
	   * OpenCL row on work item, A in privete memory
	   */
/*
	  prog = cl::Program(context, cl_util::load_prog("mul2.cl"), true);
	  cl::make_kernel<int, int, int, cl::Buffer, cl::Buffer, cl::Buffer> mul2(prog, "mul2");

	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- OpenCL row on work item, A in privete memory           --" << endl;
	  cout << "------------------------------------------------------------" << endl;

	  c.zero();

	  start = omp_get_wtime();

	  mul2(cl::EnqueueArgs(queue, cl::NDRange(c.get_row()), cl::NDRange(c.get_row() / winum)),
		   d0, d1, d2, d_a, d_b, d_c);

	  queue.finish();

	  elapsed  = omp_get_wtime() - start;

	  cl::copy(queue, d_c, c.begin(), c.end());

	  print_status_msg(a, b, c, gold, elapsed);

	  /*
	   * OpenCL row on work item, A row privete, B col local
	   */
/*
	  prog = cl::Program(context, cl_util::load_prog("mul3.cl"), true);
	  cl::make_kernel<int, int, int, cl::Buffer, cl::Buffer, cl::Buffer, cl::LocalSpaceArg> mul3(prog, "mul3");

	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- OpenCL row on work item, A row privete, B col local    --" << endl;
	  cout << "------------------------------------------------------------" << endl;

	  c.zero();

	  start = omp_get_wtime();

	  cl::LocalSpaceArg localmem = cl::Local(sizeof(float) * a.get_col());

	  mul3(cl::EnqueueArgs(queue, cl::NDRange(c.get_row()), cl::NDRange (c.get_row() / winum)),
		   d0, d1, d2, d_a, d_b, d_c, localmem);

	  queue.finish();

	  elapsed  = omp_get_wtime() - start;

	  cl::copy(queue, d_c, c.begin(), c.end());

	  print_status_msg(a, b, c, gold, elapsed);

	  return 0;

	  /*
	   * Blocked algorithm
	   */
/*
	  prog = cl::Program(context, cl_util::load_prog("mul4.cl"), true);
	  cl::make_kernel<int, int, int, cl::Buffer, cl::Buffer, cl::Buffer, cl::LocalSpaceArg, cl::LocalSpaceArg> mul4(prog, "mul4");

	  cout << "------------------------------------------------------------" << endl;
	  cout << "-- Blocked algorithm                                      --" << endl;
	  cout << "------------------------------------------------------------" << endl;
	  c.zero();

	  start = omp_get_wtime();

	  // Work-group computes a block of C.  This size is also set
	  // in a #define inside the kernel function.  Note this blocksize
	  // must evenly divide the matrix order
	  int blocksize = 16;

	  cl::LocalSpaceArg A_block = cl::Local(sizeof(float) * blocksize*blocksize);
	  cl::LocalSpaceArg B_block = cl::Local(sizeof(float) * blocksize*blocksize);

	  mul4(
		  cl::EnqueueArgs(
			  queue,
			  cl::NDRange(N,N),
			  cl::NDRange(blocksize,blocksize)),
		  d0,
		  d1,
		  d2,
		  d_a,
		  d_b,
		  d_c,
		  A_block,
		  B_block);

	  queue.finish();

	  elapsed = omp_get_wtime() - start;

	  cl::copy(queue, d_c, h_c.begin(), h_c.end());

	  print_status_msg(a, b, c, gold, elapsed);*/
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
