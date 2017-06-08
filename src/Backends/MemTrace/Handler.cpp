#include "Handler.hpp"
#include "Core/SigiLog.hpp"
#include "MemTrace.capnp.h"
#include <cassert>

using namespace SigiLog; // console logging
namespace MemTrace
{

//-----------------------------------------------------------------------------
/** Synchronization Event Handling **/
auto Handler::onSyncEv(const sigil2::SyncEvent &ev) -> void
{
    if (ev.type() == SGLPRIM_SYNC_SWAP)
        onSwap(ev.data());
}


//-----------------------------------------------------------------------------
/** Compute Event Handling **/
/*
auto Handler::onCompEv(const sigil2::CompEvent &ev) -> void
{
}
*/


//-----------------------------------------------------------------------------
/** Memory Event Handling **/
auto Handler::onMemEv(const sigil2::MemEvent &ev) -> void
{
    assert(memtrace);
    if (ev.isLoad())
        memtrace->flush(Event::MemType::READ, ev.addr(), ev.bytes());
    else if (ev.isStore())
        memtrace->flush(Event::MemType::WRITE, ev.addr(), ev.bytes());
}


//-----------------------------------------------------------------------------
/** Context Event Handling (instructions) **/
/*
auto EventHandlers::onCxtEv(const sigil2::CxtEvent &ev) -> void
{
    if (ev.type() == CxtTypeEnum::SGLPRIM_CXT_INSTR)
        cachedTCxt->onInstr();
}
*/


//-----------------------------------------------------------------------------
/** Synchronization Event Helpers **/
auto Handler::onSwap(unsigned newTID) -> void
{
    assert(newTID > 0);

    if (currentTID != newTID)
    {
        if (memtraces.find(newTID) == memtraces.cend())
        {
            memtraces.emplace(std::piecewise_construct,
                              std::forward_as_tuple(newTID),
                              std::forward_as_tuple(newTID, "."));
        }

        memtrace = &memtraces.at(newTID);
        currentTID = newTID;
    }
}

}; //end namespace STGen
