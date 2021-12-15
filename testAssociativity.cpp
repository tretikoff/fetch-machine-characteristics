#include <chrono>
#include <vector>
#include <set>
#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>


long S = 2, cur_time, prev_time, jump;
const int MB = 1024 * 1024;
long maxAssoc = 65;
const int L1SizeBound = 512;
const size_t repeatNum = 1e5;
const size_t cacheBounds[] = {512, 4096, 65536};


inline void loopRunner(int ** data, size_t assoc) {
    int **x = (int **) &data[0];
    for (size_t i = 0; i < repeatNum; ++i) {
        auto assocWas = assoc;
        while (assocWas-- > 0) {
            x = (int **)*(x);
        }
    }
}





/**
 * Determine associativity for particular cache size
 * @param levelSizeBound - max for curr cache level
 * @param stride -  step for current cache
 */
std::vector<unsigned long> traverseAssociativityLevel(long levelSizeBound, size_t stride, size_t startAssoc) {
    int** data = new int*[levelSizeBound];
    auto end = (startAssoc) * stride;
    // Create the cyclic chain of references
    for (auto i = stride; i <= end; i += stride) {
        data[i] = (int*)&data[i - stride];
    }
    data[0] = (int*)&data[end];

    auto startT = std::chrono::high_resolution_clock::now();
    auto endT = std::chrono::high_resolution_clock::now();

    std::vector<unsigned long> timeProbes = std::vector<unsigned long>();
    timeProbes.push_back(1);
    // essential
    int **x = (int **) &data[0];
    for (size_t i = 0; i < 2; ++i) {
        x = (int **)*(x);
    }

    // run for all potential Assoc
    for (size_t targetElems = startAssoc; targetElems < maxAssoc; ++targetElems) {
        startT = std::chrono::high_resolution_clock::now();
        loopRunner(data, targetElems);
        endT = std::chrono::high_resolution_clock::now();

        auto t = (endT - startT).count() / repeatNum;
        unsigned long prevT = timeProbes.back();
        timeProbes.push_back(t);

        //if (prevT != 1 && t / prevT > 1.7) {
        //    std::cout << "Potential accos is " << targetElems - 1 << '\n';
        //    break;
        //}
    }


    delete [] data;
    return timeProbes;
}



int main() {
    size_t startA = 10; // looks ok?
    auto measures = traverseAssociativityLevel(cacheBounds[1] * 1024, 2 * 1024, startA);
    //auto measures = traverseAssociativityLevel(cacheBounds[1] * 1024, 256 * 1024);
    std::cout << measures.size() << '\n';
    size_t c = startA;
    for (const auto &item : measures) {
        std::cout << c++ << ',' << item << ' ';
    }


}