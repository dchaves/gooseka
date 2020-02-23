import logging
import yaml

logger = logging.getLogger(__name__)


def execute(args):
    """ The magic goes here """
    
    logger.info("Initializing controller")

    # reading configuration
    with open(args.config_file, "r") as f_stream:
        config = yaml.load(f_stream, Loader=yaml.FullLoader)
    
    pass
