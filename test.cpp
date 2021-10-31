#include <chrono>
#include <vector>
#include <set>
#include <iostream>
#include <map>
#include <algorithm>

long H = 16;// start Stride size
// S - start associativity
long S = 2, cur_time, prev_time, jump;
const int MB = 1024 * 1024;
// z- max mem, N - max assoc, M - Max Stride
long Z = 128 * 1024, N = 50, M = 100;
//int *data[100000000];
int *data[MB * 2];

const int REPS = 512 * MB;
const int length = MB/sizeof(int) - 1;

void runLoop(int times) {
    //    int tmp = 0;
    //    int **x = (int **) &data[0];
    //    for (size_t j = 0; j < times; j++) {
    //        for (size_t k = 0; k < REPS; k++) {
    //            // read the data in strides (`i`)
    //            tmp += *x[(k)& 100000000];
    //            std:: cout << tmp << ' ';
    //        }
    //    }
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
    //std::cout << cur_time << ' ' << "cyclic refs of kb: " << currSizeKB  << '\n';
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
    for (long size = 1024; size <= 128 * 1024; size += 16 * 1024) {
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
    return tmp;


    for (long size = 1024; size <= 10 * MB; size += step - 1) {
        auto currTime = traverseCache(size, 64);
        auto currDiff = abs(currTime - minTime);
        if (size >= 256 * 1024) {
            step = 256 * 1024;
        }

        if (size == 1024) {
            minTime = currTime;
            currDiff = 1;
        } else if (currTime < minTime) {
            if (tmp.size()) {
                int back = tmp[tmp.size() - 1];
                //std::cout << "Diff is: " << size/1024 - back << '\n';
                if (size/1024 - back <= 512) {
                    std::cout << size / 1024 << ' ' << currTime << ' ' <<  currDiff <<'\n';
                    continue;
                }
            }
            tmp.push_back(size / 1024);
            //lastSpotted = size;
            minTime = currTime;
        }
        std::cout << size / 1024 << ' ' << currTime << ' ' <<  currDiff <<'\n';
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
    std::cout <<( H * N < Z) << " Start\n";

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

                //if (estimatedCacheSize <= Z) {
                //    std::cout<< "time: " << cur_time <<" jump at " << H << ' ' << S << '\n';
                //}

                RecordJump();
                seenJumpsAt.insert(H);
                if (record.count(H)) {
                    auto& timeV = record[H];
                    timeV.push_back({cur_time, S});
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
        auto& innerV  = item.second;
        std:: cout << item.first << '\n';

        std::cout << innerV.size() << '\n';
        for (const auto &vElem : innerV) {
            currT = std::get<0>(vElem);
            std::cout << currT << ' ';
            float tDiff = (prevT != 1) ? currT / prevT : 1;
            std::cout << tDiff <<'\n';
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
    std::cout << "Cache-line size: " << cacheLineS << '\n';


    auto caches = determineCacheSizes();

    std::cout << caches.size() << '\n';
    for (const auto &item : caches) {
        std::cout << item << ' ';
    }
}