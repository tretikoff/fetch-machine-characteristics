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
const size_t cacheBounds[] = {512, 4096, 32768};


inline void loopRunner(int ** data, size_t assoc) {
    int **x = (int **) &data[0];
    for (size_t i = 0; i < repeatNum; ++i) {
        auto assocWas = assoc;
        while (assocWas-- > 0) {
            x = (int **)*(x);
        }
    }
}

inline void loopRunner(int* data, size_t size, size_t assoc) {
    auto val = data[0];
    for (size_t i = 0; i < repeatNum; ++i) {
        auto assocWas = assoc;
        while (assocWas-- > 0) {
            val = data[(rand() + val) % size];
        }
    }
}

int* prepareArray(long size, size_t cacheLineSize) {
    auto* buf = new int[size]{0};

    size_t chainedInd = 0;
    size_t i = 1;
    while (i++ < size) {
        buf[chainedInd] = size;
        size_t nextStop = (cacheLineSize * rand()) % size;
        while (buf[nextStop] > 0) {
            nextStop = (1 + nextStop) % size;
        }
        buf[chainedInd] = nextStop;
        chainedInd = nextStop;
    }
    return buf;
}



/**
 * Determine associativity for particular cache size
 * @param levelSizeBound - max for curr cache level
 * @param stride -  step for current cache
 */
std::vector<unsigned long> traverseAssociativityLevel(long levelSizeBound, size_t stride, size_t startAssoc) {
    //int** data = new int*[levelSizeBound];
    //auto end = (startAssoc) * stride;
    //// Create the cyclic chain of references
    //for (auto i = stride; i <= end; i += stride) {
    //    data[i] = (int*)&data[i - stride];
    //}
    //data[0] = (int*)&data[end];
    int* data = prepareArray(levelSizeBound, 64);

    auto startT = std::chrono::high_resolution_clock::now();
    auto endT = std::chrono::high_resolution_clock::now();

    std::vector<unsigned long> timeProbes = std::vector<unsigned long>();
    timeProbes.push_back(1);
    // essential
    //int **x = (int **) &data[0];
    //for (size_t i = 0; i < 2; ++i) {
    //    x = (int **)*(x);
    //}
    int v = 0;
    for (size_t i = 0; i < 2; ++i) {
        v = data[v];
    }

    // run for all potential Assoc
    for (size_t targetElems = startAssoc; targetElems < maxAssoc; ++targetElems) {
        startT = std::chrono::high_resolution_clock::now();
        loopRunner(data, levelSizeBound, targetElems);
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
    std::vector<int> levels = {1, 2, 3};
    //for (int i = 1; i < 32; ++i) {
    //    std::cout << "Assoc: " << i << '\n';
    //    size_t startA = 10; // looks ok?
    //    auto measures = traverseAssociativityLevel(cacheBounds[1] * 1024, 2 * 1024, startA);
    //    //auto measures = traverseAssociativityLevel(cacheBounds[1] * 1024, 256 * 1024);
    //    std::cout << measures.size() << '\n';
    //    size_t c = startA;
    //    for (const auto &item : measures) {
    //        std::cout << c++ << ',' << item << ' ';
    //    }
    //    std::cout << '\n';
    //}
    for (const auto &item : levels) {
        size_t startA = item == 1 ? 1 : 4 * (item - 1); // looks ok?
        std::cout << "Level: " << item << ". Start assoc: " << startA << '\n';
        auto measures = traverseAssociativityLevel(cacheBounds[item - 1] * 1024, 2 * 1024, startA);
        std::cout << measures.size() << '\n';
        size_t c = startA;
        for (const auto &item : measures) {
            std::cout << c++ << ',' << item << ' ';
        }
        std::cout << '\n';
    }


}