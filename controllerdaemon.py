import serial
import atexit
import vgamepad as vg
from vgamepad import XUSB_BUTTON

global ser
global pad


def read_and_pad_input():
    global ser
    global pad

    x_input = 0
    y_input = 0
    input = int.from_bytes(ser.read(2))

    if input & 0x0001 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_LEFT_SHOULDER)

    if input & 0x0002 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_X)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_X)

    if input & 0x0004 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_Y)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_Y)
    
    if input & 0x0008 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_RIGHT_SHOULDER)

    if input & 0x0010 != 0:
        pad.left_trigger(255)
    else:
        pad.left_trigger(0)

    if input & 0x0020 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_A)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_A)

    if input & 0x0040 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_B)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_B)

    if input & 0x0080 != 0:
        pad.right_trigger(255)
    else:
        pad.right_trigger(0)

    if input & 0x0100 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_BACK)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_BACK)

    if input & 0x0200 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_GUIDE)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_GUIDE)

    if input & 0x0400 != 0:
        pad.press_button(XUSB_BUTTON.XUSB_GAMEPAD_START)
    else:
        pad.release_button(XUSB_BUTTON.XUSB_GAMEPAD_START)

    if input & 0x0800 != 0:
        y_input = -32768
    elif input & 0x4000 != 0:
        y_input = 32767
    else:
        y_input = 0

    if input & 0x1000 != 0:
        x_input = 32767
    elif input & 0x2000 != 0:
        x_input = -32768
    else:
        x_input = 0

    pad.left_joystick(x_value=x_input, y_value=y_input)

    pad.update()

def main():
    global ser
    global pad
    
    try:
        ser = serial.Serial(
        port='COM3',\
        baudrate=250000,\
        parity=serial.PARITY_EVEN,\
        stopbits=serial.STOPBITS_ONE,\
        bytesize=serial.EIGHTBITS,\
        timeout=None)
    
    except serial.SerialException:
        print("serial not found; check connection and try again")
        return

    ser.close()
    ser.open()
    pad = vg.VX360Gamepad()

    atexit.register(exit_program)

    while True: 
        read_and_pad_input()

def exit_program():
    global ser
    ser.close()
    pad.reset()
    pad.update()

if __name__ == '__main__':
    main()