#include "matrix.hpp"
#include <stdexcept>
#include <cmath>

void checkIndex(int r, int c, int rowCount, int colCount) {
    if (r < 0 || r >= rowCount || c < 0 || c >= colCount)
        throw std::out_of_range("Индекс матрицы находится вне допустимых границ");
}

void checkMultiplication(const Matrix& a, const Matrix& b) {
    if (a.cols() != b.rows())
        throw std::invalid_argument("Нельзя умножить матрицы: число столбцов первой не равно числу строк второй");
}

DenseMatrix::DenseMatrix(int r, int c)
    : rowCount(r), colCount(c), data(static_cast<std::size_t>(r) * c, 0.0) {
    if (r <= 0 || c <= 0)
        throw std::invalid_argument("Размеры матрицы должны быть положительными");
}

int DenseMatrix::rows() const { return rowCount; }
int DenseMatrix::cols() const { return colCount; }

double DenseMatrix::get(int r, int c) const {
    checkIndex(r, c, rowCount, colCount);
    return data[static_cast<std::size_t>(r) * colCount + c];
}

void DenseMatrix::set(int r, int c, double value) {
    checkIndex(r, c, rowCount, colCount);
    data[static_cast<std::size_t>(r) * colCount + c] = value;
}

void DenseMatrix::print(std::ostream& out) const {
    for (int r = 0; r < rowCount; ++r) {
        for (int c = 0; c < colCount; ++c)
            out << get(r, c) << (c + 1 == colCount ? '\n' : ' ');
    }
}

SparseMatrix::SparseMatrix(int r, int c) : rowCount(r), colCount(c) {
    if (r <= 0 || c <= 0)
        throw std::invalid_argument("Размеры матрицы должны быть положительными");
}

int SparseMatrix::rows() const { return rowCount; }
int SparseMatrix::cols() const { return colCount; }

double SparseMatrix::get(int r, int c) const {
    checkIndex(r, c, rowCount, colCount);
    auto it = data.find({r, c});
    return it == data.end() ? 0.0 : it->second;
}

void SparseMatrix::set(int r, int c, double value) {
    checkIndex(r, c, rowCount, colCount);
    if (value == 0.0) data.erase({r, c});
    else data[{r, c}] = value;
}

void SparseMatrix::print(std::ostream& out) const {
    for (int r = 0; r < rowCount; ++r) {
        for (int c = 0; c < colCount; ++c)
            out << get(r, c) << (c + 1 == colCount ? '\n' : ' ');
    }
}

std::size_t SparseMatrix::nonZeroCount() const { return data.size(); }
const std::map<std::pair<int, int>, double>& SparseMatrix::entries() const { return data; }

// Dense × Dense
static std::unique_ptr<Matrix> multiplyDenseDense(const DenseMatrix& a, const DenseMatrix& b) {
    checkMultiplication(a, b);
    auto result = std::make_unique<DenseMatrix>(a.rows(), b.cols());
    for (int i = 0; i < a.rows(); ++i)
        for (int k = 0; k < a.cols(); ++k) {
            const double left = a.get(i, k);
            if (left == 0.0) continue;
            for (int j = 0; j < b.cols(); ++j)
                result->set(i, j, result->get(i, j) + left * b.get(k, j));
        }
    return result;
}

// Dense × Sparse
static std::unique_ptr<Matrix> multiplyDenseSparse(const DenseMatrix& a, const SparseMatrix& b) {
    checkMultiplication(a, b);
    auto result = std::make_unique<DenseMatrix>(a.rows(), b.cols());
    for (int i = 0; i < a.rows(); ++i) {
        for (int k = 0; k < a.cols(); ++k) {
            const double left = a.get(i, k);
            if (left == 0.0) continue;
            auto it = b.entries().lower_bound({k, 0});
            while (it != b.entries().end() && it->first.first == k) {
                const int j = it->first.second;
                result->set(i, j, result->get(i, j) + left * it->second);
                ++it;
            }
        }
    }
    return result;
}

// Sparse × Dense
static std::unique_ptr<Matrix> multiplySparseDense(const SparseMatrix& a, const DenseMatrix& b) {
    checkMultiplication(a, b);
    auto result = std::make_unique<DenseMatrix>(a.rows(), b.cols());
    for (const auto& entry : a.entries()) {
        const int i = entry.first.first;
        const int k = entry.first.second;
        const double left = entry.second;
        for (int j = 0; j < b.cols(); ++j)
            result->set(i, j, result->get(i, j) + left * b.get(k, j));
    }
    return result;
}

// Sparse × Sparse
static std::unique_ptr<Matrix> multiplySparseSparse(const SparseMatrix& a, const SparseMatrix& b) {
    checkMultiplication(a, b);
    auto result = std::make_unique<SparseMatrix>(a.rows(), b.cols());
    for (const auto& leftEntry : a.entries()) {
        const int i = leftEntry.first.first;
        const int k = leftEntry.first.second;
        const double left = leftEntry.second;
        auto it = b.entries().lower_bound({k, 0});
        while (it != b.entries().end() && it->first.first == k) {
            const int j = it->first.second;
            result->set(i, j, result->get(i, j) + left * it->second);
            ++it;
        }
    }
    return result;
}

std::unique_ptr<Matrix> DenseMatrix::mul(const Matrix& other) const {
    if (const auto* dense = dynamic_cast<const DenseMatrix*>(&other))
        return multiplyDenseDense(*this, *dense);
    if (const auto* sparse = dynamic_cast<const SparseMatrix*>(&other))
        return multiplyDenseSparse(*this, *sparse);
    throw std::invalid_argument("Неизвестный тип правой матрицы");
}

std::unique_ptr<Matrix> SparseMatrix::mul(const Matrix& other) const {
    if (const auto* dense = dynamic_cast<const DenseMatrix*>(&other))
        return multiplySparseDense(*this, *dense);
    if (const auto* sparse = dynamic_cast<const SparseMatrix*>(&other))
        return multiplySparseSparse(*this, *sparse);
    throw std::invalid_argument("Неизвестный тип правой матрицы");
}

DenseMatrix readDenseFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Не удалось открыть файл: " + filename);

    int r, c;
    if (!(file >> r >> c))
        throw std::runtime_error("В начале файла должны быть размеры: число_строк число_столбцов");

    DenseMatrix matrix(r, c);
    for (int i = 0; i < r; ++i) {
        for (int j = 0; j < c; ++j) {
            double value;
            if (!(file >> value))
                throw std::runtime_error("В файле недостаточно элементов матрицы");
            matrix.set(i, j, value);
        }
    }

    std::string extra;
    if (file >> extra)
        throw std::runtime_error("В файле есть лишние данные после матрицы");

    return matrix;
}

SparseMatrix toSparse(const Matrix& source) {
    SparseMatrix result(source.rows(), source.cols());
    for (int i = 0; i < source.rows(); ++i)
        for (int j = 0; j < source.cols(); ++j) {
            const double value = source.get(i, j);
            if (value != 0.0) result.set(i, j, value);
        }
    return result;
}

bool matricesEqual(const Matrix& a, const Matrix& b, double eps) {
    if (a.rows() != b.rows() || a.cols() != b.cols()) return false;
    for (int i = 0; i < a.rows(); ++i)
        for (int j = 0; j < a.cols(); ++j)
            if (std::abs(a.get(i, j) - b.get(i, j)) > eps) return false;
    return true;
}


#ifndef MATRIX_NO_MAIN
#include <iostream>
#include <exception>

int main(int argc, char* argv[]) {
    try {
        if (argc == 4 && std::string(argv[1]) == "file") {
            DenseMatrix a = readDenseFromFile(argv[2]);
            DenseMatrix b = readDenseFromFile(argv[3]);
            auto result = a.mul(b);
            std::cout << "Результат умножения:\n";
            result->print();
            return 0;
        }

        if (argc == 2 && std::string(argv[1]) == "demo") {
            DenseMatrix a(2, 2);
            a.set(0, 0, 1); a.set(0, 1, 2);
            a.set(1, 0, 3); a.set(1, 1, 4);

            SparseMatrix b(2, 2);
            b.set(0, 0, 5); b.set(1, 1, 6);

            auto result = a.mul(b);
            std::cout << "A × B =\n";
            result->print();
            return 0;
        }

        std::cout << "Использование:\n"
                  << "  ./matrix_app demo\n"
                  << "  ./matrix_app file tests/example_A.txt tests/example_B.txt\n";
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << '\n';
        return 1;
    }
}

#endif // MATRIX_NO_MAIN
