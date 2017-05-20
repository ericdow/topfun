#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/unordered_map.hpp>

namespace TopFun {

struct NeighborLoD {
  NeighborLoD(unsigned short lod_c, unsigned short lod_n, unsigned short lod_e,
      unsigned short lod_s, unsigned short lod_w)
    : tuple(lod_c,lod_n,lod_e,lod_s,lod_w) {}
  boost::tuples::tuple<unsigned short, unsigned short, unsigned short, 
    unsigned short, unsigned short> tuple;
};

inline bool operator==(const NeighborLoD &a, const NeighborLoD &b) {
  return a.tuple == b.tuple;
}

inline bool operator!=(const NeighborLoD &a, const NeighborLoD &b) {
  return a.tuple != b.tuple;
}

inline std::size_t hash_value(const NeighborLoD &e) {
  std::size_t seed = 0;
  boost::hash_combine(seed, e.tuple.get<0>());
  boost::hash_combine(seed, e.tuple.get<1>());
  boost::hash_combine(seed, e.tuple.get<2>());
  boost::hash_combine(seed, e.tuple.get<3>());
  boost::hash_combine(seed, e.tuple.get<4>());
  return seed;
}

} // End namespace TopFun
