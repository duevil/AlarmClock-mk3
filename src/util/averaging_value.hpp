#ifndef AVERAGING_VALUE_HPP
#define AVERAGING_VALUE_HPP

#include <array>
#include <cmath>
#include <limits>
#include <type_traits>


/**
 * Utility class for averaging values
 * @tparam T The underlying value type; must be arithmetic
 * @tparam Size The size of the underlying buffer used for averaging
 */
template <typename T, std::size_t Size> requires std::is_arithmetic_v<T>
class AveragingValue
{
    static constexpr T nan = std::numeric_limits<T>::quiet_NaN();

    std::array<T, Size> data;
    std::size_t index = 0;
    std::size_t filled = 0;
    T average = nan;

public:
    /**
     * Fills the underlying buffer with NaN
     */
    constexpr AveragingValue() noexcept { data.fill(nan); }

    /**
     * Pushes a new value to the buffer and recomputes the average
     * @param value The value to push to the buffer
     * @return A reference to this instance
     */
    AveragingValue& push(T value)
    {
        if (!std::isnan(value))
        {
            data[index] = value;
            index = (index + 1) % Size;
            if (filled < Size) ++filled;

            // recompute average (simple O(Size) scan – fine for small buffers)
            T sum = T{};
            for (std::size_t i = 0; i < filled; ++i)
            {
                sum += data[i];
            }
            average = sum / static_cast<T>(filled);
        }
        return *this;
    }

    /**
     * Stream operator to push a new value to the buffer and recomputes the average
     * @param value The value to push to the buffer
     * @return A reference to this instance
     */
    AveragingValue& operator<<(T value) { return push(value); }

    /**
     * Implicit conversion to the underlying average
     */
    operator T() const noexcept { return average; } // NOLINT(*-explicit-constructor)

    /**
     * Get the underlying average value
     * @return The average value as the underlying type
     */
    T get() const noexcept { return average; }
};


#endif // AVERAGING_VALUE_HPP
