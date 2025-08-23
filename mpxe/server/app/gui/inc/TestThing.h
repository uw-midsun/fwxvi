#pragma once
#include <string>

namespace mpxe {

/**
 * @brief Tiny test class to verify headers/libs build correctly.
 */
class TestThing {
public:
  explicit TestThing(std::string name = "TestThing");

  /// Returns the name passed in the constructor.
  const std::string& name() const noexcept;

  /// Simple add to verify linking.
  int add(int a, int b) const noexcept;

  /// Returns a friendly message (uses add()).
  std::string hello() const;

private:
  std::string m_name; 
};

}  // namespace mpxe
