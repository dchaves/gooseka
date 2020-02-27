import logging
from .commands import Commands
from .utils import millis


logger = logging.getLogger(__name__)

PLATEAU=0
SLOPE=1
STOP=2
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

                    # stop the car if it has reached the number of iterations
                    if self.num_iterations is not None:
                        self.num_iterations -= 1
                        if self.num_iterations == 0:
                            self.state = STOP

        # stop the car
        elif self.state == STOP:
            self.target_duty = 0

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

        #logger.info("DUTY {} TARGET {} STATE {} DIRECTION {}".format(
        #    self.current_duty, self.target_duty, self.state, self.direction))
        
        return code_list
        
    def __init__(self, config):
        """ Initialization """
        super(BenchyCommands, self).__init__(config)

        self.num_iterations = (self.config["BENCHY_ITERATIONS"]
                               if "BENCHY_ITERATIONS" in self.config else None)
        self.last_step_millis = 0
        self.state = PLATEAU
        self.current_duty = 0
        self.target_duty = 0
        self.direction = UP
        


    

