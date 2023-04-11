__kernel void simplex_calc(
	const int ROWSIZE,
	const int COLSIZE,
    __global float* newRow,
    __global float* pivotColVal,
    __global float* wv,
    __global int* opti_unb_pr//,
    //__global int unbounded,
    //__global int pivotRow
    )
{
    int i;
    int j = get_global_id(0);
    int pivotCol;
    float pivot;
    float rat[10000];
    if(j<1)
    {
    	opti_unb_pr[0]=1;//optimal
	//checkOptimality
	for(int i=0;i<COLSIZE-1;i++)
    	{
        	if(wv[(ROWSIZE-1)*COLSIZE+i]<0)
            		opti_unb_pr[0]=0;;
    	}
    	
	//findPivotCol
	float minnegval=wv[(ROWSIZE-1)*COLSIZE+0];
       pivotCol=0;
        for(int i=1;i<COLSIZE-1;i++)
        {
            if(wv[(ROWSIZE-1)*COLSIZE+i]<minnegval)
            {
                minnegval=wv[(ROWSIZE-1)*COLSIZE+i];
                pivotCol=i;
            }
        }
	//isUnbounded
	opti_unb_pr[1]=1;//unbounded
	for(int j=0;j<ROWSIZE-1;j++)
    	{
        	if(wv[j*COLSIZE+pivotCol]>0)
            		opti_unb_pr[1]=0;
    	}
	//findPivotRow
	
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

        double minpozval=99999999;
        opti_unb_pr[2]=0;
        for(int j=0;j<ROWSIZE-1;j++)
        {
            if(rat[j]>0)
            {
                if(rat[j]<minpozval)
                {
                    minpozval=rat[j];
                    opti_unb_pr[2]=j;
                }
            }
        }
        int pivotRow=opti_unb_pr[2];
        pivot=wv[pivotRow*COLSIZE+pivotCol];
	//makeNewRow
	for(int i=0;i<COLSIZE;i++)
        {
            newRow[i]=wv[pivotRow*COLSIZE+i]/pivot;
        }
	//makePivotColVal
	for(int j=0;j<ROWSIZE;j++)
        {
            pivotColVal[j]=wv[j*COLSIZE+pivotCol];
        }
	
    }
    //printf("\n");
}
