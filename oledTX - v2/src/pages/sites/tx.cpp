#include "pages/pages.h"
#include "hardware/OLED_screen/OLED.h"
#include "state.h"

static void render(TXState* s) {
    oledUpdateTX(s->roll, s->pitch, s->throttle, s->yaw,
                 s->axis39, s->axis41,
                 s->throttleValue, s->fps, pageGetCurrent(),
                 s->btn33, s->btn47, s->btn39, s->btn41,
                 s->voltage);
}

static void onBtn2(TXState* s) {
    // TODO: arm/disarm toggle?
    (void)s;
}

// Auto-register at startup
static struct _Reg {
    _Reg() { pageRegister({render, onBtn2, "TX"}); }
} _reg;

