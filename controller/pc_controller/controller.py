import os
import sys
import argparse
import logging
from gooseka_control.controller_entrypoint import execute


class Defaults:
    CONFIG_FILE = None

    
def process_args(args, defaults, description):
    """ Handle input commands
    args - list of command line arguments
    default - default command line values
    description - a string to display at the top of the help message
    """
    parser = argparse.ArgumentParser(description=description)

    # Decoder parameters
    parser.add_argument('--config_file', '-c', dest="config_file",
                        type=str, default=defaults.CONFIG_FILE,
                        help=('A YAML configuration file ' +
                              '(defaults: %(default)s)'))

    parameters = parser.parse_args(args)
    return parameters


def launch(args, defaults, description):
    """ Basic launch functionality """

    log_level = os.getenv('CONTROLLER_LOG_LEVEL', "INFO")
    log_file = os.getenv('CONTROLLER_LOG_FILE', None)

    handlers = [logging.StreamHandler()]
    if log_file is not None:
        handlers.append(logging.FileHandler(log_file))

    logging.basicConfig(
        level=log_level,
        format="%(levelname)s: %(name)20s: %(message)s",
        handlers = handlers
    )

    parameters = process_args(args, defaults, description)
    execute(parameters)


if __name__ == "__main__":
    launch(sys.argv[1:], Defaults, __doc__)
