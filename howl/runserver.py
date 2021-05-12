"""
This script runs the howl application using a development server.
"""

from os import environ
from howl import app

if __name__ == '__main__':
    HOST = environ.get('SERVER_HOST', 'localhost')
    try:
        PORT = int(environ.get('SERVER_PORT', '7331'))
    except ValueError:
        PORT = 7331
    app.run(HOST, PORT)
