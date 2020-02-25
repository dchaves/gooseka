
class CommandCodes(object):
    DUTY_LEFT=0
    DUTY_RIGHT=1


class Commands(object):

    def _set_duty_left(self, value):
        return (CommandCodes.DUTY_LEFT, value)

    def _set_duty_right(self, value):
        return (CommandCodes.DUTY_RIGHT, value)

    def get_command(self, telemetry):
        """ Obtain the list of commands 

        Keyword arguments:
        telemetry -- dict with telemetry information

        """

        return []
    
    def __init__(self, config):
        """ Initialization """
        
        self.config = config
