#include <chrono>

using namespace std;

using namespace std::chrono;

class Timer
{
public:
    void start()
    {
        StartTime = high_resolution_clock::now();
        bRunning = true;
    }
    
    void stop() 
    {
        EndTime = high_resolution_clock::now();
        bRunning = false;
    }
    
    long elapsedMicroseconds()
    {
        
        if(bRunning)
        {
            stop();
        }

        return duration_cast<microseconds>(EndTime - StartTime).count();
    }

    
    double elapsedSeconds()
    {
        return elapsedMicroseconds() / 1000000.0;
    }

private:
    high_resolution_clock::time_point StartTime;
    high_resolution_clock::time_point EndTime;
    bool                              bRunning = false;
};