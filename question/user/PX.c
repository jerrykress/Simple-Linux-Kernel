#include "PX.h"
#include "libc.h"

int iss_prime( uint32_t x ){
    if ( !( x & 1 ) || ( x < 2 ) ) return ( x == 2 );

    for( uint32_t d = 3; ( d * d ) <= x ; d += 2 ) {
        if( !( x % d ) ) return 0;
    }
    return 1;
}


void think(){
    for (int i = 0; i < 25; i++){
        uint32_t lo = 1 << 8;
        uint32_t hi = 1 << 16;

        for (uint32_t x = lo; x < hi; x++){
            int r = iss_prime(x);
        }
    }
}


void eat(){
    print_int(pidd());
    print(":EAT");
}


bool pickup(int id){
    int l = pidd() - 1;
    int r = pidd();
    if(id == 17) r = 1;
    int w1 = mutx(l, 2, 0);
    int w2 = mutx(r, 2, 0);

    if (w1 == MUTX_WRITE_OK && w2 == MUTX_WRITE_OK) return true;
    else return false;
}


void putdown(int id){
    int l = pidd() - 1;
    int r = pidd();
    if(id == 17) r = 1;
    int r1 = mutx(l, 3, 0);
    int r2 = mutx(r, 3, 0);
}

void phil(){
    if (pickup(pidd())){
        eat();
        think();
        putdown(pidd());
        think();
    }else{
        putdown(pidd());
    }  
}


void main_PX(){
    fork();
    fork();
    fork();
    fork(); //pid:2-17

    while(1){
        if (pidd() % 2 == 0){
            phil();    
        }
        
        if (pidd() % 2 == 1){
            phil();
        }
    }
    
    return;
}