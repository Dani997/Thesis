typedef struct {
  int c1p1;
  int c3;
  int fk_s;
} TableMType;

typedef struct {
  int c1p1;
  int c3;
} TableSType;


typedef struct {
  int M_index;
  int S_index;
  int MC3xSC3;
} TableOutType;


#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable

__kernel void examp(   __global TableMType *TM,  __global int *ANS,
                             __global int *Counter, __constant int *limit, __constant int *Range )
{

int index=get_global_id(0);


 
int i;
int ans_index;
Counter[index] = 0;
int x= index * *Range;

for(i = x ; i < x+ *Range; i++)
{

  if( i < *limit && TM[i].c3>9900)
  {

          ans_index = x + Counter[index];
          ANS[ ans_index ] = i;
          atomic_inc(&Counter[index]);
        
  }

} 
}

/*
//////////          /////////
          /////////           //////////
//////////          /////////
          /////////           ////////// 
//////////          /////////
          /////////           //////////    */               

__kernel void examp2(   __global TableSType *TS,  __global int *ANS,
                             __global int *Counter, __global int *limit, __constant int *Range )
{

int index=get_global_id(0);

int i;
int ans_index;
Counter[index] = 0;
int x= index * *Range;

for(i = x ; i < x+ *Range; i++)
{

  if( i < *limit && TS[i].c3>9900)
  {

          ans_index  = x + Counter[index];
          ANS[ ans_index ] = i;
          atomic_inc(&Counter[index]);
  }

} 
}

/*
//////////          /////////
          /////////           //////////
//////////          /////////
          /////////           ////////// 
//////////          /////////
          /////////           //////////    */           


__kernel void exampfinal(   __global TableMType *TM,
                            __global int *M_Out,
                            __global int *CounterM,
                            __constant int *TM_interval_size,

                            __global TableSType *TS,  
                            __global int *S_Out,
                            __global int *CounterS,
                            __constant int *TS_interval_size,
                            __constant int *TS_global, 
                            
                            __global TableOutType *Final_Resoult,
                            __global int *CounterFinal)
{

  int i = get_global_id(0);


CounterFinal[ i ]=0;
int j,k,l;
int M_start, S_start;

M_start = i * *TM_interval_size; 
for( j = 0; j < CounterM[i]; j++ )
{
  for( k = 0; k < *TS_global; k++ )
  {
    S_start = k * *TS_interval_size;
    for( l = 0; l < CounterS[k]; l++ )
    {
      
      if( TM[ M_Out[ M_start + j ] ].fk_s == TS[ S_Out[ S_start + l ]].c1p1 )
      {
        
        Final_Resoult[ M_start + CounterFinal[i] ].M_index = TM[ M_Out[ M_start + j ] ].fk_s;
        Final_Resoult[ M_start + CounterFinal[i] ].S_index = TS[ S_Out[ S_start + l ]].c1p1;
        Final_Resoult[ M_start + CounterFinal[i] ].MC3xSC3 =  TM[ M_Out[ M_start + j ] ].c3 * TS[ S_Out[ S_start + l ]].c3;
        CounterFinal[ i ]++;
        break;
      }

    }
  }
}
//printf("talalt: %d \n", CounterFinal[i]);
}