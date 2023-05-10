/*
Copyright 2022 aki27

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include QMK_KEYBOARD_H
#include <stdio.h>
#include "quantum.h"


// Layer
#define L1_ENT LT(1, KC_ENT)
#define L2_SPC LT(2, KC_SPC)
// Modifier-Tap
#define MT_S_JA LSFT_T(KC_LNG1)
#define MT_G_EN LGUI_T(KC_LNG2)
#define MT_C_EN LCTL_T(KC_LNG2)
#define MT_A_SH LALT_T(KC_SLSH)
#define MT_A_BS LALT_T(KC_BSPC)
#define MT_A_JA LALT_T(KC_LNG1)
#define MT_S_BS LSFT_T(KC_BSPC)
// Modifiers
#define M_SG_4 SGUI(KC_4) // ScreenShot for mac
#define M_A_SC LALT(KC_SPC)
#define M_C_UP LCTL(KC_UP)
#define M_CS_T RCS(KC_T)
#define M_CS_SC RCS(KC_SPC)
#define M_A_GRV LALT(KC_GRV)
#define M_S_ENT LSFT(KC_ENT)

#define MS_BTN1 KC_MS_BTN1
#define MS_BTN2 KC_MS_BTN2
#define MS_BTN3 KC_MS_BTN3
#define MS_BTN4 KC_MS_BTN4
#define MS_BTN5 KC_MS_BTN5
// #define MS_SCR KC_MY_SCR
#define MS_SCR SCRL_MO
#define MS_CINC KC_TO_CLICKABLE_INC // マウスレイヤーが有効になるまでの待機時間を増やす
#define MS_CDEC KC_TO_CLICKABLE_DEC // マウスレイヤーが有効になるまでの待機時間を減らす
#define MS_S_DV KC_SCROLL_DIR_V // スクロールの縦軸を反転
#define MS_S_DH KC_SCROLL_DIR_H // スクロールの横軸を反転
#define MS_L_LK KC_MOUSE_LAYER_LOCK // マウスレイヤーを解除する挙動を無効化する(オンでLayer Switchingで切り替える挙動)


#define LW_LNG2 LT(1,KC_LNG2)//LT(1,KC_LNG2)  // lower
#define RS_LNG1 LT(2,KC_LNG1)//LT(2,KC_LNG1)  // raise
#define DEL_ALT ALT_T(KC_DEL)//ALT_T(KC_DEL)
//#define SPC_SFT LSFT_T(KC_SPC)
#define MS_BTN1 KC_MS_BTN1
#define MS_BTN2 KC_MS_BTN2
#define MS_BTN3 KC_MS_BTN3



enum click_state {
    NONE = 0,
    WAITING,    // マウスレイヤーが有効になるのを待つ。 Wait for mouse layer to activate.
    CLICKABLE,  // マウスレイヤー有効になりクリック入力が取れる。 Mouse layer is enabled to take click input.
    CLICKING,   // クリック中。 Clicking.
    SCROLLING   // スクロール中。 Scrolling.
};

enum click_state state;     // 現在のクリック入力受付の状態 Current click input reception status
uint16_t click_timer;       // タイマー。状態に応じて時間で判定する。 Timer. Time to determine the state of the system.

uint16_t to_clickable_time = 100;   // この秒数(千分の一秒)、WAITING状態ならクリックレイヤーが有効になる。  For this number of seconds (milliseconds), if in WAITING state, the click layer is activated.
uint16_t to_reset_time = 800; // この秒数(千分の一秒)、CLICKABLE状態ならクリックレイヤーが無効になる。 For this number of seconds (milliseconds), the click layer is disabled if in CLICKABLE state.

const uint16_t click_layer = 4;   // マウス入力が可能になった際に有効になるレイヤー。Layers enabled when mouse input is enabled

int16_t scroll_v_mouse_interval_counter;   // 垂直スクロールの入力をカウントする。　Counting Vertical Scroll Inputs
int16_t scroll_h_mouse_interval_counter;   // 水平スクロールの入力をカウントする。  Counts horizontal scrolling inputs.

int16_t scroll_v_threshold = 50;    // この閾値を超える度に垂直スクロールが実行される。 Vertical scrolling is performed each time this threshold is exceeded.
int16_t scroll_h_threshold = 50;    // この閾値を超える度に水平スクロールが実行される。 Each time this threshold is exceeded, horizontal scrolling is performed.

int16_t after_click_lock_movement = 0;      // クリック入力後の移動量を測定する変数。 Variable that measures the amount of movement after a click input.

int16_t mouse_record_threshold = 30;    // ポインターの動きを一時的に記録するフレーム数。 Number of frames in which the pointer movement is temporarily recorded.
int16_t mouse_move_count_ratio = 5;     // ポインターの動きを再生する際の移動フレームの係数。 The coefficient of the moving frame when replaying the pointer movement.





const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {

    [0] = LAYOUT(
        KC_Q,     KC_W,     KC_E,     KC_R,     KC_T,              KC_Y,     KC_U,     KC_I,     KC_O,     KC_P,
        KC_A,     KC_S,     KC_D,     KC_F,     KC_G,              KC_H,     KC_J,     KC_K,     KC_L,     KC_MINS,
        KC_Z,     KC_X,     KC_C,     KC_V,     KC_B,              KC_N,     KC_M,     KC_COMM,  KC_DOT,   KC_SLSH,
                  RGB_MOD,  MT_G_EN,  L2_SPC,   KC_LCTL,  MS_BTN1, MT_S_BS,  L1_ENT,   MT_A_JA,  RGB_RMOD
    ),
    [1] = LAYOUT(
        KC_BSLS,  KC_CIRC,  KC_EXLM,  KC_AMPR,  KC_PIPE,           KC_AT,    KC_EQL,   KC_PLUS,  KC_ASTR,  KC_PERC,
        KC_HASH,  KC_DLR,   KC_DQUO,  KC_QUOT,  KC_TILD,           KC_LEFT,  KC_DOWN,  KC_UP,    KC_RGHT,  KC_GRV,
        KC_LSFT,  KC_COLN,  KC_LPRN,  KC_LCBR,  KC_LBRC,           KC_RBRC,  KC_RCBR,  KC_RPRN,  KC_SCLN,  XXXXXXX,
                  RGB_MOD,  KC_LALT,  MO(3),    KC_LSFT,  MS_BTN1, XXXXXXX,  XXXXXXX,  XXXXXXX,  RGB_RMOD
    ),
    [2] = LAYOUT(
        KC_F1,    KC_F2,    KC_F3,    KC_F4,    KC_F5,             KC_F6,    KC_F7,    KC_F8,    KC_F9,    KC_F10,
        KC_1,     KC_2,     KC_3,     KC_4,     KC_5,              KC_6,     KC_7,     KC_8,     KC_9,     KC_0,
        KC_F11,   XXXXXXX,  KC_LSFT,  XXXXXXX,  KC_TAB,            KC_ESC,   KC_DEL,   _______,  _______,  KC_F12,
                  RGB_MOD,  XXXXXXX,  XXXXXXX,  XXXXXXX,  MS_BTN1, M_S_ENT,  MO(3),    KC_LCTL,  RGB_RMOD
    ),
    [3] = LAYOUT(
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  M_CS_T,            XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  M_A_GRV,
        M_SG_4,   XXXXXXX,  M_CS_SC,  M_A_SC,   M_C_UP,            XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  KC_INS,
                  RGB_MOD,  _______,  _______,  _______,  MS_BTN1, _______,  EE_CLR,   QK_BOOT,  RGB_RMOD
    ),
    [4] = LAYOUT(
        XXXXXXX,  XXXXXXX,  XXXXXXX,  MS_BTN2,  MS_BTN3,           MS_BTN3,  MS_BTN2,  XXXXXXX,  XXXXXXX,  TO(5),
        KC_LGUI,  XXXXXXX,  XXXXXXX,  MS_BTN4,  MS_BTN1,           MS_BTN1,  MS_BTN4,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        KC_LSFT,  KC_LCTL,  XXXXXXX,  MS_BTN5,  XXXXXXX,           XXXXXXX,  MS_BTN5,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                  XXXXXXX,  TO(0),    TO(0),    MS_SCR,   XXXXXXX, MS_SCR,   TO(0),    TO(0),    XXXXXXX
    ),
    [5] = LAYOUT(
        KC_TG_OS, XXXXXXX,  XXXXXXX,  MS_BTN2,  MS_BTN3,           MS_BTN3,  MS_BTN2,  MS_S_DV,  MS_CINC,  SCRL_IN,
        KC_LGUI,  XXXXXXX,  XXXXXXX,  MS_BTN4,  MS_BTN1,           MS_BTN1,  MS_BTN4,  MS_S_DH,  MS_CDEC,  CPI_SW,
        KC_LSFT,  XXXXXXX,  RGB_TOG,  MS_BTN5,  XXXXXXX,           XXXXXXX,  MS_BTN5,  MS_L_LK,  ROT_L15,  ROT_R15,
                  XXXXXXX,  EE_CLR,   TO(0),    MS_SCR,   XXXXXXX, MS_SCR,   TO(0),    QK_BOOT,  XXXXXXX
    ),
    [6] = LAYOUT(
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX
    ),
    [7] = LAYOUT(
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
        XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,           XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,
                  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX, XXXXXXX,  XXXXXXX,  XXXXXXX,  XXXXXXX
    )
};


keyevent_t encoder1_ccw = {
    .key = (keypos_t){.row = 3, .col = 0},
    .pressed = false
};

keyevent_t encoder1_cw = {
    .key = (keypos_t){.row = 3, .col = 9},
    .pressed = false
};

bool encoder_update_user(uint8_t index, bool clockwise) {
    if (index == 0) { /* First encoder */
        if (clockwise) {
            encoder1_cw.pressed = true;
            encoder1_cw.time = (timer_read() | 1);
            action_exec(encoder1_cw);
        } else {
            encoder1_ccw.pressed = true;
            encoder1_ccw.time = (timer_read() | 1);
            action_exec(encoder1_ccw);
        }
    }

    return true;
}

void matrix_scan_user(void) {

    if (IS_PRESSED(encoder1_ccw)) {
        encoder1_ccw.pressed = false;
        encoder1_ccw.time = (timer_read() | 1);
        action_exec(encoder1_ccw);
    }

    if (IS_PRESSED(encoder1_cw)) {
        encoder1_cw.pressed = false;
        encoder1_cw.time = (timer_read() | 1);
        action_exec(encoder1_cw);
    }

}


layer_state_t layer_state_set_user(layer_state_t state) {
    switch (get_highest_layer(state)) {
    case 1:
        //rgblight_sethsv_range(HSV_YELLOW, 0, 9);
        cocot_set_scroll_mode(true);
        break;
    case 2:
        //rgblight_sethsv_range(HSV_GREEN, 0, 9);
        cocot_set_scroll_mode(true);
        break;
    case 3:
        //rgblight_sethsv_range(HSV_CYAN, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    case 4:
        //rgblight_sethsv_range(HSV_AZURE, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    case 5:
        //rgblight_sethsv_range(HSV_BLUE, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    case 6:
        //rgblight_sethsv_range(HSV_MAGENTA, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    case 7:
        //rgblight_sethsv_range(HSV_MAGENTA, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    default:
        //rgblight_sethsv_range(HSV_RED, 0, 9);
        cocot_set_scroll_mode(false);
        break;
    }
    //rgblight_set_effect_range( 9, 36);
  return state;
};


#ifdef RGB_MATRIX_ENABLE

bool rgb_matrix_indicators_advanced_user(uint8_t led_min, uint8_t led_max) {
    int is_layer = get_highest_layer(layer_state|default_layer_state);
    HSV hsv = {0, 255, rgblight_get_val()};
    if (is_layer == 1) {
      hsv.h = 128; //CYAN
    } else if (is_layer == 2)  {
      hsv.h = 85; //GREEN
    } else if (is_layer == 3)  {
      hsv.h = 43; //YELLOW
    } else if (is_layer == 4)  {
      hsv.h = 11; //CORAL
    } else if (is_layer == 5)  {
      hsv.h = 0; //RED
    } else if (is_layer == 6)  {
      hsv.h = 64; //CHARTREUSE
    } else {
      hsv.h = 191; //PURPLE
    }
    RGB rgb = hsv_to_rgb(hsv);

    for (uint8_t i = led_min; i <= led_max; i++) {
        if (HAS_FLAGS(g_led_config.flags[i], 0x02)) {
          rgb_matrix_set_color(i, rgb.r, rgb.g, rgb.b);
        }
    }
    return false;
};

#endif






// クリック用のレイヤーを有効にする。　Enable layers for clicks
void enable_click_layer(void) {
    layer_on(click_layer);
    click_timer = timer_read();
    state = CLICKABLE;
}

// クリック用のレイヤーを無効にする。 Disable layers for clicks.
void disable_click_layer(void) {
    state = NONE;
    layer_off(click_layer);
    scroll_v_mouse_interval_counter = 0;
    scroll_h_mouse_interval_counter = 0;
}

// 自前の絶対数を返す関数。 Functions that return absolute numbers.
int16_t my_abs(int16_t num) {
    if (num < 0) {
        num = -num;
    }

    return num;
}

// 自前の符号を返す関数。 Function to return the sign.
int16_t mmouse_move_y_sign(int16_t num) {
    if (num < 0) {
        return -1;
    }

    return 1;
}

// 現在クリックが可能な状態か。 Is it currently clickable?
bool is_clickable_mode(void) {
    return state == CLICKABLE || state == CLICKING || state == SCROLLING;
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {

    switch (keycode) {
        case KC_MS_BTN1:
        case KC_MS_BTN2:
        case KC_MS_BTN3:
        case KC_MS_BTN4:
        case KC_MS_BTN5:
        {
            report_mouse_t currentReport = pointing_device_get_report();

            // どこのビットを対象にするか。 Which bits are to be targeted?
            uint8_t btn = 1 << (keycode - KC_MS_BTN1);

            if (record->event.pressed) {
                // ビットORは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットのどちらかが「1」の場合に「1」にします。
                // Bit OR compares bits in the same position on the left and right sides of the operator and sets them to "1" if either of both bits is "1".
                currentReport.buttons |= btn;
                state = CLICKING;
                after_click_lock_movement = 30;
            } else {
                // ビットANDは演算子の左辺と右辺の同じ位置にあるビットを比較して、両方のビットが共に「1」の場合だけ「1」にします。
                // Bit AND compares the bits in the same position on the left and right sides of the operator and sets them to "1" only if both bits are "1" together.
                currentReport.buttons &= ~btn;
                enable_click_layer();
            }

            pointing_device_set_report(currentReport);
            pointing_device_send();
            return false;
        }
/*
        case KC_MS_BTN4:
        case KC_MS_BTN5:
            if  (record->event.pressed) {
                enable_click_layer();
            }
*/
        case SCRL_MO:
            if (record->event.pressed) {
                state = SCROLLING;
            } else {
                enable_click_layer();   // スクロールキーを離した時に再度クリックレイヤーを有効にする。 Enable click layer again when the scroll key is released.
            }
         return false;

         default:
            if  (record->event.pressed) {
                disable_click_layer();
            }

    }

    return true;
}


report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

    int16_t current_x = mouse_report.x;
    int16_t current_y = mouse_report.y;
    int16_t current_h = 0;
    int16_t current_v = 0;

    if (current_x != 0 || current_y != 0) {

        switch (state) {
            case CLICKABLE:
                click_timer = timer_read();
                break;

            case CLICKING:
                after_click_lock_movement -= my_abs(current_x) + my_abs(current_y);

                if (after_click_lock_movement > 0) {
                    current_x = 0;
                    current_y = 0;
                }

                break;

            case SCROLLING:
            {
                int8_t rep_v = 0;
                int8_t rep_h = 0;

                // 垂直スクロールの方の感度を高める。 Increase sensitivity toward vertical scrolling.
                if (my_abs(current_y) * 2 > my_abs(current_x)) {

                    scroll_v_mouse_interval_counter += current_y;
                    while (my_abs(scroll_v_mouse_interval_counter) > scroll_v_threshold) {
                        if (scroll_v_mouse_interval_counter < 0) {
                            scroll_v_mouse_interval_counter += scroll_v_threshold;
                            rep_v += scroll_v_threshold;
                        } else {
                            scroll_v_mouse_interval_counter -= scroll_v_threshold;
                            rep_v -= scroll_v_threshold;
                        }

                    }
                } else {

                    scroll_h_mouse_interval_counter += current_x;

                    while (my_abs(scroll_h_mouse_interval_counter) > scroll_h_threshold) {
                        if (scroll_h_mouse_interval_counter < 0) {
                            scroll_h_mouse_interval_counter += scroll_h_threshold;
                            rep_h += scroll_h_threshold;
                        } else {
                            scroll_h_mouse_interval_counter -= scroll_h_threshold;
                            rep_h -= scroll_h_threshold;
                        }
                    }
                }

                current_h = rep_h / scroll_h_threshold;
                current_v = -rep_v / scroll_v_threshold;
                current_x = 0;
                current_y = 0;
            }
                break;

            case WAITING:
                if (timer_elapsed(click_timer) > to_clickable_time) {
                    enable_click_layer();
                }
                break;

            default:
                click_timer = timer_read();
                state = WAITING;
        }
    }
    else
    {
        switch (state) {
            case CLICKING:
            case SCROLLING:

                break;

            case CLICKABLE:
                if (timer_elapsed(click_timer) > to_reset_time) {
                    disable_click_layer();
                }
                break;

             case WAITING:
                if (timer_elapsed(click_timer) > 50) {
                    state = NONE;
                }
                break;

            default:
                state = NONE;
        }
    }

    mouse_report.x = current_x;
    mouse_report.y = current_y;
    mouse_report.h = current_h;
    mouse_report.v = current_v;

    return mouse_report;
}


