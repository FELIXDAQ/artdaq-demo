#ifndef ds50daq_Compression_HuffmanTable_hh
#define ds50daq_Compression_HuffmanTable_hh

#include <memory>
#include <vector>
#include <list>
#include <istream>
#include <string>

#include "ds50daq/Compression/Tree.hh"
#include "ds50daq/Compression/Properties.hh"
#include "ds50daq/Compression/SymCode.hh"
#include "ds50daq/Compression/SymProb.hh"

namespace ds50 {
  class HuffmanTable;

  std::ostream & operator<<(std::ostream & ost, HuffmanTable const & h);
}

class ds50::HuffmanTable {
public:
  typedef std::unique_ptr<Node> Node_ptr;
  typedef std::list<Node *> HeadList;
  typedef std::vector<Node_ptr>NodeVec;

  struct ItPair {
    typedef HeadList::iterator It;
    It low_;
    It high_;

    It left() { return high_; }
    It right() { return low_; }

    ItPair(): low_(), high_() { }
    ItPair(It higher, It lower): low_(lower), high_(higher) { }
  };

  typedef ItPair(*Algo)(HeadList &);

  HuffmanTable(std::istream & training_set, size_t countmax, size_t max_samples);
  HuffmanTable(ADCCountVec const & training_set, size_t countmax);
  HuffmanTable(SymsVec const& sv);

  void extractTable(SymTable & out) const;
  void writeTable(std::string const & filename) const;
  void writeTableReversed(std::string const & filename) const;
  void print(std::ostream & ost) const
  { heads_.front()->print(ost); }

private:
  void initNodes(SymsVec const &);
  void initHeads();
  void constructTree();
  void makeTable(ADCCountVec const &, size_t countmax);

  NodeVec nodes_;
  HeadList heads_;
};

inline
std::ostream &
ds50::operator<<(std::ostream & ost, HuffmanTable const & h)
{
  h.print(ost);
  return ost;
}

#endif /* ds50daq_Compression_HuffmanTable_hh */
