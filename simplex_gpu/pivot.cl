__kernel void pivot(
    const int pivotRow,
	const int ROWSIZE,
	const int COLSIZE,
    __global float* newRow,
    __global float* pivotColVal,
    __global float* wv)
{
    int i;
    int j = get_global_id(0);
    //printf("j=%d\n",j);
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
