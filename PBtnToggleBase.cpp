#include "PBtnToggleBase.h"

// PBtnToggleBase::state_ bit values:
//   0 - pressed, timer started
//   1 - onPress triggered
//   2 - onLongPress triggered
//   3 - onRelease triggered
//   4 - check running
//   5 - pressed button state
//   6 - last button state
//   7 - last event was long press, ignore next release, then press ad clear on press

/**
 * Create button instance
 *
 * @param btn_pin Pin where button is wired
 * @param pressed_state Pin state when button is pressed. Value LOW or HIGH depends on wiring.
 */
PBtnToggleBase::PBtnToggleBase(int btn_pin, int pressed_state) {
    PBtnToggleBase::btn_ = btn_pin;
    PBtnToggleBase::state_ = B00000000;
    state_pressed_on_high_(pressed_state==HIGH);
    PBtnToggleBase::on_press_callback_ = NULL;
    PBtnToggleBase::on_long_press_callback_ = NULL;
    PBtnToggleBase::on_release_callback_ = NULL;
    PBtnToggleBase::timer_ = millis();
}

/**
 * Set optional callback function to be called on button press event
 *
 * @param callback Callback function pointer
 */
void PBtnToggleBase::onPress(ToggleFunc callback) {
    PBtnToggleBase::on_press_callback_ = callback;
}

/**
 * Set optional callback function to be called on button long press event
 *
 * @param callback Callback function pointer
 */
void PBtnToggleBase::onLongPress(LongPressFunc callback) {
    PBtnToggleBase::on_long_press_callback_ = callback;
}

/**
 * Set optional callback function to be called on button release event
 *
 * @param callback Callback function pointer
 */
void PBtnToggleBase::onRelease(ToggleFunc callback) {
    PBtnToggleBase::on_release_callback_ = callback;
}

void PBtnToggleBase::trigger_events_(bool btn_pressed) {
    if (btn_pressed) {
        // button pressed, trigger press event if not triggered already
        Serial.printf("btn %d is pressed\n", btn_);
        if (!state_press_triggered_()) {
            Serial.printf("btn %d is being triggered\n", btn_);
            // if last event was long press, then do not trigger this press event
            if (!state_longpress_triggered_() && PBtnToggleBase::on_press_callback_) {
                Serial.printf("btn %d triggers on press callback\n", btn_);
                PBtnToggleBase::on_press_callback_(PBtnToggleBase::btn_, state_pressed_on_high_()?HIGH:LOW);
            }
            // if no long press event callback set, then stop press timer
            if (!PBtnToggleBase::on_long_press_callback_) {
                state_press_timer_started_(false);
            }

            state_press_triggered_(true);
            state_longpress_triggered_(false);
            state_release_triggered_(false);
            PBtnToggleBase::timer_ = millis();
        }
        // longpress event callback set but not triggered, button press period reached long press state
        if (PBtnToggleBase::on_long_press_callback_ && !state_longpress_triggered_() && PBtnToggleBase::timer_ + PBTNTOGGLEBASE_LONGCLICK_TIME < millis()) {
            Serial.printf("btn %d triggers on lonng press callback\n", btn_);
            bool noSkip = PBtnToggleBase::on_long_press_callback_(PBtnToggleBase::btn_, state_pressed_on_high_()?HIGH:LOW);
            // if long press callback returns true, then set longpress event status as triggered, this will make button release not to trigger release event
            if (noSkip) {
                state_press_triggered_(false);
                state_longpress_triggered_(true);
                state_release_triggered_(false);
            }
            state_press_timer_started_(false);
        }
    } else {
        Serial.printf("btn %d is released\n", btn_);
        // if no press or longpress triggered before, then stop timer
        if (!state_press_triggered_() && !state_longpress_triggered_()) {
            Serial.printf("btn %d does not have to trigger release\n", btn_);
            state_press_timer_started_(false);
            return;
        }
        // trigger release
        if (!state_release_triggered_() && !state_longpress_triggered_() && state_press_triggered_()) {
            Serial.printf("btn %d trigger release\n", btn_);
            if (PBtnToggleBase::on_release_callback_) {
                PBtnToggleBase::on_release_callback_(PBtnToggleBase::btn_, !state_pressed_on_high_()?HIGH:LOW);
            }
            state_press_timer_started_(false);

            state_press_triggered_(false);
            state_longpress_triggered_(false);
            state_release_triggered_(true);
        }
    }
}

void PBtnToggleBase::reportStatus(char *loc){
    // Serial.println(loc);
    // Serial.printf(
    //     "btn %d: is_running %d, is pressed %d, was pressed %d, timer started %d, triggered %d, long press triggered %d, release_triggered %d, press on high %d\n", 
    //     btn_, 
    //     state_is_running_(), 
    //     is_btn_pressed_(),
    //     state_was_button_pressed_(), 
    //     state_press_timer_started_(),
    //     state_press_triggered_(),
    //     state_longpress_triggered_(),
    //     state_release_triggered_(),
    //     state_pressed_on_high_()
    // );
}

/**
 * Check button state and trigger event callback functions. This method must be called in loop()
 */
void PBtnToggleBase::check() {
    if (state_is_running_()) { 
        reportStatus("state is running, ignore");
        // delay(1000);
        return;
    }
    state_is_running_(true);
    reportStatus("after set is running to true");
    

    // get current button state
    bool btn_pressed = this->is_btn_pressed_();
    bool btn_was_pressed = state_was_button_pressed_();

    // has state changed during last check
    bool btn_state_changed = btn_pressed != btn_was_pressed;  // 读第6位
    state_was_button_pressed_(btn_pressed);  // 设置第6位

    // trigger event if debounce time is passed
    // btn state没变 && 持续时间超过了PBTNTOGGLEBASE_CLICK_DEBOUNCE_TIME
    bool v = state_press_timer_started_();
    long due_time = PBtnToggleBase::timer_ + PBTNTOGGLEBASE_CLICK_DEBOUNCE_TIME;
    long cur_time = millis();
    // Serial.printf("press timer started %d, due_time %d, cur_time %d\n", v, due_time, cur_time);
    if (v && !btn_state_changed && due_time < cur_time) {
        reportStatus("before trigger");
        PBtnToggleBase::trigger_events_(btn_pressed);
        reportStatus("after trigger");
    }

    // reset timer if state has changed
    if (btn_state_changed) {
        Serial.printf("btn_pressed %d, btn_was_pressed %d", btn_pressed, btn_was_pressed);
        PBtnToggleBase::timer_ = millis();
        state_press_timer_started_(true);
        reportStatus("after state changed");
    }
    state_is_running_(false);
    reportStatus("after set is running to false");
    // delay(1000);
}

/**
 * Get pin number
 */
int PBtnToggleBase::getPin() {
    return this->btn_;
}

/**
 * Check if initial
 */
bool PBtnToggleBase::state_press_timer_started_() {
    return bitRead(PBtnToggleBase::state_, 0) == 1;
}
/**
 * Set current running state
 */
void PBtnToggleBase::state_press_timer_started_(bool set) {  // 这位运算玩得... 这么缺内存么
    Serial.printf("set btn %d timer started to %d\n", btn_, set);
    state_set_state_(set, 0);
}

/**
 * Check if press event triggered
 */
bool PBtnToggleBase::state_press_triggered_() {
    return bitRead(PBtnToggleBase::state_, 1) == 1;
}
/**
 * Set press event trigger status
 */
void PBtnToggleBase::state_press_triggered_(bool set) {
    state_set_state_(set, 1);
}
/**
 * Check if long press event triggered
 */
bool PBtnToggleBase::state_longpress_triggered_() {
    return bitRead(PBtnToggleBase::state_, 2) == 1;
}
/**
 * Set long press event trigger status
 */
void PBtnToggleBase::state_longpress_triggered_(bool set) {
    state_set_state_(set, 2);
}
/**
 * Check if release event triggered
 */
bool PBtnToggleBase::state_release_triggered_() {
    return bitRead(PBtnToggleBase::state_, 3) == 1;
}
/**
 * Set release event trigger status
 */
void PBtnToggleBase::state_release_triggered_(bool set) {
    state_set_state_(set, 3);
}

/**
 * Check if button check() is currently running
 */
bool PBtnToggleBase::state_is_running_() {
    return bitRead(PBtnToggleBase::state_, 4) == 1;
}
/**
 * Set current running state
 */
void PBtnToggleBase::state_is_running_(bool set) {
    state_set_state_(set, 4);
}

/**
 * Get pin state on button press
 */
bool PBtnToggleBase::state_pressed_on_high_() {
    return bitRead(PBtnToggleBase::state_, 5) == 1;
}
/**
 * Set pin state in button press
 */
void PBtnToggleBase::state_pressed_on_high_(bool set) {
    state_set_state_(set, 5);
}

/**
 * Get button pressed status on last check
 */
bool PBtnToggleBase::state_was_button_pressed_() {
    return bitRead(PBtnToggleBase::state_, 6) == 1;
}
/**
 * Set button pressed status for next check
 * Used to detect if button pressed status is changed
 */
void PBtnToggleBase::state_was_button_pressed_(bool set) {
    state_set_state_(set, 6);
}

/**
 * Common method for setting state bit value
 */
void PBtnToggleBase::state_set_state_(bool set, int bit) {
    if (set) {
        bitSet(PBtnToggleBase::state_, bit);
    } else {
        bitClear(PBtnToggleBase::state_, bit);
    }
}
