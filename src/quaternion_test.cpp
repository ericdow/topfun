#include <iostream>
#include <boost/math/quaternion.hpp>

int main(int argc, char* argv[]) {
  typedef boost::math::quaternion<float> quat;
  quat q0(1,2,3,4), q1(1,2,3,4);
  q1 += q0;
  std::cout << q1 << std::endl;
}
