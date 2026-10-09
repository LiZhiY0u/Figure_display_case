#include <cassert>
#include <cstdlib>
#include <vector>
#include "App/App.h"
#include <Arduino.h>
SerialFake Serial;
namespace { std::vector<int> calls; int fail = 0; }
namespace HAL { void Init() { calls.push_back(1); } }
namespace InputQueue { bool Init() { calls.push_back(2); return fail != 2; } }
namespace BluetoothService {
bool Init() { calls.push_back(3); return fail != 3; }
void Update() { calls.push_back(5); }
}
namespace AppTasks { bool Start() { calls.push_back(4); return fail != 4; } }
int main(int argc, char** argv)
{
    fail = argc > 1 ? std::atoi(argv[1]) : 0;
    App::Update(); assert(calls.empty());
    assert(App::Init() == (fail == 0));
    const std::vector<int> expected = fail == 2 ? std::vector<int>{1,2} :
        fail == 3 ? std::vector<int>{1,2,3} : std::vector<int>{1,2,3,4};
    assert(calls == expected);
    assert(App::Init() == (fail == 0)); assert(calls == expected);
    App::Update();
    if (fail == 0) { auto updated = expected; updated.push_back(5); assert(calls == updated); }
    else assert(calls == expected);
}
