#include <cassert>
#include "OledUI/ToastTimeline.h"
int main() {
    ToastTimeline::State state;
    assert(!state.Active(0)); state.Start(100);
    const unsigned times[] = {100,399,400,2399,2400,2699};
    const int positions[] = {128,18,18,18,18,127};
    for (unsigned i=0;i<6;++i) { assert(state.Active(times[i])); assert(state.X(times[i])==positions[i]); }
    assert(!state.Active(2700));
    assert(!state.Active(100)); // An expired toast must not resurrect after counter wrap.
    state.Start(0xfffffff0u);
    assert(state.Active(284)); assert(state.X(284)==18);
    assert(!state.Active(2584));
}
