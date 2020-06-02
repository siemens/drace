#ifndef DETECTOR_OUTPUT_H
#define DETECTOR_OUTPUT_H

#include <ipc/ExtsanData.h>

#include <detector/Detector.h>
#include <util/LibLoaderFactory.h>
#include <unordered_map>

class DetectorOutput {
  std::unordered_map<uint32_t, void**> tls;

  void** allocate_memory(uint32_t tid) {
    void** mem = new void*;

    tls.insert({tid, mem});
    return mem;
  }

  void fork(uint32_t tid, uint32_t parent_tid) {
    void** mem = allocate_memory(tid);
    _det->fork(parent_tid, tid, mem);
  }

 protected:
  std::shared_ptr<util::LibraryLoader> _libdetector =
      util::LibLoaderFactory::getLoader();
  std::unique_ptr<Detector> _det;

 public:
  explicit DetectorOutput(const char* detector) {
    if (!_libdetector->load(detector)) {
      throw std::runtime_error("could not load library");
    }
    decltype(CreateDetector)* create_detector =
        (*_libdetector)["CreateDetector"];
    if (nullptr == create_detector) {
      throw std::runtime_error("could not bind detector");
    }
    _det = std::unique_ptr<Detector>(create_detector());

    const char* _argv = "";
    _det->init(1, &_argv, callback);
    std::cout << detector << std::endl;
    std::cout << "init_done\n";
  }

  ~DetectorOutput() {
    _det->finalize();
    for (auto it = tls.begin(); it != tls.end(); it++) {
      delete it->second;
    }
    std::cout << "finished ";
  }

  void makeOutput(ipc::event::BufferEntry* buf) {
    switch (buf->type) {
      case ipc::event::Type::FUNCENTER:
        _det->func_enter(*(tls[buf->payload.funcenter.thread_id]),
                         (void*)(buf->payload.funcenter.pc));
        break;
      case ipc::event::Type::FUNCEXIT:
        _det->func_exit(*(tls[buf->payload.funcexit.thread_id]));
        break;
      case ipc::event::Type::MEMREAD:
        _det->read(*(tls[buf->payload.memaccess.thread_id]),
                   (void*)(buf->payload.memaccess.pc),
                   (void*)(buf->payload.memaccess.addr),
                   buf->payload.memaccess.size);
        break;
      case ipc::event::Type::MEMWRITE:
        _det->write(*(tls[buf->payload.memaccess.thread_id]),
                    (void*)(buf->payload.memaccess.pc),
                    (void*)(buf->payload.memaccess.addr),
                    buf->payload.memaccess.size);
        break;
      case ipc::event::Type::ACQUIRE:
        _det->acquire(*(tls[buf->payload.mutex.thread_id]),
                      (void*)(buf->payload.mutex.addr),
                      buf->payload.mutex.recursive, buf->payload.mutex.write);
        break;
      case ipc::event::Type::RELEASE:
        _det->release(*(tls[buf->payload.mutex.thread_id]),
                      (void*)(buf->payload.mutex.addr),
                      buf->payload.mutex.write);
        break;
      case ipc::event::Type::HAPPENSBEFORE:
        _det->happens_before(*(tls[buf->payload.happens.thread_id]),
                             (void*)(buf->payload.happens.id));
        break;
      case ipc::event::Type::HAPPENSAFTER:
        _det->happens_after(*(tls[buf->payload.happens.thread_id]),
                            (void*)(buf->payload.happens.id));
        break;
      case ipc::event::Type::ALLOCATION:
        _det->allocate(*(tls[buf->payload.allocation.thread_id]),
                       (void*)(buf->payload.allocation.pc),
                       (void*)(buf->payload.allocation.addr),
                       buf->payload.allocation.size);
        break;
      case ipc::event::Type::FREE:
        _det->deallocate(*(tls[buf->payload.allocation.thread_id]),
                         (void*)(buf->payload.allocation.addr));
        break;
      case ipc::event::Type::FORK:
        fork(buf->payload.forkjoin.child, buf->payload.forkjoin.parent);
        break;
      case ipc::event::Type::JOIN:
        _det->join(buf->payload.forkjoin.parent, buf->payload.forkjoin.child);

        delete tls[buf->payload.forkjoin.child];  // delete 'local' tls
        tls.erase(buf->payload.forkjoin.child);
        break;
      case ipc::event::Type::DETACH:
        _det->detach(*(tls[buf->payload.detachfinish.thread_id]),
                     buf->payload.detachfinish.thread_id);
        break;
      case ipc::event::Type::FINISH:
        _det->finish(*(tls[buf->payload.detachfinish.thread_id]),
                     buf->payload.detachfinish.thread_id);
        delete tls[buf->payload.forkjoin.child];  // delete 'local' tls
        tls.erase(buf->payload.forkjoin.child);
        break;
      default:
        std::cout << "ERROR";
    }
  }

  static void callback(const Detector::Race* race) {
    static uintptr_t s1 = 0, s2 = 0;
    if (s1 != race->first.stack_trace[0] && s2 != race->second.stack_trace[0]) {
      static int i = 0;
      s1 = race->first.stack_trace[0];
      s2 = race->second.stack_trace[0];
      std::cout << "pc:   " << (void*)(s1) << " " << (void*)(s2) << std::endl
                << "addr: " << (void*)(race->first.accessed_memory) << std::endl
                << "rw:   " << (race->first.write ? "write" : "read") << " "
                << (race->second.write ? "write" : "read") << std::endl
                << "tid:  " << std::hex << race->first.thread_id << " "
                << std::hex << race->second.thread_id << std::endl;
      std::cout << "race: " << ++i << "\n";
    }
  }
};

#endif  // DETECTOR_OUTPUT_H
