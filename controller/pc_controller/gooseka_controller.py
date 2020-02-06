from inputs import get_gamepad
from inputs import devices

def main():
    """Just print out some event infomation when the gamepad is used."""
    while 1:
        events = get_gamepad()
        for event in events:
            # print(event.ev_type, event.code, event.state)
            if(event.code == "ABS_X" or event.code == "ABS_RX"):
                print("STEERING ", event.code, event.state)
                continue
            if(event.code == "ABS_Z" or event.code == "ABS_RZ"):
                print("GAS ", event.code, event.state)
                continue

if __name__ == "__main__":
    main()