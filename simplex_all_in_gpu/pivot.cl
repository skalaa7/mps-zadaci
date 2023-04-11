__kernel void pivot(
	const int ROWSIZE,
	const int COLSIZE,
    __global float* newRow,
    __global float* pivotColVal,
    __global float* wv,
    __global int* opti_unb_pr
    )
{
    int i;
    int j = get_global_id(0);
    //printf("j=%d\n",j);
    int pivotRow=opti_unb_pr[2];
    if(j<ROWSIZE)
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
                //printf("%d,%d,%f",j,i,wv[j*COLSIZE+i]);
            }
        }
    }
    //printf("\n");
}
