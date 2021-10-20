#include <chrono>

long H = 16, S = 1, cur_time, prev_time, jump;
long Z = 1000000000000, N = 100, M = 100;

// The time() subroutine returns the timing measurement for a million reads for a specified higher stride and number of spots
long Time() {
   auto sTime = std::chrono::high_resolution_clock::now();
   return 1;
}

// The DeltaDiff() subroutine returns true if there is a jump(or a difference in timing measurements) between the current time and previous time
long DeltaDiff() {
 return 1;
}

// The RecordJump() subroutine records the position of the jump in stride and spots
long RecordJump() {
 return 1;
}

// The isMovement() subroutine returns true if there is a movement in the position of
//the jumps for a current stride when compared to a previous stride
bool isMovement() {
 return 1;

}

//The DetectEntity subroutine starts decreasing the stride from specified value of H and records the stride at which there is movement in jumps for each entity. This gives each entity and its stride of entity.
void DetectEntity(long H) {
}

/**
 * Написать программу, которая вычисляет и печатает характеристики кэша данных компьютера
● Число уровней
● Для каждого уровня

● Размер
● Длина линейки
● Ассоцитивность

● Рекомендуемый язык: C++

● Функция для чтения таймера высокого
разрешения std::chrono::high_resolution_clock::now()
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
        if (isMovement()) {
            H *= 2;
        } else {
            break;
        }
    }
    DetectEntity(H);
}