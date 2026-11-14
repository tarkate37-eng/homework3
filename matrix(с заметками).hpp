#ifndef MATRIX_HPP
#define MATRIX_HPP

#include <iostream>
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include <string>
#include <utility>

class Matrix {
public:
    virtual ~Matrix() = default;
    virtual int rows() const = 0;
    virtual int cols() const = 0;
    virtual double get(int r, int c) const = 0;
    virtual void set(int r, int c, double value) = 0;
    virtual void print(std::ostream& out = std::cout) const = 0;
    virtual std::unique_ptr<Matrix> mul(const Matrix& other) const = 0;
};

void checkIndex(int r, int c, int rowCount, int colCount);
void checkMultiplication(const Matrix& a, const Matrix& b);

class DenseMatrix : public Matrix {
private:
    int rowCount, colCount;
    std::vector<double> data;
public:
    DenseMatrix(int r, int c);
    int rows() const override;
    int cols() const override;
    double get(int r, int c) const override;
    void set(int r, int c, double value) override;
    void print(std::ostream& out = std::cout) const override;
    std::unique_ptr<Matrix> mul(const Matrix& other) const override;
};

class SparseMatrix : public Matrix {
private:
    int rowCount, colCount;
    std::map<std::pair<int, int>, double> data;
public:
    SparseMatrix(int r, int c);
    int rows() const override;
    int cols() const override;
    double get(int r, int c) const override;
    void set(int r, int c, double value) override;
    void print(std::ostream& out = std::cout) const override;
    std::size_t nonZeroCount() const;
    const std::map<std::pair<int, int>, double>& entries() const;
    std::unique_ptr<Matrix> mul(const Matrix& other) const override;
};

DenseMatrix readDenseFromFile(const std::string& filename);
SparseMatrix toSparse(const Matrix& source);
bool matricesEqual(const Matrix& a, const Matrix& b, double eps = 1e-9);

#endif
