#include <iostream>
#include <chrono>
#include "ThreadPool.h"

using namespace std;

int main()
{
    cout << "========== ThreadPool Test ==========\n\n";

    ThreadPool pool(4);

    for (int i = 1; i <= 20; i++)
    {
        pool.submit([i]()
        {
            cout << "Task " << i
                 << " executed by Thread "
                 << this_thread::get_id()
                 << endl;

            this_thread::sleep_for(chrono::milliseconds(500));
        });
    }

    pool.wait();

    cout << "\nAll tasks completed successfully!\n";

    return 0;
}