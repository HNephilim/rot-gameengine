#include <app/first_app.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <profiler/profiler.hpp>

/* int main(int argc, char *argv[]) { */
/*     PROF_INIT_PROC("Initializing App"); */
/*     rot::FirstApp app{}; */
/**/
/*     try { */
/*         app.run(); */
/*     } catch (const std::exception &e) { */
/*         std::cerr << e.what() << '\n'; */
/*         return 1; */
/*     } */
/**/
/*     PROF_DUMP_TRACE("bin\\profile"); */
/*     return 0; */
/* } */
#include <iostream>

#include "profiler/profiler.hpp"

#include <stdlib.h>
#include <string>
#include <thread>
#include <vector>

using namespace std;

void foo(const char *str, long iterations) {
    cout << str << " started execution\n";
    PROF_INIT_THD(str);
    PROF_SCOPED(PROF_LVL_ALL, "Thread FOO");

    int a = 1;
    for (long i = 0; i < iterations; i++)
        for (long j = 0; j < iterations; j++)
            a *= a;

    cout << str << " finished execution\n";
}

void bar(const char *str, long iterations) {
    cout << str << " started execution\n";
    PROF_INIT_THD(str);
    PROF_SCOPED(PROF_LVL_ALL, "Thread BAR");

    int a = 1;
    for (long i = 0; i < iterations; i++)
        for (long j = 0; j < iterations; j++)
            a *= a;

    cout << str << " finished execution\n";
}

int main() {
    if (const char *lvl = getenv("GP_PROFILE_LEVEL")) {
        cout << "Env = " << lvl << endl;
    }

    PROF_INIT_PROC("Main Process");

    thread a(foo, "Thread A", 100);
    thread b(bar, "Thread B", 10);

    a.join();
    b.join();

    PROF_DUMP_TRACE();

    // json::JSON *dump;
    // dump = new json::JSON("test.json");

    // dump->object(
    //     {[&] {
    //        dump->attribute("a", "t");
    //        dump->attribute("y", "t");
    //      },
    //      [&] { dump->attribute("b", "t"); },
    //      [&] {
    //        dump->attributeArray("Events", {
    //                                           [&] { dump->attribute("c", "t"); },
    //                                           [&] { dump->attribute("c", "t"); },
    //                                       });
    //      },
    //      [&] { dump->attribute("c", "t"); }, [&] { dump->attribute("d", "t"); },
    //      [&] { dump->attribute("e", "t"); },
    //      [&] {
    //        dump->attributeArray(
    //            "Ojects", {
    //                          [&] {
    //                            dump->attributeObject(
    //                                "Obj1", {
    //                                            [&] { dump->attribute("a", "t"); },
    //                                        });
    //                          },
    //                          [&] {
    //                            dump->attributeObject(
    //                                "Obj2", {
    //                                            [&] { dump->attribute("b", "t"); },
    //                                        });
    //                          },
    //                      });
    //      },
    //      [&] { dump->attribute("f", "t"); }, [&] { dump->attribute("g", "t"); },
    //      [&] {
    //        dump->attributeObject("Obj3", {
    //                                          [&] { dump->attribute("z", "x"); },
    //                                      });
    //      }});

    // delete dump;
    return 0;
}
