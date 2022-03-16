#include <iostream>
#include <unistd.h>

//#include "include/ThreadPool.h"
#include "ThreadPool.hpp"

void test_run(int id){
    for(int i=1;i<=10;i++){
        printf("id=%d,times=%d\n",id,i);
        usleep(1000000);
    }
}

int main() {
    std::cout << "Hello, World!" << std::endl;

    tp::ThreadPool threadPool(4);

    for(int i=1;i<=10;i++){
        threadPool.enqueue([i]{
            test_run(i);
        });
    }
    while(true){
        sleep(10000);
    }
    return 0;
}
