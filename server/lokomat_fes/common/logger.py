import logging
import logging.config


def setup_logger(devices_logging_level: int = logging.DEBUG, show_scheduler_logs: bool = True):
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
            "runner": {"handlers": ["runner"], "level": logging.INFO if show_scheduler_logs else logging.WARNING},
            "lokomat_fes": {"handlers": ["console"], "level": devices_logging_level},
            "pyScienceMode": {"handlers": ["console"], "level": devices_logging_level},
        },
    }
    logging.config.dictConfig(logging_config)
