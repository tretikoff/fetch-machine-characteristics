#include <chrono>
#include <vector>
#include <set>
#include <iostream>

long H = 16;// start Stride size
// S - start associativity
long S = 2, cur_time, prev_time, jump;
const int MB = 1024 * 1024;
// z- max mem, N - max assoc, M - Max Stride
long Z = 1024, N = 50, M = 100;
int *data[100000000];
int movements[256];

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
        //        x = (int **) *(x+ i % 100000000);
        x = (int **) *(x);
        auto v = x + 111;
        //        std::cerr << v << ' ';
    }

}

// The time() subroutine returns the timing measurement for a million reads
// for a specified higher stride H and number of spots S
long Time() {
    auto end = (S - 1) * H;
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
    jumps.insert(H - 1);
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
    int jmpC = 0;
    while (H * N < Z) {
        //        S = 1;
        while (S < N) {
            cur_time = Time();
            if (DeltaDiff()) {
                jmpC++;
                //                std::cout<< "time: " << cur_time <<" jump at " << H << ' ' << S << '\n';
                std::cout<< "time: " << cur_time <<" jump at " << H << ' ' << S << ' ' << S /jmpC  << '\n';
                RecordJump();
            }
            //            S++;
            S += 2;
            prev_time = cur_time;
        }
        //        std::cout<< prev_time << " for " << H << '\n';
        if (isMovement()) { // Вроде как мы должны проверить, что jumps и prev_jumps различаются. Если нет - прерываем цикл
            //            movements[H] = jumps.size();
            prev_jumps = jumps;
            jumps.clear();
            H *= 2; // stride number
        } else {
            break;
        }
    }
    DetectEntity(H);
    //    std:: cout << H / 1024 << ' ' << S;
    std:: cout << H  << ' ' << S << ' ' << jmpC;
}