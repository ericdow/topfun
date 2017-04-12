#include <iostream>
#include <Eigen/Dense>

int main(int argc, char* argv[]) {
  Eigen::Vector3d v(1,2,3), w(1,0,0);
  std::cout << v.cross(w) << std::endl;
}
