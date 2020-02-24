import logging
import yaml
from .fsm import FSM_Controller

logger = logging.getLogger(__name__)


def execute(args):
    """ The magic goes here """
    
    logger.info("Initializing controller")

    # reading configuration
    with open(args.config_file, "r") as f_stream:
        config = yaml.load(f_stream, Loader=yaml.FullLoader)

    controller = FSM_Controller(config)
    controller.loop()

    # FIXME: remove legacy code
    # now calling legacy code
    #from .legacy import main
    #main()
