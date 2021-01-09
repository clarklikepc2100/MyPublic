#include<stdio.h>
#include<string.h>
#include<mpi.h>
#include<math.h>
double f(double x)
{
    double y;
	y=4/(1+pow(x,2.0)); 
	return y;
}
double Trap(double left_endpt,double right_endpt,int trap_count,double base_len)
{
    double estimate,x;
    estimate=(f(left_endpt)+f(right_endpt))/2.0;
    for(int i=1;i<trap_count;i++)
    {
        x=left_endpt+i*base_len;
        estimate+=f(x);
    }
    estimate=estimate*base_len;
    return estimate;
}
int main(int argc,char **argv)
{
    int numProcs,rank;
    double a,b,n;
    MPI_Status status;
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numProcs);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    if(rank==0)
    {
        printf("Please input a,b,n\n");
        scanf("%lf%lf%lf",&a,&b,&n);
        for(int i=1;i<numProcs;i++)
        {
            MPI_Send(&a,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
            MPI_Send(&b,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
            MPI_Send(&n,1,MPI_DOUBLE,i,0,MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&a,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&b,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        MPI_Recv(&n,1,MPI_DOUBLE,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    }
    double h,local_n;
    h=(b-a)/n;
    local_n=n/numProcs;
    double get_trap[numProcs];
    double local_a=a+rank*local_n*h;
    double local_b=local_a+local_n*h;
    double local_trap=Trap(local_a,local_b,local_n,h);
    printf("local trap is :%lf\n",local_trap);
    double total_trap;

    printf("Begin MPI_Reduce\n");
    MPI_Reduce(&local_trap,&total_trap,1,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
    //MPI_Gather(&local_trap,1,MPI_DOUBLE,get_trap,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    printf("Begin MPI_Gather\n");
    MPI_Gather(&local_trap,1,MPI_DOUBLE,get_trap,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    
    if(rank!=0){
      MPI_Send(&local_trap,1,MPI_DOUBLE,0,99,MPI_COMM_WORLD);
      //printf("Begin MPI_Send\n");
      printf("Proc %d of %d > Does anyone have a toothpick?\n",rank, numProcs);
    }
    //MPI_Send(&total_trap,1,MPI_DOUBLE,0,99,MPI_COMM_WORLD);
    else
    {    
        //Reduce
        printf("Use Reduce:Integral from %.0f to %.0f = %lf\n",a,b,total_trap);
        
	//Gather Begin
	total_trap=0;
        for(int i=0;i<numProcs;i++)
            total_trap+=get_trap[i];
        printf("Use Gather:Integral from %.0f to %.0f = %lf\n",a,b,total_trap);
        //Gather End

        //Send-Recv Begin
        total_trap=local_trap;
        for(int i=1;i<numProcs;i++)
        {
            //MPI_Probe(MPI_ANY_SOURCE,99,MPI_COMM_WORLD,&status);
            //MPI_Recv(get_trap+status.MPI_SOURCE,1,MPI_DOUBLE,status.MPI_SOURCE,99,MPI_COMM_WORLD,&status);
            MPI_Recv(&local_trap,1,MPI_DOUBLE,i,99,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
	}
        //for(int i=0;i<numProcs;i++)
        //    total_trap+=get_trap[i];
        printf("Use Send:Integral from %.0f to %.0f = %lf\n",a,b,total_trap);
        //Send-Recv End
    }
    MPI_Finalize();
    return 0;
       
}

