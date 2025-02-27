#ifndef EMBEDDING_CMREMBEDDING_HPP_
#define EMBEDDING_CMREMBEDDING_HPP_

#include "XACC.hpp"
#include "EmbeddingAlgorithm.hpp"
#include "find_embedding.hpp"

namespace xacc {
namespace cmr {

class XACCInteractions : public find_embedding::LocalInteraction {
public:
  bool _canceled = false;
  void cancel() { _canceled = true; }

private:
  virtual void displayOutputImpl(const std::string &mess) const {
    xacc::info(mess);
  }
  virtual bool cancelledImpl() const { return _canceled; }
};

class CMREmbedding : public xacc::quantum::EmbeddingAlgorithm {
public:
  virtual xacc::quantum::Embedding
  embed(std::shared_ptr<xacc::Graph> problem,
        std::shared_ptr<xacc::Graph> hardware,
        std::map<std::string, std::string> params =
            std::map<std::string, std::string>());

  virtual const std::string name() const { return "cmr"; }

  virtual const std::string description() const {
    return "An EmbeddingAlgorithm wrapping the D-Wave MinorMiner.";
  }
};

} // namespace cmr
} // namespace xacc

#endif
