/* Program to demonstrate time taken by function fun() */
#include <stdio.h> 
#include <time.h> 

// A function that terminates when enter key is pressed 
void fun() 
{ 
    printf("fun() starts \n"); 
    printf("Press enter to stop fun \n"); 
    while(1) 
    { 
        if (getchar()) 
            break; 
    } 
    printf("fun() ends \n"); 
} 

// The main program calls fun() and measures time taken by fun() 
int main() 
{ 
    // Calculate the time taken by fun() 
    time_t ini, fim; 

    time(&ini); 
    fun();
    time(&fim);
    
    time_t time_taken = fim - ini;

    printf("fun() took %d seconds to execute \n", time_taken); 
    return 0; 
}

