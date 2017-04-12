#include <iostream>
#include <Eigen/Dense>

int main(int argc, char* argv[]) {
  Eigen::Matrix<float,3,1> v(1,2,3), w(1,0,0);
  std::cout << v.cross(w) << std::endl;
}
