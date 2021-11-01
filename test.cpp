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


long traverseCache(long size, int cacheLineSize) {
    char* buf = new char[size];

    auto startTime = std::chrono::high_resolution_clock::now();
    //auto startTime = clock();

    // replace with cache-line size
    for (size_t i = 0; i < 64 * MB; i++){
        ++buf[(i * cacheLineSize) % size]; // means we write to a new cache-line (I hope so)
    }

    //auto endTime = clock();
    auto endTime = std::chrono::high_resolution_clock::now();
    delete [] buf;

    return (endTime - startTime).count();
    //return (endTime - startTime);
}

// looks ok
void determineL1Size(std::vector<long>& ans) {
    long maxDiff = 0;
    long prevTime = 1;
    std::vector<std::pair<long, long>> l1Probes;
    long maxDiffSize = 1024;
    // run separate loop for L1
    for (long size = 1024; size <= L1SizeBound * 1024; size += 16 * 1024) {
        auto currTime = traverseCache(size, 64);
        auto currDiff = std::abs(currTime - prevTime);
        //std::cout << size/1024 << ' ' << currTime << ' '  << currDiff << ' ' <<  '\n';
        if (!l1Probes.empty()) {
            auto prevDiff = l1Probes.back().first;
            auto relDiff = currDiff / prevDiff;
            if (relDiff > maxDiff) {
                maxDiff = prevDiff;
                maxDiffSize = size / 1024;
            }
        }

        l1Probes.emplace_back(currDiff, size/1024);
        prevTime = currTime;
    }

    std::cout << maxDiffSize << '\n';
    ans.push_back(maxDiffSize - 1);
}

std::vector<long> determineCacheSizes() {
    // of size
    std::vector<long> ans;
    // of time
    std::vector<long> tmp;

    long minTime = 1;
    auto step = 256 * 1024;
    // consider step for cache change like 256kb since first occurrence
    long maxDiff = 0;
    long prevTime = 1;

    determineL1Size(tmp);
    //return tmp;
    long startSize = 512 * 1024;
    // L2 and L3; 4096 for L2
    //std::vector<long> cacheBound{static_cast<long>(pow(2, 12)), 9200};
    std::vector<long> cacheBound{static_cast<long>(pow(2, 12)), static_cast<long>(pow(2, 14))};
    std::vector<std::pair<long, long>> probes;
    long cacheS = 1;

    for (const auto &item : cacheBound) {
        std::cout << "Seek from " <<startSize/1024 << " to " << item * 1024 << '\n';
        probes.clear();
        for (long size = startSize; size <= item * 1024 - 1; size += step) {
            auto currTime = traverseCache(size, 64);
            auto currDiff = std::abs(currTime - prevTime);
            if (size == startSize) {
                prevTime = currTime;
                continue;
            }
            if (!probes.empty()) {
                if (currDiff > maxDiff) {
                    maxDiff = currDiff;
                    cacheS = size / 1024;
                }
            }
            probes.emplace_back(currDiff, size/1024);
            prevTime = currTime;
            std::cout << size / 1024 << ' ' << currTime << ' ' <<  currDiff <<'\n';
        }
        startSize = item * 1024;
        std::cout << "Potential cache is " << cacheS << '\n';
        tmp.push_back(cacheS);
        maxDiff = 1;
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