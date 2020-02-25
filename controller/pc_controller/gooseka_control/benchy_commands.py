from .commands import Commands
from .utils import millis

PLATEAU=0
SLOPE=1
UP = 0
DOWN = 1

class BenchyCommands(Commands):
    """ Gamepad controller """

    def get_command(self, telemetry):
        """ Obtain the list of commands from the gamepad 

        Keyword arguments:
        telemetry -- dict with telemetry information
        """

        current_millis = millis()

        if self.state == PLATEAU:
            if (current_millis - self.last_step_millis >
                self.config["BENCHY_MS_PLATEAU"]):
                
                self.state = SLOPE
                self.current_duty = self.target_duty
                self.last_step_millis = current_millis
                
                if (self.direction == UP and
                    (self.target_duty + self.config["BENCHY_STEP"] <= self.config["BENCHY_MAX_DUTY"])):
                    self.target_duty += self.config["BENCHY_STEP"]

                elif self.direction == UP:
                    self.direction = DOWN
                    self.target_duty -= self.config["BENCHY_STEP"]

                elif (self.direction == DOWN and
                      (self.target_duty - self.config["BENCHY_STEP"] >= self.config["BENCHY_MIN_DUTY"])):
                    self.target_duty -= self.config["BENCHY_STEP"]

                elif self.direction == DOWN:
                    self.direction = UP
                    self.target_duty += self.config["BENCHY_STEP"]

        else:
            if (current_millis -  self.last_step_millis > self.config["BENCHY_MS_STEEP"]):
                self.state = PLATEAU
                self.current_duty = self.target_duty
                self.last_step_millis = current_millis

            else:
                if self.direction == UP:
                    self.current_duty = (
                        self.target_duty - self.config["BENCHY_STEP"] +
                        self.config["BENCHY_STEP"] * (
                            current_millis - self.last_step_millis)/(
                                1.0 *self.config["BENCHY_MS_STEEP"]))

                else:
                    self.current_duty = (
                        self.target_duty + self.config["BENCHY_STEP"] -
                        self.config["BENCHY_STEP"] * (
                            current_millis - self.last_step_millis)/(
                                1.0 *self.config["BENCHY_MS_STEEP"]))
                    
        code_list = []
        code_list.append(self._set_duty_left(int(self.current_duty)))
        code_list.append(self._set_duty_right(int(self.current_duty)))
        
        return code_list
        
    def __init__(self, config):
        """ Initialization """
        super(BenchyCommands, self).__init__(config)
        
        self.last_step_millis = 0
        self.state = PLATEAU
        self.current_duty = 0
        self.target_duty = 0
        self.direction = UP
        


    

