import logging
import logging.config


def setup_logger(level: int = logging.DEBUG, show_exec: bool = True):
    logging_config = {
        "version": 1,
        "disable_existing_loggers": False,
        "formatters": {
            "simple": {"format": "%(asctime)s - %(name)s.%(levelname)s:\t%(message)s"},
        },
        "datefmt": "%Y-%m-%d %H:%M:%S",
        "handlers": {
            "console": {"class": "logging.StreamHandler", "formatter": "simple", "stream": "ext://sys.stdout"},
            "runner": {"class": "logging.StreamHandler", "formatter": "simple", "stream": "ext://sys.stdout"},
        },
        "loggers": {
            "runner": {"handlers": ["runner"], "level": logging.INFO if show_exec else logging.WARNING},
            "lokomat_fes": {"handlers": ["console"], "level": level},
            "pyScienceMode": {"handlers": ["console"], "level": level},
        },
    }
    logging.config.dictConfig(logging_config)
