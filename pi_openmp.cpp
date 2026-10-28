#include <iostream>
#include <omp.h>
#include <chrono>

using namespace std;
using namespace chrono;

const int N = 100000000;
const int studNum = 431316;
const int blockSize = 10 * studNum;

int main() {
    cout << "Число потоков \t Время\t\t Результат\n";

    int threadsToTest[] = {1, 2, 4, 8, 12, 16};

    for(int i = 0; i < 6; i++) {
        int threadNum = threadsToTest[i];

        omp_set_num_threads(threadNum);

        double totalSum = 0;

        auto start = high_resolution_clock::now();

        #pragma omp parallel for schedule(dynamic, blockSize) reduction(+:totalSum)

        for(int j = 0; j < N; j++){
            double x = (j + 0.5) / N;
            totalSum += 4.0 / (1.0 + x * x);
        }

        auto end = high_resolution_clock::now();
        double timeSpent = duration_cast<duration<double>>(end - start).count();

        double pi = totalSum / N;
        cout << threadNum << "\t\t" << timeSpent << "\t" << pi << endl;

    }
    return 0;
}