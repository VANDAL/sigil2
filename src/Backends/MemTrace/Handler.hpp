#ifndef MEMTRACE_EVENTHANDLERS_H
#define MEMTRACE_EVENTHANDLERS_H

#include "Core/Backends.hpp"
#include "CapnLogger.hpp"
#include <unordered_map>

namespace MemTrace
{

class Handler : public BackendIface
{
  public:
    Handler() {}
    Handler(const Handler &) = delete;
    Handler &operator=(const Handler &) = delete;

    virtual auto onSyncEv(const sigil2::SyncEvent &ev) -> void override;
    //virtual auto onCompEv(const sigil2::CompEvent &ev) -> void override;
    virtual auto onMemEv(const sigil2::MemEvent &ev) -> void override;
    //virtual auto onCxtEv(const sigil2::CxtEvent &ev) -> void override;
    /* Sigil2 event hooks */

  private:
    auto onSwap(unsigned newTID) -> void;

    unsigned currentTID{0};
    CapnLogger *memtrace{nullptr};
    std::unordered_map<unsigned, CapnLogger> memtraces;
};

}; //end namespace MemTrace

#endif
