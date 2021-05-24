"""
The flask application package.
"""

from flask import Flask
from flask_sqlalchemy import SQLAlchemy

app = Flask(__name__)
app.config.from_object('howl.config.Config')

db = SQLAlchemy(app)

import howl.api
import howl.views

db.create_all()
