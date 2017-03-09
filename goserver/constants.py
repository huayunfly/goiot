#!/usr/bin/env python
"""
Constants definition for GoIoT server.
"""

from enum import Enum

__author__ = 'Yun Hua'
__email__ = 'huayunflys@126.com'
__url__ = 'https://github.com/huayunfly/goiot'
__license__ = 'Apache License, Version 2.0'
__version__ = '0.1'
__status__ = 'Beta'


class GoStatus(Enum):
    """
    Status codes.
    """
    S_OK = 0
    S_ERROR = 1
    S_INVALID_NAME = 2
    S_INVALID_ID = 3
    S_INVALID_PROVIDER = 4
    S_NO_IMPLEMENTED = 5
    S_COMM_ERROR = 6
    S_PORT_ERROR = 7
    S_UNKNOWN = 8


class GoQuality(Enum):
    """
    Quality codes.
    """
    Q_NORMAL = 1


class GoOperation(Enum):
    """
    Operation codes.
    """
    OP_READ = 1
    OP_WRITE = 2
    OP_ASYNC_READ = 3
    OP_ASYNC_WRITE = 4
    OP_REFRESH = 5


class GoPrivilege(Enum):
    """
    Privilege
    """
    READABLE = 1 << 0
    WRITEABLE = 1 << 1


class GoTransactionId(Enum):
    """
    Transaction Id
    """
    ID_EMPTY = -1



GO_EU_TYPE_ANALOG = 0x1


