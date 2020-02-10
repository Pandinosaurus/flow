/**
 * @copyright 2020 Fetch Robotics Inc.
 * @author Brian Cairl
 *
 * @file dispatch.h
 * @brief Defines a standard stamped data dispatching type
 */
#ifndef FLOW_CAPTOR_DISPATCH_H
#define FLOW_CAPTOR_DISPATCH_H

// C++ Standard Library
#include <chrono>
#include <ostream>
#include <limits>
#include <type_traits>
#include <utility>

namespace flow
{

/**
 * @brief Helper struct used to specify stamp attributes
 *
 *        Must be specialized for non-integral sequence stamp types with the same fields:
 *        - <code>min</code> : the minimum value of \p StampT
 *        - <code>max</code> : the maximum value of \p StampT
 *        - <code>offset_type</code> : associated offset/duration type
 *
 * @tparam StampT  sequence stamp type
 */
template<typename StampT>
struct StampTraits
{
  static_assert(std::is_fundamental<StampT>(), "'StampT' must be fundemental type");

  /// Stamp type
  using stamp_type = StampT;

  /// Associated duration/offset type
  using offset_type = typename std::make_signed<StampT>::type;

  /// Returns minimum stamp value
  static constexpr StampT min() { return std::numeric_limits<StampT>::min();};

  /// Returns maximum stamp value
  static constexpr StampT max() { return std::numeric_limits<StampT>::max();};
};


/**
 * @brief Helper struct used to specify stamp attributes for <chrono> time types
 *
 * @tparam ClockT     clock on which this time point is measured
 * @tparam DurationT  <code>std::chrono::duration</code> type used to measure the time since epoch
 */
template<typename ClockT, typename DurationT>
struct StampTraits<std::chrono::time_point<ClockT, DurationT>>
{
  /// Stamp type
  using stamp_type = std::chrono::time_point<ClockT, DurationT>;

  /// Associated duration/offset type
  using offset_type = DurationT;

  /// Returns minimum stamp value
  static constexpr stamp_type min() { return stamp_type::min();};

  /// Returns maximum stamp value
  static constexpr stamp_type max() { return stamp_type::max();};
};


/**
 * @brief Dispatch data wrapper
 *
 *        Custom "dispatching" objects may be used with this library in lieu of
 *        the standard <code>Dispatch</code> template provided in this file, so
 *        long as they implement the following methods:
 *        - <code>DispatchT::stamp()</code>: returns data sequencing stamp
 *        - <code>DispatchT::data()</code>: returns an immutable reference to the data itself
 *        - Standard relational operators <code><, >, <=, >=, ==, !=</code>. Note that the
 *          provided <code>Dispatch</code> template only considers the data sequencing stamp
 *          when performing comparisons, as relational operators are mainly meant for data
 *          ordered purposes. Additionally, data elements with different stamps are to be
 *          considered unique, though repeating stamps are allowed in flow capture queues.
 * \n
 *        Custom dispatch types must also specialize the <code>DispatchTraits</code> helper
 *        struct, which is used to provide core <code>stamp_type</code> and <code>value_type</code>
 *        to other dependent objects throughout the library.
 *
 * @tparam StampT  sequencing stamp type; used for data ordering (e.g. time, sequence number, etc.)
 * @tparam ValueT  data value type; must be copyable
 */
template<typename StampT, typename ValueT>
class Dispatch
{
public:
  Dispatch() = default;

  /**
   * @brief Dispatch value constructor (move enabled)
   */
  template<typename... ValueArgsTs>
  explicit Dispatch(const StampT& stamp, ValueArgsTs&&... value_args) :
    stamp_{stamp},
    value_{std::forward<ValueArgsTs>(value_args)...}
  {}

  /**
   * @brief Returns sequencing stamp associated with data element
   */
  inline const StampT& stamp() const
  {
    return stamp_;
  }

  /**
   * @brief Returns const reference to underlying data element
   */
  inline const ValueT& data() const
  {
    return value_;
  }

  /**
   * @brief LT relational overload
   */
  inline bool operator<(const Dispatch& other) const
  {
    return this->stamp_ < other.stamp_;
  }

private:
  /// Sequencing stamp associated with data
  StampT stamp_;

  /// Data element
  ValueT value_;

  /**
   * @brief Output stream overload for <code>Dispatch</code> codes
   * @param[in,out] os  output stream
   * @param dispatch  dispatch object
   * @return os
   */
  friend inline std::ostream& operator<<(std::ostream& os, const Dispatch& dispatch)
  {
    return os << "stamp: " << dispatch.stamp_ << "\nvalue: " << dispatch.value_;
  }
};


/**
 * @brief Dispatch type traits struct
 */
template<typename DispatchT>
struct DispatchTraits
#ifndef DOXYGEN_SKIP
;
template<typename StampT, typename ValueT>
struct DispatchTraits<Dispatch<StampT, ValueT>>
#endif  // DOXYGEN_SKIP
{
  /// Dispatch stamp type
  using stamp_type = StampT;

  /// Dispatch data type
  using value_type = ValueT;

  /// Associated duration/offset type
  using offset_type = typename StampTraits<StampT>::offset_type;
};


/**
 * @brief Data capture/sequencing information
 *
 * @tparam StampT  sequence stamp type
 */
template<typename StampT>
struct CaptureRange
{
  /// Target time stamp produced from captured data associated with oldest captured element
  StampT lower_stamp;

  /// Target time stamp produced from captured data associated with newest captured element
  StampT upper_stamp;

  /**
   * @brief Sequencing range constructor
   * @param _lower_stamp  lower sequencing stamp bound
   * @param _upper_stamp  upper sequencing stamp bound
   * @param _newest_follower_stamp  stamp used to limit difference in current driving
   *                                stamp and oldest sequence stamp in following captor
   */
  CaptureRange(const StampT _lower_stamp = StampTraits<StampT>::min(),
               const StampT _upper_stamp = StampTraits<StampT>::max()) :
    lower_stamp{_lower_stamp},
    upper_stamp{_upper_stamp}
  {}

  /**
   * @brief Output stream overload for <code>CaptureRange</code> codes
   * @param[in,out] os  output stream
   * @param range  capture stamp range
   * @return os
   */
  friend inline std::ostream& operator<<(std::ostream& os, const CaptureRange& range)
  {
    return os << "lower_stamp: " << range.lower_stamp << ", upper_stamp: " << range.upper_stamp;
  }
};

}  // namespace flow

#endif  // FLOW_CAPTOR_DISPATCH_H
