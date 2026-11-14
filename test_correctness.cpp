#include "../matrix.hpp"
#include <iostream>
#include <random>
#include <stdexcept>
#include <cmath>

static void require(bool condition, const std::string& name) {
    if (!condition) throw std::runtime_error("ТЕСТ НЕ ПРОЙДЕН: " + name);
    std::cout << "[OK] " << name << '\n';
}

static DenseMatrix randomMatrix(int rows, int cols, double density, std::mt19937& rng) {
    DenseMatrix result(rows, cols);
    std::uniform_real_distribution<double> chance(0.0, 1.0);
    std::uniform_real_distribution<double> values(-5.0, 5.0);

    for (int i = 0; i < rows; ++i)
        for (int j = 0; j < cols; ++j)
            if (chance(rng) < density)
                result.set(i, j, values(rng));

    return result;
}

int main() {
    try {
        std::cout << "=== ТЕСТЫ КОРРЕКТНОСТИ ===\n";

        // 1. Маленький пример с ответом, посчитанным вручную.
        DenseMatrix a(2, 2), b(2, 2), expected(2, 2);
        a.set(0,0,1); a.set(0,1,2); a.set(1,0,3); a.set(1,1,4);
        b.set(0,0,5); b.set(0,1,6); b.set(1,0,7); b.set(1,1,8);
        expected.set(0,0,19); expected.set(0,1,22);
        expected.set(1,0,43); expected.set(1,1,50);

        auto manual = a.mul(b);
        require(matricesEqual(*manual, expected), "маленький пример, проверенный вручную");

        // 2. Умножение на единичную и нулевую матрицы.
        DenseMatrix identity(3, 3), zero(3, 3), sample(3, 3);
        for (int i = 0; i < 3; ++i) identity.set(i, i, 1.0);
        sample.set(0,0,2); sample.set(0,2,4);
        sample.set(1,1,-3); sample.set(2,0,7);

        SparseMatrix sparseIdentity = toSparse(identity);
        SparseMatrix sparseZero = toSparse(zero);
        SparseMatrix sparseSample = toSparse(sample);

        auto idDenseSparse = sample.mul(sparseIdentity);
        auto idSparseDense = sparseIdentity.mul(sample);
        auto zeroDenseSparse = sample.mul(sparseZero);
        auto zeroSparseDense = sparseZero.mul(sample);

        require(matricesEqual(*idDenseSparse, sample) &&
                matricesEqual(*idSparseDense, sample),
                "умножение на единичную матрицу");
        require(matricesEqual(*zeroDenseSparse, zero) &&
                matricesEqual(*zeroSparseDense, zero),
                "умножение на нулевую матрицу");

        // 3. Главная кросс-проверка: одни и те же данные в dense и sparse формате.
        std::mt19937 rng(20261114);
        DenseMatrix denseA = randomMatrix(30, 24, 0.12, rng);
        DenseMatrix denseB = randomMatrix(24, 18, 0.10, rng);
        SparseMatrix sparseA = toSparse(denseA);
        SparseMatrix sparseB = toSparse(denseB);

        auto dd = denseA.mul(denseB);
        auto ds = denseA.mul(sparseB);
        auto sd = sparseA.mul(denseB);
        auto ss = sparseA.mul(sparseB);

        require(matricesEqual(*dd, *ds) &&
                matricesEqual(*dd, *sd) &&
                matricesEqual(*dd, *ss),
                "кросс-проверка всех четырёх сочетаний Dense/Sparse");

        // 4. Дополнительная проверка индексов и несовместимых размеров.
        bool badSizeCaught = false;
        try {
            DenseMatrix wrong(5, 2);
            auto unused = a.mul(wrong);
        } catch (const std::invalid_argument&) {
            badSizeCaught = true;
        }
        require(badSizeCaught, "обнаружение несовместимых размеров");

        bool badIndexCaught = false;
        try {
            (void)a.get(10, 0);
        } catch (const std::out_of_range&) {
            badIndexCaught = true;
        }
        require(badIndexCaught, "обнаружение выхода за границы матрицы");

        // 5. Ноль не должен храниться в SparseMatrix.
        SparseMatrix s(2, 2);
        s.set(0, 0, 4.0);
        s.set(0, 0, 0.0);
        require(s.nonZeroCount() == 0, "SparseMatrix удаляет явно записанный ноль");

        std::cout << "Все тесты пройдены успешно.\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
}
