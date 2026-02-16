#include "pages/pages.h"
#include "hardware/OLED_screen/OLED.h"
#include "state.h"

static void render(TXState* s) {
    oledUpdateCalibration(s->rawRoll, s->rawPitch, s->rawYaw,
                          s->offsetRoll, s->offsetPitch, s->offsetYaw);
}

static void onBtn2(TXState* s) {
    // Set center offsets
    s->offsetRoll = s->rawRoll;
    s->offsetPitch = s->rawPitch;
    s->offsetYaw = s->rawYaw;
}

// Auto-register at startup
static struct _Reg {
    _Reg() { pageRegister({render, onBtn2, "CAL"}); }
} _reg;