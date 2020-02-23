import numpy as np

class PID(object):
    """ Implementation of a basic pid controller """

    def step(self, error, delta_ratio):
        """ implementation of a basic pid controller

        Keyword arguments:
        error -- current error gathered from the sensors
        delta_ratio -- time passed between steps as a ratio. If the time between pid execution is always constant this parameter should be always 1.

        """

        it_prop_error = error
        it_deriv_error = (error - self.last_error)/delta_ratio
        it_integral_error = error * delta_ratio
        self.integral_error += it_integral_error

        if self.max_integral is not None:
            self.integral_error = np.clip(self.integral_error,
                                          -self.max_integral, self.max_integral) 

        control = (it_prop_error * self.kp +
                   it_deriv_error * self.kd +
                   self.integral_error * self.ki)

        # storing the current error for the next step
        self.last_error = error

        return control
    
    def __init__(self, kp, kd, ki, max_integral=None):
        """ Intialization 

        Keyword arguments:
        kp -- proportial constant
        kd -- derivative constant
        ki -- integral constant
        max_integral -- maximum error in the integral component or None (unlimited)

        """
        self.integral_error = 0
        self.last_error = 0
        self.max_integral = max_integral
        self.kp = kp
        self.kd = kd
        self.ki = ki
        
        
    
