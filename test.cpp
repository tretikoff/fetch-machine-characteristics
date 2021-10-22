#include <chrono>
#include <vector>
#include <set>

long H = 16;// Stride size
long S = 1, cur_time, prev_time, jump;
long Z = 1000000000000, N = 100, M = 100;
int *data[100000000];

void runLoop(int times) {
    int **x = (int **) &data[0];
    for (int i = 0; i < times; i++) {
        x = (int **) *x;
    }
}

// The time() subroutine returns the timing measurement for a million reads
// for a specified higher stride H and number of spots S
long Time() {
    auto end = (S - 1) * H;
    // Create the cyclic chain of references
    for (int i = H; i <= end; i += H) {
        data[i] = (int*)&data[i - H];
    }
    data[0] = (int*)&data[end];

    runLoop(200000);
    auto startTime = std::chrono::high_resolution_clock::now();
    runLoop(1000000);
    auto endTime = std::chrono::high_resolution_clock::now();
    return (startTime - endTime).count();
}

// The DeltaDiff() subroutine returns true if there is a jump(or a difference in timing measurements) between the current time and previous time
bool DeltaDiff() {
    long diff = 100000;
    return cur_time - prev_time > diff;
}

std::set<long> prev_jumps;
std::set<long> jumps;

// The RecordJump() subroutine records the position of the jump in stride and spots
void RecordJump() {
    jumps.insert(S - 1);
}

// The isMovement() subroutine returns true if there is a movement in the position of
// the jumps for a current stride when compared to a previous stride
bool isMovement() {
    return (prev_jumps != jumps);
}

//The DetectEntity subroutine starts decreasing the stride from specified value of H and records the stride at which there is movement in jumps for each entity. This gives each entity and its stride of entity.
void DetectEntity(long H) {
    while (H > 0) {
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
    while (H * N < Z) {
        while (S < N) {
            cur_time = Time();
            if (DeltaDiff()) {
                RecordJump();
            }
            S += 1;
            prev_time = cur_time;
        }
        if (isMovement()) { // Вроде как мы должны проверить, что jumps и prev_jumps различаются. Если нет - прерываем цикл
            prev_jumps = jumps;
            H *= 2; // stride number
        } else {
            break;
        }
    }
    DetectEntity(H);
}