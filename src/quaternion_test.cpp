#include <iostream>
#include <memory>
#include <boost/math/quaternion.hpp>

struct map {
  std::unique_ptr<int> pint;
};

map construct_map() {
  map m;
  m.pint = std::unique_ptr<int>(new int(10));  
  return m; 
}

int main(int argc, char* argv[]) {
  typedef boost::math::quaternion<float> quat;
  quat q0(1,2,3,4), q1(1,2,3,4);
  q1 += q0;
  std::cout << q1 << std::endl;
  map foo = construct_map();
  std::cout << *(foo.pint) << std::endl;
}
