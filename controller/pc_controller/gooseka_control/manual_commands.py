from inputs import get_gamepad
from inputs import devices
from .commands import Commands


class ManualCommands(Commands):
    """ Gamepad controller """

    def get_command(self):
        """ Obtain the list of commands from the gamepad """
        
        code_list = []
        for event in events:
            # print(event.code, event.state)
            if (event.code == "ABS_Z"):
                code_list.append(_set_duty_left(event.state))
            if (event.code == "ABS_RZ"):
                code_list.append(_set_duty_right(event.state))

        return code_list
    
    def __init__(self):
        """ Initialization """
        
        super(Commands, self).__init__()
