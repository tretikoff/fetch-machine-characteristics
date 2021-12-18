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
const size_t cacheBounds[] = {512, 4096, 16384};


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

inline float getMaxDiff(size_t level) {
    switch (level) {
        case 1:
            return 1.69;
        case 2:
            return 1.34;
        case 3:
            return 90;
        default:
             return 1.7;
    }
}

inline bool checkTimeDifference(long lhs, long rhs, float diff) {
    if (diff == getMaxDiff(3)) {
        return lhs - rhs > diff;
    }
    return ((lhs + 0.1) / rhs) > diff;
}


/**
 * Determine associativity for particular cache size
 * @param levelSizeBound - max for curr cache level
 * @param stride -  step for current cache
 */
std::vector<unsigned long> traverseAssociativityLevel(long levelSizeBound, size_t stride, size_t startAssoc, size_t targetLevel) {
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
    float maxDiff = getMaxDiff(targetLevel);
    // run for all potential Assoc
    for (size_t targetElems = startAssoc; targetElems < maxAssoc; ++targetElems) {
        startT = std::chrono::high_resolution_clock::now();
        loopRunner(data, levelSizeBound, targetElems);
        endT = std::chrono::high_resolution_clock::now();

        auto t = (endT - startT).count() / repeatNum;
        unsigned long prevT = timeProbes.back();
        timeProbes.push_back(t);

        //if (prevT != 1 && t / prevT > maxDiff) {
        //if (prevT != 1 && checkTimeDifference(t, prevT, maxDiff)) {
        //    std::cout << "Potential accos is " << targetElems - 1 << '\n';
        //    //break;
        //}
    }


    delete [] data;
    return timeProbes;
}

inline void mainRoutine(std::vector<int> &levels, size_t stride) {
    std::cout << "Stride: " << stride << '\n';
    for (const auto &item : levels) {
        size_t startA = item == 1 ? 1 : 4 * (item - 1); // looks ok?
        std::cout << "Level: " << item << ". Start assoc: " << startA << '\n';
        auto measures = traverseAssociativityLevel(cacheBounds[item - 1] * 1024, stride, startA, item);
        std::cout << measures.size() << '\n';
        size_t c = startA;
        for (const auto &item : measures) {
            std::cout << c++ << ',' << item << ' ';
        }
        std::cout << '\n';
    }
}


void tryDetermineAssoc(std::vector<unsigned long>& probes, size_t level, size_t startAssoc) {
    auto maxDiff = getMaxDiff(level);
    auto prevT = probes.front();
    for (size_t i = startAssoc; i < probes.size(); ++i) {
        auto t = probes[i];
        if (prevT != 1 && checkTimeDifference(t, prevT, maxDiff)) {
            std::cout << "Potential Assoc for L" << level << " is: " << i << '\n';
            break;
        }
        prevT = t;
    }

}

int main() {
    std::vector<int> levels = {1, 2}; // better for 2 levels
    //for (long st = 2 * 1024; st < 2 << 14; st <<= 2) { // seems like not so optimal
    //    mainRoutine(levels, st);
    //}

    for (const auto &item : levels) {
        size_t startA = item == 1 ? 1 : 4 * (item - 1); // looks ok?
        std::cout << "Level: " << item << ". Start assoc: " << startA << '\n';
        auto measures = traverseAssociativityLevel(cacheBounds[item - 1] * 1024, 512, startA, item);
        std::cout << measures.size() << '\n';
        size_t c = startA;
        for (const auto &item : measures) {
            std::cout << c++ << ',' << item << ' ';
        }
        std::cout << '\n';

        tryDetermineAssoc(measures, item, startA); // this is demo-feature, results are not always accurate
    }


}