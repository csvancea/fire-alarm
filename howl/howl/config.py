import os

basedir = os.path.abspath(os.path.dirname(__file__))

class Config(object):
    SQLALCHEMY_DATABASE_URI = 'sqlite:///' + os.path.join(basedir, 'app.db')
    ENTRIES_PER_PAGE = 10
    MQ2_SENSOR_MAX_VALUE = 1023
    SEND_NOTIFICATIONS = True

