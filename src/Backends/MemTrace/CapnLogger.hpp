#ifndef MEMTRACE_CAPNLOGGER_H
#define MEMTRACE_CAPNLOGGER_H

#include "Core/Primitive.h"
#include "MemTrace.capnp.h"
#include <capnp/message.h>
#include <capnp/serialize-packed.h>
#include <zlib.h>
#include <future>
#include <vector>

/* Uses CapnProto library (https://capnproto.org)
 * to serialize the event stream to a binary representation.
 * Binary serialization schemes may provide faster parsing
 * of the event stream and better compression */

namespace MemTrace
{
using Event = MemTraceStream::Mem;
using OrphanagePtr = std::unique_ptr<::capnp::MallocMessageBuilder>;
using OrphanList = std::vector<::capnp::Orphan<Event>>;

class CapnLogger
{
  public:
    CapnLogger(unsigned tid, std::string outputPath);
    CapnLogger(const CapnLogger &other) = delete;
    ~CapnLogger();

    auto flush(Event::MemType type, uintptr_t addr, size_t size) -> void;

  private:
    auto flushOrphansOnMaxEvents() -> void;
    auto flushOrphansNow() -> void;
    auto flushOrphansAsync() -> void;

    static constexpr unsigned maxEventsPerMessage = 500000;

    OrphanagePtr orphanage;
    OrphanList orphans;
    /* use an orphanage because we don't know the event count ahead of time */

    gzFile fz;
    unsigned events{0};

    std::future<bool> doneCopying;
    /* Use as a barrier to ensure one capnproto
     * message gets copied at a time */
};

}; //end namespace MemTrace

#endif
