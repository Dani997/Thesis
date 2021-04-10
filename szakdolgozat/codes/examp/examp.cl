typedef struct {
  int c1p1;
  int c2;
  int c3;
  int c4;
} Table1Type;
typedef struct {
  int index;
} TableResultType;


#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

__kernel void examp(   __global Table1Type *T1, __global TableResultType *ANS,
                             __global int *Counter, __global int *limit, __global int *Range )
{

int index=get_global_id(0);
int i;
Counter[index] = 0;
int x= index * *Range;

for(i = x ; i < x+ *Range; i++)
{
    if( i< *limit && T1[i].c3 == 1 && T1[i].c4 > 9500)
    {
        ANS[ index* *Range + Counter[index] ].index = i;
        atomic_inc(&Counter[index]);

    }
} 

}
