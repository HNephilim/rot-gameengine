//===----------- profiler.cpp - Chrome Tracing Profiler library -----------===//
//
// Credits to Guilherme Valarini
// Original implementation:
// https://gitlab.com/ompcluster/llvm-project/-/tree/main/openmp/libomptarget/utils
//
//===----------------------------------------------------------------------===//
//
// Definitions for profiling an application into a Chrome Trace.
//
//===----------------------------------------------------------------------===//

#include "profiler.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdint.h>
#include <stdio.h>

#ifdef GENERIC_PROFILER

#include <algorithm>
#include <chrono>
#include <list>
#include <mutex>
#include <ratio>
#include <stack>
#include <string>
#include <vector>

#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <windows.h>

#include <processthreadsapi.h>

#include "json.hpp"

namespace _profiler {

// Config parameters.
// =============================================================================
constexpr char default_process_name[] = "Worker Process";
constexpr char default_thread_name[] = "Worker Thread";

// Profiler structures.
// =============================================================================
// Profile entry that measure the time between two points in the program.
namespace chrono = std::chrono;
using ProfilerDuration = chrono::duration<double, std::micro>;

// Convenient wrapper for Process LLVM Lib
namespace id {
struct Process {
    using Pid = unsigned int;
    static unsigned int getProcessId() { return GetCurrentProcessId(); }
};

// Convenient wrapper for Threading LLVM Lib
struct Thread {
    using Tid = unsigned int;
    static unsigned int getThreadId() { return GetCurrentThreadId(); }
};
} // namespace id

struct ProfilerClock {
    using duration = chrono::nanoseconds;
    using rep = duration::rep;
    using period = duration::period;
    using time_point = chrono::time_point<chrono::steady_clock>;

    static const bool is_steady = false;

    static time_point now() noexcept { return chrono::steady_clock::now(); }
};

struct Entry {
    std::string name = "";    // Timeline label
    std::string details = ""; // Detailed description
    ProfilerClock::time_point start = ProfilerClock::time_point();
    ProfilerClock::time_point end = ProfilerClock::time_point();
};

// Profile entry that measure the time between two points in the program.
struct ThreadProfiler {
    std::string name = "";      // Timeline thread name
    id::Thread::Tid tid = 0;    // Thread ID
    int index = 0;              // Order in the thread list
    std::stack<Entry> stack;    // Entries currently active
    std::vector<Entry> entries; // Completed entries
};

struct ProcessProfiler {
    std::string name = "";                     // Timeline process name
    id::Process::Pid pid = 0;                  // Process ID
    int index = 0;                             // Order in the process list
    std::string filename = "";                 // Name for the dumped trace file
    bool enabled = false;                      // Enables the profiler.
    std::list<ThreadProfiler> threads_profile; // Process threads profilers
};

// Profiler global context.
// =============================================================================
// Global profiler context for the process.
static ProcessProfiler *process_profiler;
static std::mutex process_profiler_mtx;
// Local profiler for each thread.
static thread_local ThreadProfiler *thread_profiler;

// Helpers functions.
// =============================================================================
static double toProfileScale(ProfilerClock::time_point tp) {
    return chrono::duration_cast<ProfilerDuration>(tp.time_since_epoch()).count();
}

static double toProfileScale(ProfilerClock::duration d) { return chrono::duration_cast<ProfilerDuration>(d).count(); }

static std::string toLowerSnakeCase(const std::string &str) {
    std::string res = str;
    for (auto &c : res) {
        if (c == ' ') {
            c = '_';
        } else {
            c = std::tolower(c);
        }
    }
    return res;
}

// API functions.
// =============================================================================
void initProcessProfiler(std::string &&process_name, int index) {
    std::unique_lock<std::mutex> process_lk(process_profiler_mtx);

    // Strong check: immediately return if process profiler is already intialized.
    if (process_profiler != nullptr) {
        return;
    }

    if (process_name.empty()) {
        process_name = default_process_name;
    }

    // Create profile filename: filename_prefix + "_" + process_name + ".json"
    std::string filename = "_";
    if (const char *env_str = std::getenv("GP_FILENAME_PREFIX"))
        filename = std::string(env_str) + "_" + toLowerSnakeCase(process_name) + ".json";

    // Create a new process profiler.
    process_profiler = new ProcessProfiler{
        .name = std::move(process_name),
        .pid = id::Process::getProcessId(),
        .index = index,
        .filename = filename,
        .enabled = !filename.empty(),
    };
}

void initThreadProfiler(std::string &&thread_name, int index) {
    // Weak check: init process profiler if it is needed.
    // Avoids double locking `process_profiler_mtx`.
    if (process_profiler == nullptr) {
        initProcessProfiler();
    }

    if (!process_profiler->enabled) {
        return;
    }

    std::unique_lock<std::mutex> process_lk(process_profiler_mtx);

    if (thread_profiler != nullptr) {
        return;
    }

    if (thread_name.empty()) {
        thread_name = default_thread_name;
    }

    // Create new thread profiler and set thread local reference.
    process_profiler->threads_profile.emplace_back(ThreadProfiler{
        .name = std::move(thread_name),
        .tid = id::Thread::getThreadId(),
        .index = index,
    });
    thread_profiler = &process_profiler->threads_profile.back();
}

void beginProfilePoint(const std::string &&name, const std::string &&details) {
    printf("Begin profile point called\n");
    assert(name != "");

    // Weak check: init thread profiler if it is needed.
    // Avoids locking `process_prosfiler_mtx`.
    if (thread_profiler == nullptr) {
        initThreadProfiler();
    }

    if (!process_profiler->enabled) {
        return;
    }

    // Add new entry to local profiler stack.
    thread_profiler->stack.emplace(Entry{
        .name = std::move(name),
        .details = std::move(details),
        .start = ProfilerClock::now(),
    });
}

void endProfilePoint() {
    printf("End profile point called!\n");
    assert(process_profiler != nullptr);

    if (!process_profiler->enabled) {
        return;
    }

    assert(thread_profiler != nullptr);
    assert(!thread_profiler->stack.empty());

    // Finish top stack entry and move it to the entries list.
    Entry &entry = thread_profiler->stack.top();
    entry.end = ProfilerClock::now();
    thread_profiler->entries.emplace_back(entry);
    thread_profiler->stack.pop();
}

void dumpTracingFile() {
    assert(process_profiler != nullptr);

    if (!process_profiler->enabled) {
        return;
    }

    std::unique_lock<std::mutex> process_lk(process_profiler_mtx);

    assert(std::all_of(process_profiler->threads_profile.begin(), process_profiler->threads_profile.end(),
                       [](const ThreadProfiler &tprof) { return tprof.stack.empty(); }));

    // Start JSON file.
    using json = nlohmann::json;

    std::vector<json> entry_vec;
    // Traced Events
    for (const auto &tprof : process_profiler->threads_profile) {
        for (const auto &entry : tprof.entries) {
            json prof_entry;
            prof_entry["ph"] = "X";
            prof_entry["name"] = entry.name;
            prof_entry["pid"] = process_profiler->pid;
            prof_entry["tid"] = static_cast<int64_t>(tprof.tid);
            prof_entry["ts"] = toProfileScale(entry.start);
            prof_entry["dur"] = toProfileScale(entry.end - entry.start);
            prof_entry["args"] = entry.details;

            entry_vec.push_back(prof_entry);
        }
    }
    // Metadata
    // Naming and ordering of processes.
    json metadata_name;
    metadata_name["ph"] = "M";
    metadata_name["name"] = "process_name";
    metadata_name["pid"] = process_profiler->pid;
    metadata_name["args"] = json::object({{"name", process_profiler->name}});

    json metadata_sort_index;
    metadata_sort_index["ph"] = "M";
    metadata_sort_index["name"] = "process_sort_index";
    metadata_sort_index["pid"] = process_profiler->pid;
    metadata_sort_index["args"] = json::object({{"sort_index", process_profiler->index}});
    entry_vec.push_back(metadata_name);
    entry_vec.push_back(metadata_sort_index);

    // Naming and ordering of threads
    for (const auto &tprof : process_profiler->threads_profile) {
        json naming_thread;
        naming_thread["ph"] = "M";
        naming_thread["name"] = "thread_name";
        naming_thread["pid"] = process_profiler->pid;
        naming_thread["tid"] = static_cast<int64_t>(tprof.tid);
        naming_thread["args"] = json::object({{"name", tprof.name}});

        json sorting_thread;
        sorting_thread["ph"] = "M";
        sorting_thread["name"] = "process_sort_index";
        sorting_thread["pid"] = process_profiler->pid;
        sorting_thread["tid"] = static_cast<int64_t>(tprof.tid);
        sorting_thread["args"] = json::object({{"sort_index", tprof.index}});
        entry_vec.push_back(naming_thread);
        entry_vec.push_back(sorting_thread);
    }

    json trace;
    trace["traceEvents"] = entry_vec;

    std::ofstream out("trace.json");
    out << std::setw(4) << trace << std::endl;

    /* json *dump; */
    /* dump = new json::json("profile.json"); // process_profiler->filename); */
    /* dump->object({[&] { */
    /*     // chrometracing array of events (include traced events and metadata */
    /*     // ones). */
    /*     dump->attributearray( */
    /*         dump = new json::json("profile.json"); // process_profiler->filename); */
    /*         {[&] {                                 // traced events. */
    /*              for (const auto &tprof : process_profiler->threads_profile) { */
    /*                  for (const auto &entry : tprof.entries) { */
    /*                      dump->object({[&] { dump->attribute("ph", "x"); }, [&] { dump->attribute("name", entry.name); }, */
    /*                                    [&] { dump->attribute("pid", process_profiler->pid); }, */
    /*                                    [&] { dump->attribute("tid", static_cast<int64_t>(tprof.tid)); }, */
    /*                                    [&] { dump->attribute("ts", toprofilescale(entry.start)); }, */
    /*                                    [&] { dump->attribute("dur", toprofilescale(entry.end - entry.start)); }, */
    /*                                    [&] { dump->attribute("args", entry.details); }}); */
    /*                  } */
    /*              } */
    /*          }, */
    /*          [&] { // metadata. naming and ordering of processes. */
    /*              dump->object({[&] { dump->attribute("ph", "m"); }, [&] { dump->attribute("name", "process_name"); }, */
    /*                            [&] { dump->attribute("pid", process_profiler->pid); }, */
    /*                            [&] { dump->attributeobject("args", {[&] { dump->attribute("name", process_profiler->name); }}); }}); */
    /*          }, */
    /*          [&] { */
    /*              dump->object({[&] { dump->attribute("ph", "m"); }, [&] { dump->attribute("name", "process_sort_index"); }, */
    /*                            [&] { dump->attribute("pid", process_profiler->pid); }, */
    /*                            [&] { dump->attributeobject("args", {[&] { dump->attribute("sort_index", process_profiler->index); }});
     * }}); */
    /*          }, */
    /*          [&] { */
    /*              // naming and ordering of threads. */
    /*              for (const auto &tprof : process_profiler->threads_profile) { */
    /*                  dump->object( */
    /*                      {[&] { */
    /*                           dump->object({[&] { dump->attribute("ph", "m"); }, [&] { dump->attribute("name", "thread_name"); }, */
    /*                                         [&] { dump->attribute("pid", process_profiler->pid); }, */
    /*                                         [&] { dump->attribute("tid", static_cast<int64_t>(tprof.tid)); }, */
    /*                                         [&] { dump->attributeobject("args", {[&] { dump->attribute("name", tprof.name); }}); }}); */
    /*                       }, */
    /*                       [&] { */
    /*                           dump->object({[&] { dump->attribute("ph", "m"); }, [&] { dump->attribute("name", "thread_sort_index"); },
     */
    /*                                         [&] { dump->attribute("pid", process_profiler->pid); }, */
    /*                                         [&] { dump->attribute("tid", static_cast<int64_t>(tprof.tid)); }, */
    /*                                         [&] { dump->attributeobject("args", {[&] { dump->attribute("sort_index", tprof.index); }});
     * }}); */
    /*                       }}); */
    /*              } */
    /*          }}); */
    /* }}); */
    /* delete dump; */
}

} // namespace _profiler

#endif // GENERIC_PROFILER
