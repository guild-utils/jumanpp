//
// Created by Arseny Tolmachev on 2017/03/06.
//

#ifndef JUMANPP_SLICEABLE_ARRAY_H
#define JUMANPP_SLICEABLE_ARRAY_H

#include "util/array_slice.h"

namespace jumanpp {
namespace util {

template <typename T>
class ConstSliceable;

template <typename T>
class Sliceable {
  using underlying = util::MutableArraySlice<T>;
  underlying data_;
  size_t rowSize_;
  size_t numRows_;

 public:
  using size_type = typename util::MutableArraySlice<T>::size_type;
  using value_type = typename util::MutableArraySlice<T>::value_type;

  constexpr Sliceable() : data_{}, rowSize_{0}, numRows_{0} {}

  constexpr Sliceable(const MutableArraySlice<T>& data_, size_t rowSize_,
                      size_t numRows_)
      : data_(data_), rowSize_(rowSize_), numRows_{numRows_} {
    JPP_DCHECK_EQ(data_.size(), rowSize_ * numRows_);
  }

  T& operator[](size_type idx) { return data_.at(idx); }
  const T& operator[](size_type idx) const { return data_.at(idx); }
  T& at(size_type idx) { return data_.at(idx); }
  const T& at(size_type idx) const { return data_.at(idx); }
  T& at(size_t row, size_t col) { return this->row(row).at(col); }
  const T& at(size_t row, size_t col) const { return this->row(row).at(col); }
  util::MutableArraySlice<T> row(i32 i) {
    JPP_DCHECK_IN(i, 0, numRows_);
    return util::MutableArraySlice<T>{data_, rowSize_ * i, rowSize_};
  }
  util::ArraySlice<T> row(i32 i) const {
    JPP_DCHECK_IN(i, 0, numRows_);
    return util::ArraySlice<T>{data_, rowSize_ * i, rowSize_};
  }

  util::MutableArraySlice<T> data() { return data_; }

  util::ArraySlice<T> data() const { return util::ArraySlice<T>{data_}; }

  Sliceable<T> topRows(size_t nrows) {
    JPP_DCHECK_GE(nrows, 0);
    JPP_DCHECK_LE(nrows, numRows_);
    underlying slice{data_, 0, rowSize_ * nrows};
    return Sliceable<T>{slice, rowSize_, nrows};
  }

  Sliceable<T> rows(size_t start, size_t end) {
    JPP_DCHECK_GE(start, 0);
    JPP_DCHECK_LE(start, numRows_);
    JPP_DCHECK_LE(start, end);
    JPP_DCHECK_LE(end, numRows_);
    auto numElem = end - start;
    underlying slice{data_, start * rowSize_, rowSize_ * numElem};
    return Sliceable<T>{slice, rowSize_, numElem};
  }

  ConstSliceable<T> topRows(size_t nrows) const;
  ConstSliceable<T> rows(size_t start, size_t end) const;

  size_t size() const { return data_.size(); }
  size_t rowSize() const { return rowSize_; }
  size_t numRows() const { return numRows_; }

  typename underlying::iterator begin() { return data_.begin(); }
  typename underlying::const_iterator begin() const { return data_.begin(); }
  typename underlying::iterator end() { return data_.end(); }
  typename underlying::const_iterator end() const { return data_.end(); }
};

template <typename T>
class ConstSliceable {
  using underlying = util::ArraySlice<T>;
  underlying data_;
  size_t rowSize_;
  size_t numRows_;

 public:
  using size_type = typename util::ArraySlice<T>::size_type;
  using value_type = typename util::ArraySlice<T>::value_type;

  constexpr ConstSliceable() : data_{}, rowSize_{0}, numRows_{0} {}

  constexpr ConstSliceable(const Sliceable<T>& sl)
      : data_{sl.data()}, rowSize_{sl.rowSize()}, numRows_{sl.numRows()} {}

  constexpr ConstSliceable(const ArraySlice<T>& data_, size_t rowSize_,
                           size_t numRows_)
      : data_(data_), rowSize_(rowSize_), numRows_{numRows_} {
    JPP_DCHECK_EQ(data_.size(), rowSize_ * numRows_);
  }

  const T& operator[](size_type idx) const { return data_.at(idx); }
  const T& at(size_type idx) const { return data_.at(idx); }
  const T& at(size_t row, size_t col) const { return this->row(row).at(col); }
  util::ArraySlice<T> row(i32 i) const {
    JPP_DCHECK_IN(i, 0, numRows_);
    return util::ArraySlice<T>{data_, rowSize_ * i, rowSize_};
  }

  util::ArraySlice<T> data() const { return util::ArraySlice<T>{data_}; }

  ConstSliceable<T> topRows(size_t nrows) const {
    JPP_DCHECK_GE(nrows, 0);
    JPP_DCHECK_LE(nrows, numRows_);
    underlying slice{data_, 0, rowSize_ * nrows};
    return ConstSliceable<T>{slice, rowSize_, nrows};
  }

  ConstSliceable<T> rows(size_t start, size_t end) const {
    JPP_DCHECK_IN(start, 0, numRows_);
    JPP_DCHECK_LE(start, end);
    JPP_DCHECK_LE(end, numRows_);
    auto numElem = end - start;
    underlying slice{data_, start * rowSize_, rowSize_ * numElem};
    return ConstSliceable<T>{slice, rowSize_, numElem};
  }

  size_t size() const { return data_.size(); }
  size_t rowSize() const { return rowSize_; }
  size_t numRows() const { return numRows_; }

  typename underlying::const_iterator begin() const { return data_.begin(); }
  typename underlying::const_iterator end() const { return data_.end(); }
};

template <typename T>
ConstSliceable<T> Sliceable<T>::topRows(size_t nrows) const {
  JPP_DCHECK_GE(nrows, 0);
  JPP_DCHECK_LE(nrows, numRows_);
  underlying slice{data_, 0, rowSize_ * nrows};
  return ConstSliceable<T>{slice, rowSize_, nrows};
}

template <typename T>
ConstSliceable<T> Sliceable<T>::rows(size_t start, size_t end) const {
  JPP_DCHECK_IN(start, 0, numRows_);
  JPP_DCHECK_LE(start, end);
  JPP_DCHECK_LE(end, numRows_);
  auto numElem = end - start;
  underlying slice{data_, start * rowSize_, rowSize_ * numElem};
  return ConstSliceable<T>{slice, rowSize_, numElem};
}

}  // namespace util
}  // namespace jumanpp

#endif  // JUMANPP_SLICEABLE_ARRAY_H
