ПРОЕКТ УМНОЖЕНИЯ ПЛОТНЫХ И РАЗРЕЖЕННЫХ МАТРИЦ

СТРУКТУРА
matrix.hpp                 объявления классов Matrix, DenseMatrix, SparseMatrix
main.cpp                   объявления реализаций матриц + main программы
tests/test_correctness.cpp отдельная программа тестов корректности
bench/benchmark.cpp        отдельная программа измерения производительности

Реализация классов и функция main теперь находятся в одном main.cpp.
Для тестов и benchmark main временно отключается макросом MATRIX_NO_MAIN.
tests/                     примеры входных файлов
bench/results.csv          таблица, создаваемая при запуске benchmark

КОМПИЛЯЦИЯ (из корневой папки проекта, C++17)

Основная программа:
g++ -std=c++17 -O2 -Wall -Wextra -pedantic main.cpp -o matrix_app

Тесты:
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -DMATRIX_NO_MAIN tests/test_correctness.cpp main.cpp -o tests_correctness

Benchmark:
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -DMATRIX_NO_MAIN bench/benchmark.cpp main.cpp -o benchmark

ЗАПУСК

Демонстрация:
./matrix_app demo

Умножение матриц из файлов:
./matrix_app file tests/example_A.txt tests/example_B.txt

Тесты корректности:
./tests_correctness

Измерение производительности (по умолчанию 220x220, 3 повтора):
./benchmark

Можно задать размер и число повторов:
./benchmark 300 5

ФОРМАТ ФАЙЛА
Первая строка: количество_строк количество_столбцов
Далее все элементы матрицы построчно, разделённые пробелами.

ВАЖНО О BENCHMARK
Результаты зависят от компьютера, компилятора и нагрузки системы.
Для отчёта запускайте benchmark на своём компьютере. Не выдавайте примерные
или чужие значения за результаты собственного измерения.
