#include <stdio.h>
#include "bitstream.h"

int main(){

  // DPRC registers
  unsigned int *control_pointer=(unsigned int *)0x80000e00;
  unsigned int *address_pointer=(unsigned int*)0x80000e04;
  unsigned int *status_pointer=(unsigned int*)0x80000e08;
  unsigned int *timer_pointer=(unsigned int*)0x80000e0C;
  unsigned int *reset_pointer=(unsigned int*)0x80000e10;

  // FIR registers
  unsigned int *control_pointer1=(unsigned int *)0x80000d00;
  unsigned int *address_pointer1=(unsigned int*)0x80000d04;
  unsigned int *outaddr_pointer1=(unsigned int*)0x80000d08;
  unsigned int *timer_pointer1=(unsigned int*)0x80000d0c;
  
  int i;
  unsigned int input_data[100], output_data[91];

  // Initialize FIR input data and I/O registers
  for (i=0;i<100;i++) input_data[i]=i;
  *address_pointer1=input_data;
  *outaddr_pointer1=output_data;

  printf("Starting test...\n");
  printf("Fast FIR filter results...\n");
  // Trigger FIR DMA for reading input data from the memory
  *control_pointer1=1;
 
  // Wait until FIR DMA finishes writing output data
  while(*control_pointer1 != 2){}
  // Display output results and FIR execution time
  for (i=0;i<91;i++)  printf("%d ",output_data[i]); 
  printf("\nElapsed clock cycles: %d\n", *timer_pointer1);

  // Set DPRC registers, trigger first reconfiguration, and wait until it completes
  *address_pointer=(unsigned int)&bitstream1;
  *reset_pointer=1;
  *control_pointer=sizeof(bitstream1)/sizeof(unsigned int);
  while((*status_pointer!=0xF) && (*status_pointer!=0x8) && (*status_pointer!=0x1)){}
  printf("Partial Reconfiguration ended...Status:%d\n",*status_pointer);
  
  // If reconfiguration ended successfully, trigger FIR module and diplay results
  if (*status_pointer==15){
    printf("Slow FIR filter results...\n");
   *control_pointer1=1; 
   while(*control_pointer1 != 2){} 
   for (i=0;i<91;i++) printf("%d ",output_data[i]);
   printf("\nElapsed clock cycles: %d\n", *timer_pointer1);
  }

  // Set DPRC registers, trigger second reconfiguration, and wait until it completes
  *address_pointer=(unsigned int)&bitstream0; 
  *control_pointer=sizeof(bitstream0)/sizeof(unsigned int); 
  while((*status_pointer!=0xF) && (*status_pointer!=0x8) && (*status_pointer!=0x1)){} 
  printf("Partial Reconfiguration ended...Status:%d\n",*status_pointer);

  // If reconfiguration ended successfully, trigger FIR module and diplay results
  if (*status_pointer==15){
    printf("Fast FIR filter results...\n");
    *control_pointer1=1;
    while(*control_pointer1 != 2){}
    for (i=0;i<91;i++) printf("%d ",output_data[i]);
    printf("\nElapsed clock cycles: %d\n", *timer_pointer1);
  }

}




