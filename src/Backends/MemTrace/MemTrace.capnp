@0xc8b46734edf9fd25;

struct MemTraceStream {
  struct Mem {

    enum MemType {
      none  @0;
      read  @1;
      write @2;
    }

    type @0:MemType;
    size @1:UInt8;
    addr @2:UInt64;

  }

  events @0 :List(Mem);
}
