typedef struct {  
  int c1p3;        
  int c2;        
  int c3;        
  int c4;        
  int fk_p1_p3;        
  int fk_p2_p3;        
} Table3Type;        
typedef struct {  
  int c1p2;        
  int c2;        
  int c3;        
  int fk_p1_p2;        
} Table2Type;   


__kernel void komplex1Kernel(                             
                  __global Table3Type *T1,          
                  __global Table2Type *T2,          
                  __global Table3Type *B,          
                   __global int *counter)   
{                                          

int i, j, k;
int index = get_global_id(0);
counter[index]=0;

  for(i=0; i<2; i++)
  {
    k=index*2 +i;
    if(T2[ k].c2>=4 )
    {

      for(j=0; j<50000; j++)
      {
        if( T1[j].fk_p2_p3 == T2[k].c1p2 & T1[j].c3>=5488)
        {
          B[  index + counter[index]*100 ] = T1[j];
          counter[index]+=1;
        }
      }
    }
  }

} 
