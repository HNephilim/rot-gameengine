//===------------ profiler.h - Chrome Tracing Profiler library ------------===//
//
// Credits to Guilherme Valarini
// Original implementation:
// https://gitlab.com/ompcluster/llvm-project/-/tree/main/openmp/libomptarget/utils
//
//===----------------------------------------------------------------------===//
//
// Declarations for profiling an application into a Chrome Trace.
//
//===----------------------------------------------------------------------===//

#ifndef _GENERIC_PROFILER_H
#define _GENERIC_PROFILER_H

#ifdef GENERIC_PROFILER

#include <limits>
#include <mutex>
#include <string>

namespace _profiler {

/// 32-Bit field data attributes controlling information collected on profiling.
enum ProfileLevel : uint32_t {
    // Collect commonly used information.
    PROF_LVL_USER = 0x0001,
    // Collect all informations.
    PROF_LVL_ALL = 0xffffffff,
};

inline uint32_t getProfileLevel() {
    static uint32_t prof_lvl = PROF_LVL_USER;
    static std::once_flag Flag{};
    std::call_once(Flag, []() {
        if (char *env_str = getenv("GP_PROFILE_LEVEL"))
            prof_lvl = std::stoul(env_str, nullptr, 16);
    });

    return prof_lvl;
}

/// Initialize the profiler global context for the current process.
///
/// \param process_name shown in the tracing timeline.
/// \param index used to order multiple processes in the same timeline.
void initProcessProfiler(std::string &&process_name = "", int index = std::numeric_limits<short>::max());

/// Initialize the profiler local context for the calling thread.
///
/// \param thread_name shown in the tracing timeline.
/// \param index used to order multiple threads in the same timeline.
void initThreadProfiler(std::string &&thread_name = "", int index = std::numeric_limits<short>::max());

/// Begin a profile point.
///
/// The new profiler point is added to the top of the local thread context stack
/// and it must be ended by calling endProfilePoint().
///
/// \param name of the profile point.
/// \param details of the current profile point in a stringified JSON format.
void beginProfilePoint(const std::string &&name = "", const std::string &&details = "{}");

/// End the most recent profile point.
///
/// This function ends the most recent profile point present at the top of the
/// calling thread context stack. It must always be called after a
/// beginProfilePoint() call.
void endProfilePoint();

/// Dump the profiler global context into a tracing file.
///
/// \param filename desired for the dumped tracing file.
void dumpTracingFile();

/// Scoped profiler point.
///
/// Helper class that calls beginProfilePoint() at its contruction and
/// endProfilePoint() at its destruction.
struct ScopedProfilePoint {
    // The class is non-copyable and non-movable.
    ScopedProfilePoint() = delete;
    ScopedProfilePoint(const ScopedProfilePoint &) = delete;
    ScopedProfilePoint &operator=(const ScopedProfilePoint &) = delete;
    ScopedProfilePoint(ScopedProfilePoint &&) = delete;
    ScopedProfilePoint &operator=(ScopedProfilePoint &&) = delete;

    // Check if the ProfilePoint was started due to the Prof_LVL police
    bool started = false;
    /// Begin the scoped profiler point.
    ///
    /// The new profiler point is added to the top of the local thread context
    /// stack and it will be ended at ~ScopedProfilePoint().
    ///
    /// \param name of the profile point.
    /// \param details of the current profile point in a stringified JSON format.
    ScopedProfilePoint(const ProfileLevel prof_lvl, const std::string &&name, const std::string &&details = "{}") {
        if ((started = (getProfileLevel() >= prof_lvl)))
            beginProfilePoint(std::move(name), std::move(details));
    }

    /// End the scoped profiler point.
    ///
    /// End the profiler point at the top of the local thread context.
    ~ScopedProfilePoint() {
        if (started)
            endProfilePoint();
    }
};

} // namespace _profiler

// Usage macros
// Always use this macros to interact with the profiler.
// =============================================================================
#define _CONCAT(a, b) a##b
#define CONCAT(a, b) _CONCAT(a, b)
#define GEN_UNQ_SYM() CONCAT(__VAR_, __LINE__)

#define PROF_LVL_USER _profiler::PROF_LVL_USER
#define PROF_LVL_ALL _profiler::PROF_LVL_ALL
#define CHECK_PROF_LVL(PROF_LVL) (_profiler::getProfileLevel() >= PROF_LVL)
#define PROF_INIT_PROC(...) _profiler::initProcessProfiler(__VA_ARGS__)
#define PROF_INIT_THD(...) _profiler::initThreadProfiler(__VA_ARGS__)
#define PROF_BEGIN(PROF_LVL, ...)                                                                                                          \
    if (CHECK_PROF_LVL(PROF_LVL))                                                                                                          \
    _profiler::beginProfilePoint(__VA_ARGS__)
#define PROF_END(PROF_LVL)                                                                                                                 \
    if (CHECK_PROF_LVL(PROF_LVL))                                                                                                          \
    _profiler::endProfilePoint()
#define PROF_BEGIN_NEXT(...)                                                                                                               \
    _profiler::endProfilePoint();                                                                                                          \
    _profiler::beginProfilePoint(__VA_ARGS__)
#define PROF_DUMP_TRACE() _profiler::dumpTracingFile()
#define PROF_SCOPED(PROF_LVL, ...) _profiler::ScopedProfilePoint GEN_UNQ_SYM()(PROF_LVL, __VA_ARGS__)
#else

#define PROF_INIT_PROC(...)                                                                                                                \
    {}
#define PROF_INIT_THD(...)                                                                                                                 \
    {}
#define PROF_BEGIN(PROF_LVL, ...)                                                                                                          \
    {}
#define PROF_END(PROF_LVL)                                                                                                                 \
    {}
#define PROF_BEGIN_NEXT(...)                                                                                                               \
    {}
#define PROF_DUMP_TRACE(filename)                                                                                                          \
    {}
#define PROF_SCOPED(PROF_LVL, ...)                                                                                                         \
    {}
#define PROF_LVL_USER
#define PROF_LVL_ALL

#endif // GENERIC_PROFILER

#endif // _GENERIC_PROFILER_H
