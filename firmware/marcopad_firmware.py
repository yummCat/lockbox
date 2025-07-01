import board


from kmk.kmk_keyboard import KMKKeyboard
from kmk.scanners.keypad import KeysScanner
from kmk.keys import KC
from kmk.modules.macros import Macros, UnicodeModeWinC


keyboard = KMKKeyboard()


macros = Macros(unicode_mode=UnicodeModeWinC)
keyboard.modules.append(macros)


PINS = [board.D11, board.D9, board.D5, board.D7, board.D8, board.D10]


keyboard.matrix = KeysScanner(
    pins=PINS,
    value_when_pressed=False,
)


keyboard.keymap = [
    [KC.MACRO('1f438'),
     KC.MACRO('1fae1'),
     KC.MACRO('2728'),
     KC.MACRO('1f4af'),
     KC.MACRO('1f47b'),
     KC.MACRO('1f62d')]
]

if __name__ == '__main__':
    keyboard.go()