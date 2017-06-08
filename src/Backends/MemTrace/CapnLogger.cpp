#include "CapnLogger.hpp"
#include "Core/SigiLog.hpp"
#include <memory>


using SigiLog::fatal;

//-----------------------------------------------------------------------------
/** CapnProto -> Gzip file **/
namespace kj
{

class GzOutputStream : public OutputStream
{
    /* Based off of FdOutputStream in capnproto library */
  public:
    explicit GzOutputStream(gzFile fz) : fz(fz) {}
    KJ_DISALLOW_COPY(GzOutputStream);
    ~GzOutputStream() noexcept(false) {}

    void write(const void* buffer, size_t size) override
    {
        int ret = gzwrite(fz, buffer, size);
        if (ret == 0)
            fatal("error writing gzipped capnproto serializaton");
    }

  private:
    gzFile fz;
};

}; //end namespace kj


namespace capnp
{

inline void writePackedMessageToGz(gzFile fz, MessageBuilder &message)
{
    /* Based off of writePackedMessageToFd in capnproto library */

    kj::GzOutputStream output(fz);
    writePackedMessage(output, message.getSegmentsForOutput());
}

}; //end nampespace capnp


//-----------------------------------------------------------------------------
/** CapnProto Logging **/
namespace MemTrace
{

namespace
{


template <typename EventStream, typename OrphanagePtr, typename OrphanList>
auto flushOrphans(OrphanagePtr flushedOrphanage, OrphanList flushedOrphans, gzFile fz) -> bool
{
    /* need to keep the orphanage alive until it's flushed */
    (void)flushedOrphanage;

    /* create the message now that we have a fixed length */
    ::capnp::MallocMessageBuilder message;
    auto eventStreamBuilder = message.initRoot<EventStream>();
    auto eventsBuilder = eventStreamBuilder.initEvents(flushedOrphans.size());

    for (unsigned i=0; i<flushedOrphans.size(); ++i)
    {
        auto reader = flushedOrphans[i].getReader();
        eventsBuilder.setWithCaveats(i, reader);
    }

    ::capnp::writePackedMessageToGz(fz, message);

    /* burn down the orphanage and orphans */
    flushedOrphans.clear(); /* kill orphans first,
                               otherwise we die when we run into the burning orphanage
                               to kill the orphans */
    return true;
}

}; //end namespace


//-----------------------------------------------------------------------------
/** Multiple reads/writes compressed **/
CapnLogger::CapnLogger(unsigned tid, std::string outputPath)
{
    assert(tid >= 1);

    /* initialize orphanage */
    orphanage = std::make_unique<::capnp::MallocMessageBuilder>();

    /* nothing being copied yet */
    doneCopying = std::async([]{return true;});

    auto filePath = (outputPath + "/memtrace-" + std::to_string(tid) +
                     ".capn.bin.gz");
    fz = gzopen(filePath.c_str(), "wb");
    if (fz == NULL)
        fatal(std::string("opening gzfile: ") + strerror(errno));
}


CapnLogger::~CapnLogger()
{
    flushOrphansNow();
    int ret = gzclose(fz);
    if (ret != Z_OK)
        fatal(std::string("closing gzfile: ") + strerror(errno));
}


    //auto flush(Event::MemType type, Addr addr, size_t size) -> void;
auto CapnLogger::flush(Event::MemType type, uintptr_t addr, size_t size) -> void
{
    auto orphan = orphanage->getOrphanage().newOrphan<Event>();
    auto mem = orphan.get();
    mem.setType(type);
    mem.setSize(size);
    mem.setAddr(addr);
    orphans.emplace_back(std::move(orphan));
    flushOrphansOnMaxEvents();
}

auto CapnLogger::flushOrphansOnMaxEvents() -> void
{
    assert(events <= maxEventsPerMessage);
    if (++events == maxEventsPerMessage)
    {
        flushOrphansAsync();
        events = 0;
    }
}


auto CapnLogger::flushOrphansNow() -> void
{
    assert(events <= maxEventsPerMessage);
    if (events > 0)
    {
        flushOrphansAsync();
        doneCopying.get(); // blocking flush
        events = 0;
    }
}


auto CapnLogger::flushOrphansAsync() -> void
{
    /* asynchronously copy orphans and flush */
    assert(doneCopying.valid());
    doneCopying.get();
    doneCopying = std::async(std::launch::async,
                             flushOrphans<MemTraceStream, OrphanagePtr, OrphanList>,
                             std::move(orphanage), std::move(orphans), fz);
    /* start a new orphanage */
    orphans.clear();
    orphanage = std::make_unique<::capnp::MallocMessageBuilder>();
}

}; //end namespace STGen
