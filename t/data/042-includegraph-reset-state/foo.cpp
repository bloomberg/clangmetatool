#include "foo.h"

void foo(){}

int main(){
   internal_int retry = RETRY_COUNT;
   internal_int count = 3;
   while(count < retry){
     foo();
     count++;
   }
   return 0;
}
