const winston = require('winston');

class Logger {
    constructor(name) {
        this.logger = winston.createLogger({
            level: 'debug',
            defaultMeta: { service: name },
            transports: [
                new winston.transports.Console({
                    format: winston.format.combine(
                        winston.format.timestamp(),
                        winston.format.metadata({ fillExcept: ['timestamp', 'service', 'level', 'message'] }),
                        winston.format.colorize(),
                        this.winstonConsoleFormat()
                    )
                })
            ]
        });
    }

    winstonConsoleFormat() {
        return printf(({ timestamp, level, message, metadata }) => {
            const metadataString = metadata != null ? JSON.stringify(metadata) : '';
            return `[${timestamp}][${level}][${this.name}@${this.hostname}] ${message}. ${'metadata: ' + metadataString}`;
        })
    }

    // We expose four levels of logging for this tutorial

    debug(log, metadata) {
        this.logger.debug(log, metadata);
    }

    info(log, metadata) {
        this.logger.info(log, metadata);
    }

    warn(log, metadata) {
        this.logger.warn(log, metadata);
    }

    error(log, metadata) {
        this.logger.error(log, metadata);
    }

    log(level, log, metadata) {
        const metadataObject = {}
        if (metadata) {
            metadataObject.metadata = metadata;
        }

        this.logger[level](log, metadataObject);
    }
}

// We want our logger to have a name, like 'logging-tutorial'
module.exports = new Logger(process.env.APP_NAME)

// We will also expose a function if we want
// to use the logger with custom parameters
module.getLogger = (name) => {
    return new Logger(name);
}