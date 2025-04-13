/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

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
#include "quantum.h"

#ifdef PRECISION_ENABLE
  #include "precision.c"
#endif

// Enum定義: Precision モードスイッチ
enum my_keyball_keycodes {
    PRC_SW = KEYBALL_SAFE_RANGE, // Precision モードスイッチ   
};

// clang-format off
const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  [0] = LAYOUT_universal(
    KC_Q     , KC_W     , KC_E     , KC_R     , KC_T     ,                            KC_Y     , KC_U     , KC_I     , KC_O     , KC_P     ,
    KC_A     , KC_S     , KC_D     , KC_F     , KC_G     ,                            KC_H     , KC_J     , KC_K     , KC_L     , KC_MINS  ,
    KC_Z     , KC_X     , KC_C     , KC_V     , KC_B     ,                            KC_N     , KC_M     , KC_COMM  , KC_DOT   , KC_SLSH  ,
    KC_LCTL  , KC_LGUI  , KC_LALT  ,LSFT_T(KC_LNG2),LT(1,KC_SPC),LT(3,KC_LNG1),KC_BSPC,LT(2,KC_ENT),LSFT_T(KC_LNG2),KC_RALT,KC_RGUI, KC_RSFT
  ),

  [1] = LAYOUT_universal(
    KC_F1    , KC_F2    , KC_F3    , KC_F4    , KC_RBRC  ,                            KC_F6    , KC_F7    , KC_F8    , KC_F9    , KC_F10   ,
    KC_F5    , KC_EXLM  , S(KC_6)  ,S(KC_INT3), S(KC_8)  ,                           S(KC_INT1), KC_BTN1  , KC_PGUP  , KC_BTN2  , KC_SCLN  ,
    S(KC_EQL),S(KC_LBRC),S(KC_7)   , S(KC_2)  ,S(KC_RBRC),                            KC_LBRC  , KC_DLR   , KC_PGDN  , KC_BTN3  , KC_F11   ,
    KC_INT1  , KC_EQL   , S(KC_3)  , _______  , _______  , _______  ,      TO(2)    , TO(0)    , _______  , KC_RALT  , KC_RGUI  , KC_F12
  ),

  [2] = LAYOUT_universal(
    KC_TAB   , KC_7     , KC_8     , KC_9     , KC_MINS  ,                            KC_NUHS  , _______  , KC_BTN3  , _______  , KC_BSPC  ,
   S(KC_QUOT), KC_4     , KC_5     , KC_6     ,S(KC_SCLN),                            S(KC_9)  , KC_BTN1  , KC_UP    , KC_BTN2  , KC_QUOT  ,
    KC_SLSH  , KC_1     , KC_2     , KC_3     ,S(KC_MINS),                           S(KC_NUHS), KC_LEFT  , KC_DOWN  , KC_RGHT  , _______  ,
    KC_ESC   , KC_0     , KC_DOT   , KC_DEL   , KC_ENT   , KC_BSPC  ,      _______  , _______  , _______  , _______  , _______  , _______
  ),

  [3] = LAYOUT_universal(
    RGB_TOG  , AML_TO   , AML_I50  , AML_D50  , _______  ,                            _______  , _______  , SSNP_HOR , SSNP_VRT , SSNP_FRE ,
    RGB_MOD  , RGB_HUI  , RGB_SAI  , RGB_VAI  , SCRL_DVI ,                            _______  , _______  , _______  , _______  , _______  ,
    RGB_RMOD , RGB_HUD  , RGB_SAD  , RGB_VAD  , SCRL_DVD ,                            CPI_D1K  , CPI_D100 , CPI_I100 , CPI_I1K  , KBC_SAVE ,
    QK_BOOT  , KBC_RST  , PRC_SW  , _______  , _______  , _______  ,      _______  , _______  , _______  , _______  , KBC_RST  , QK_BOOT
  ),
};
// clang-format on

#define MOUSE_LAYER         6
#define MOUSE_LAYER_TIMEOUT 800  // ミリ秒
#define MOUSE_MOVE_THRESHOLD 4   // 動いたと見なす最小移動量

static uint16_t mouse_layer_timer = 0;
static bool mouse_layer_active = false;

// レイヤ状態の更新
layer_state_t layer_state_set_user(layer_state_t state) {
    keyball_set_scroll_mode(get_highest_layer(state) == 6);

    #ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        switch(get_highest_layer(remove_auto_mouse_layer(state, true))) {
            case 1:
            case 2:
            case 3:
            case 4:
            case 6:
                state = remove_auto_mouse_layer(state, false);
                set_auto_mouse_enable(false);
                break;
            default:
                set_auto_mouse_enable(true);
                break;
        }
    #endif
    return state;
}

// キー入力の処理
bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    // マウスボタンや修飾キーを押した場合はマウスレイヤを維持
    switch (keycode) {
        case KC_BTN1:
        case KC_BTN2:
        case KC_BTN3:
        case KC_BTN4:
        case KC_BTN5:
        case KC_LCTL:
        case KC_LALT:
        case KC_LSFT:
            return true;  // マウスレイヤにとどまる
        default:
            if (mouse_layer_active && record->event.pressed) {
                // 他のキーが押された場合、マウスレイヤを即座に終了
                layer_clear();  // レイヤ0に戻す
                mouse_layer_active = false;
            }
            break;
    }

    // Precisionスイッチ処理
    #ifdef PRECISION_ENABLE
    switch (keycode) {
        case PRC_SW:  
            precision_switch(record->event.pressed); 
            return false;
    }
    #endif

    return true;
}

// トラッキング中の処理
void pointing_device_task_user(report_mouse_t mouse_report) {
    if (mouse_layer_active) {
        if (abs(mouse_report.x) >= MOUSE_MOVE_THRESHOLD ||
            abs(mouse_report.y) >= MOUSE_MOVE_THRESHOLD ||
            abs(mouse_report.h) >= MOUSE_MOVE_THRESHOLD ||
            abs(mouse_report.v) >= MOUSE_MOVE_THRESHOLD) {
            mouse_layer_timer = timer_read();
        }

        if (timer_elapsed(mouse_layer_timer) > MOUSE_LAYER_TIMEOUT) {
            layer_clear();
            mouse_layer_active = false;
        }
    }
}

// マウスレイヤに入るための関数
void enter_mouse_layer(void) {
    layer_on(MOUSE_LAYER);  // マウスレイヤを有効化
    mouse_layer_timer = timer_read();  // タイマーをリセット
    mouse_layer_active = true;  // マウスレイヤをアクティブに
}

#ifdef OLED_ENABLE
#    include "lib/oledkit/oledkit.h"

// OLED表示の更新
void oledkit_render_info_user(void) {
    keyball_oled_render_keyinfo();
    keyball_oled_render_ballinfo();
    keyball_oled_render_layerinfo();
}
#endif
