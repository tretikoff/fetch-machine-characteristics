#include <chrono>
#include <vector>
#include <set>
#include <iostream>
#include <map>
#include <algorithm>
#include <cmath>

long H = 16;// start Stride size
// S - start associativity
long S = 2, cur_time, prev_time, jump;
const int MB = 1024 * 1024;
// z- max mem, N - max assoc, M - Max Stride
long Z = 128 * 1024, N = 50, M = 100;
//int *data[100000000];
int *data[MB * 2];

const int L1SizeBound = 512;
const int REPS = 512 * MB;
const int length = MB/sizeof(int) - 1;

void runLoop(int times) {
    int **x = (int **) &data[0];
    for (int i = 0; i < times; i++) {
        x = (int **) *(x);
    }

}

long estimatedCacheSize = 1;


// The time() subroutine returns the timing measurement for a million reads
// for a specified higher stride H and number of spots S
long Time() {
    auto end = (S - 1) * H;
    auto currSizeKB = (end - H)/1024;
    // and if currSizeKB <= CACHE_BOUND
    if (estimatedCacheSize < currSizeKB) {
        estimatedCacheSize = currSizeKB;
    }
    std::cout << cur_time << ' ' << "cyclic refs of kb: " << currSizeKB  << '\n';
    // Create the cyclic chain of references
    for (auto i = H; i <= end; i += H) {
        data[i] = (int*)&data[i - H];
    }
    data[0] = (int*)&data[end];

    runLoop(200000);
    auto startTime = std::chrono::high_resolution_clock::now();
    runLoop(1000000);
    auto endTime = std::chrono::high_resolution_clock::now();
    //std::cout << "inside Time : " << (endTime - startTime).count() << '\n';
    return (endTime - startTime).count();
}

// The DeltaDiff() subroutine returns true if there is a jump(or a difference in timing measurements) between the current time and previous time
bool DeltaDiff() {
    //    const long diff = 1000000;
    const long diff = 1e6;
    return (cur_time - prev_time) < diff;
}

std::set<long> prev_jumps;
std::set<long> jumps;

// The RecordJump() subroutine records the position of the jump in stride and spots
void RecordJump() {
    //    jumps.insert(S - 1);
    jumps.insert(H);
}

// The isMovement() subroutine returns true if there is a movement in the position of
// the jumps for a current stride when compared to a previous stride
bool isMovement() {
    return (prev_jumps != jumps);
}

//The DetectEntity subroutine starts decreasing the stride from specified value of H and records the stride at which there is movement in jumps for each entity. This gives each entity and its stride of entity.
void DetectEntity(long H) {
    while (H > 0) {
        //        std::cout << movements[H] << ' ';
        // if (movementInJumps) { std::cout << ""; }
        H /= 2;
    }
}


bool sortFirst(const std::tuple<int, int>& lhs,const std::tuple<int, int>& rhs) {
    //    return (std::get<0>(lhs) < std::get<0>(rhs) && std::get<1>(lhs) < std::get<1>(rhs));
    //    return (std::get<1>(lhs) < std::get<1>(rhs) && std::get<0>(lhs) < std::get<0>(rhs));
    return (std::get<1>(lhs) < std::get<1>(rhs));
}


size_t* prepareArray(long size, size_t cacheLineSize) {
    auto* buf = new size_t[size]{0};

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

long traverseL1Cache(long size, size_t cacheLineSize) {
    char* buf = new char[size];
    auto startTime = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < 64 * MB; i++){
        ++buf[(i * cacheLineSize) % size]; // means we write to a new cache-line (I hope so)
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    delete [] buf;

    return (endTime - startTime).count();
}

long traverseCache(long size, size_t cacheLineSize) {
    auto* buf = prepareArray(size, cacheLineSize);
    size_t v = 0;
    //auto startTime = std::chrono::high_resolution_clock::now();
    auto startTime = std::chrono::steady_clock::now();

    // repeat 10 times
    for (size_t i = 0; i < 10 * size; i++) {
        v = buf[v];
    }
    auto endTime = std::chrono::steady_clock::now();

    delete [] buf;

    return (endTime - startTime).count();
}

// looks ok
void determineL1Size(std::vector<long>& ans) {
    long maxDiff = 1;
    long prevTime = 1;
    std::vector<std::pair<long, long>> l1Probes;
    long maxDiffSize = 1024;
    // run separate loop for L1
    //for (long size = 16 * 1024; size <= L1SizeBound * 1024; size += 16 * 1024) {
    for (long size = 8 * 1024; size <= L1SizeBound * 1024; size *= 2) {
        auto currTime = traverseCache(size, 64);
        auto currDiff = std::abs(currTime - prevTime);
        //std::cout << size/1024 << ' ' << currTime << ' '  << currDiff << ' ' <<  '\n';
        if (!l1Probes.empty()) {
            auto prevDiff = l1Probes.back().first;
            auto relDiff = currDiff / prevDiff;
            //std::cout << size/1024 << ' ' << currDiff << ' '  << relDiff << ' ' << maxDiff << '\n';
            if (relDiff > maxDiff) {
                maxDiff = currDiff;
                maxDiffSize = size / 1024;
            }
        }

        l1Probes.emplace_back(currDiff, size/1024);
        prevTime = currTime;
    }

    std::cout << "L1 size: " <<  maxDiffSize << '\n';
    ans.push_back(maxDiffSize - 1);
}

std::vector<long> determineCacheSizes() {
    // of size
    std::vector<long> ans;
    // of time
    std::vector<long> tmp;

    long minTime = 1;
    auto step = 1024 * 1024;
    // consider step for cache change like 256kb since first occurrence
    long maxDiff = 1;
    long prevTime = 1;

    determineL1Size(tmp);
    //return tmp;
    long startSize = 512 * 1024;
    // L2 and L3; 4096 for L2
    //std::vector<long> cacheBound{static_cast<long>(pow(2, 12)), 9200};
    std::vector<long> cacheBound{static_cast<long>(pow(2, 12)), static_cast<long>(pow(2, 16))};
    std::vector<std::pair<long, long>> probes;
    long cacheS = 1;
    bool foundCache = false;
    float criticalBorderForCaches[] = {3, 2.2};
    size_t cacheLvl = 0;

    for (const auto &item : cacheBound) {
        std::cout << "Seek from " << startSize/1024 << " to " << item << '\n';
        probes.clear();
        for (long size = startSize; size <= item * 1024 + 1; size *= 2) {
            if (foundCache) break;
            auto currTime = traverseCache(size, 64) / 10; // should divide by 10?
            auto currDiff = std::abs(currTime - prevTime);
            if (size == startSize) {
                prevTime = currTime;
                probes.emplace_back(currDiff, size/1024);
                maxDiff = currDiff;
                continue;
            }
            if (!probes.empty()) {
                //if (currDiff > maxDiff) {
                if (maxDiff == 1) {
                    maxDiff = currDiff;
                } else if ((currDiff + 0.1) / maxDiff > criticalBorderForCaches[cacheLvl] && !foundCache) { // experienced even 7 here at cache border
                    maxDiff = currDiff;
                    cacheS = size / 1024;
                    foundCache = true;
                }
            }
            probes.emplace_back(currDiff, size/1024);
            prevTime = currTime;
            std::cout << size / 1024 << ' ' << currTime << ' ' <<  currDiff <<'\n';
        }
        startSize = item * 1024;
        std::cout << "Potential cache is " << cacheS << '\n';
        tmp.push_back(cacheS);
        maxDiff = cacheS = 1;
        foundCache = false;
        cacheLvl++;
    }


    return tmp;
}


/**
 * Написать программу, которая вычисляет и печатает характеристики кэша данных компьютера
● Число уровней
● Для каждого уровня
    ● Размер
    ● Длина линейки
    ● Ассоцитивность
 * */
int main() {
    std::cout << "Start\n";

    std::vector<long> jumpsTime;
    std::set<long> seenJumpsAt;
    // H -> (time, S)
    std::map<long, std::vector<std::tuple<int, int>>> record;


    int jmpC = 0;
    while (H * N < Z) {
        S = 2;
        while (S < N) {
            cur_time = Time();
            //std::cout << "cur_time: " << cur_time << '\n';
            if (DeltaDiff()) {
                jmpC++;
                // need to do something with S
                //std::cout<< "time: " << cur_time <<" jump at " << H << ' ' << S << '\n';

                RecordJump();
                seenJumpsAt.insert(H);
                if (record.count(H)) {
                    auto& timeV = record[H];
                    timeV.emplace_back(cur_time, S);
                } else {
                    record.insert({H, std::vector<std::tuple<int, int>>()});
                }
            }
            //S++;
            S += 4;
            prev_time = cur_time;
        }
        //std::cout<< prev_time << " for " << H << '\n';
        if (isMovement()) { // Вроде как мы должны проверить, что jumps и prev_jumps различаются. Если нет - прерываем цикл
            //movements[H] = jumps.size();
            //prev_jumps = jumps;
            jumps.clear();
            H *= 2; // stride number
        } else {
            break;
        }
    }
    //DetectEntity(H);
    //std:: cout << H / 1024 << ' ' << S;
    std:: cout << H  << ' ' << S << ' ' << jmpC << "\n\n";

    long currT, prevT = 1;
    long cacheLineS = 1;
    double maxDiff = 1.2;
    for (const auto &item : record) {
        auto& innerV = item.second;
        //std:: cout << item.first << ' ';

        for (const auto &vElem : innerV) {
            currT = std::get<0>(vElem);
            float tDiff = (prevT != 1) ? currT / prevT : 1;
            if (tDiff > maxDiff) {
                maxDiff = tDiff;
                std::cout << "Spark in " << currT << ' ' << item.first << "; diff: " << tDiff << '\n';
                cacheLineS = item.first;
            }
            prevT = currT;
        }


        //std::sort(innerV.begin(), innerV.end(), sortFirst);
        //std::cout << std::get<0>(innerV[0])  << ' ' << std::get<1>(innerV[0]) << '\n';
    }

    // TODO: perhaps, multiple runs needed to stabilise results
    std::cout << "Cache-line size: " << cacheLineS << std::endl;


    auto caches = determineCacheSizes();

    std::cout << "Levels: " << caches.size() << '\n';
    for (const auto &item : caches) {
        std::cout << item << ' ';
    }
}