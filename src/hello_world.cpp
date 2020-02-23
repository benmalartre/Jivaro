#include <iostream>
#include <chrono>
#include <ctime>
#include <functional>

#include "timer.h"

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usd/primrange.h>

int fibonacci(int n)
{
    if (n < 3) return 1;
    return fibonacci(n-1) + fibonacci(n-2);
}

void TestTime()
{
    std::chrono::time_point<std::chrono::system_clock> start, end;
    start = std::chrono::system_clock::now();
    int result = fibonacci(42);
    end = std::chrono::system_clock::now();
 
    uint64_t elapsed_seconds = 
        std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    std::time_t end_time = 
        std::chrono::system_clock::to_time_t(end);
 
    std::cout << " chrono elapsed time: " << elapsed_seconds * 1.e-9 << "s\n";

    uint64_t startT, endT;
    startT = ns();
    result = fibonacci(42);
    endT = ns();
 
    elapsed_seconds = (endT - startT); 
    std::cout << "timer elapsed time: " << elapsed_seconds * 1.e-9 << "s\n";
}
 
class TIMER_TIME
{
public:
    TIMER_TIME( std::function func )
        : m_func( func )

    void operator()()
    {
        uint64_t startT = ns();

        m_func();

        std::cout << "Timer took " << (ns() - startT) * 1e-9 
            << " seconds..." << std::endl;
    }

private:
    std::function m_func;
};

class CHRONO_TIME
{
public:
    CHRONO_TIME( std::function func )
        : m_func( func )

    void operator()()
    {
        uint64_t startT = ns();
        std::chrono::time_point<std::chrono::system_clock> startT, endT;
        startT = std::chrono::system_clock::now();

        m_func();
        endT = std::chrono::system_clock::now();
 
    uint64_t elapsed_seconds = 
        std::chrono::duration_cast<std::chrono::nanoseconds>(end-start).count();
    std::time_t end_time = 
        std::chrono::system_clock::to_time_t(end);
 
    std::cout << "Chrono took " << (endT - startT) * 1e-9 
            << " seconds..." << std::endl;
    }

private:
    std::function m_func;
};

template< typename Func >
CHRONO_TIME<Func> decorate(Func func) {
  return FunctionDecorator<Func>(std::move(func));
}
int main(void)
{
    
    std::string filePath = 
        "/Users/benmalartre/Documents/RnD/USD_BUILD/assets/maneki_anim.usd";
    pxr::UsdStageRefPtr stage = pxr::UsdStage::Open(filePath, pxr::UsdStage::LoadAll);

    pxr::UsdPrimRange range = stage.TraverseAll();

    return 1;
    

  
}