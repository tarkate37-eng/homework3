#include "../matrix.hpp"
#include <iostream>
#include <random>
#include <chrono>
#include <iomanip>
#include <fstream>
#include <vector>
#include <string>

static DenseMatrix randomMatrix(int n, double density, std::mt19937& rng) {
    DenseMatrix result(n, n);
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::uniform_real_distribution<double> values(-1.0, 1.0);

    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (chance(rng) < density)
                result.set(i, j, values(rng));

    return result;
}

template<class Function>
double measureMilliseconds(Function operation, int repetitions) {
    double checksum = 0.0;
    const auto start = std::chrono::steady_clock::now();

    for (int i = 0; i < repetitions; ++i) {
        auto result = operation();
        checksum += result->get(0, 0);
    }

    const auto finish = std::chrono::steady_clock::now();
    volatile double keep = checksum;
    (void)keep;

    return std::chrono::duration<double, std::milli>(finish - start).count()
           / repetitions;
}

int main(int argc, char* argv[]) {
    const int n = argc >= 2 ? std::stoi(argv[1]) : 220;
    const int repetitions = argc >= 3 ? std::stoi(argv[2]) : 3;

    if (n <= 0 || repetitions <= 0) {
        std::cerr << "Размер и число повторений должны быть положительными.\n";
        return 1;
    }

    const std::vector<double> densities = {0.50, 0.10, 0.05, 0.03, 0.02, 0.01, 0.001};
    std::mt19937 rng(123456);

    std::ofstream csv("bench/results.csv");
    if (!csv) {
        std::cerr << "Не удалось создать bench/results.csv. Запустите программу из корня проекта.\n";
        return 1;
    }

    csv << "density_percent,DD_ms,DS_ms,SD_ms,SS_ms,fastest\n";
    std::cout << "Размер матриц: " << n << " x " << n
              << "; повторений: " << repetitions << "\n\n";
    std::cout << "Плотность | Dense×Dense | Dense×Sparse | Sparse×Dense | Sparse×Sparse | Быстрее всех\n";
    std::cout << "----------|------------|--------------|--------------|---------------|-------------\n";

    for (double density : densities) {
        // Создаём данные один раз. Dense и Sparse варианты получают одни и те же числа.
        DenseMatrix a = randomMatrix(n, density, rng);
        DenseMatrix b = randomMatrix(n, density, rng);
        SparseMatrix sa = toSparse(a);
        SparseMatrix sb = toSparse(b);

        const double dd = measureMilliseconds([&]() { return a.mul(b); }, repetitions);
        const double ds = measureMilliseconds([&]() { return a.mul(sb); }, repetitions);
        const double sd = measureMilliseconds([&]() { return sa.mul(b); }, repetitions);
        const double ss = measureMilliseconds([&]() { return sa.mul(sb); }, repetitions);

        const double times[] = {dd, ds, sd, ss};
        const std::string names[] = {"Dense×Dense", "Dense×Sparse", "Sparse×Dense", "Sparse×Sparse"};
        int fastestIndex = 0;
        for (int i = 1; i < 4; ++i)
            if (times[i] < times[fastestIndex]) fastestIndex = i;

        std::cout << std::fixed << std::setprecision(3)
                  << std::setw(8) << density * 100 << "% | "
                  << std::setw(10) << dd << " | "
                  << std::setw(12) << ds << " | "
                  << std::setw(12) << sd << " | "
                  << std::setw(13) << ss << " | "
                  << names[fastestIndex] << '\n';

        csv << std::setprecision(6) << density * 100 << ','
            << dd << ',' << ds << ',' << sd << ',' << ss << ','
            << names[fastestIndex] << '\n';
    }

    std::cout << "\nПодробная таблица сохранена в bench/results.csv\n";
    std::cout << "Ищите плотность, при которой Sparse×Sparse становится быстрее Dense×Dense.\n";
    return 0;
}
