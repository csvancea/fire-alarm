"""
Routes and views for the flask application.
"""

from datetime import datetime
from flask import render_template, request
from howl import app, db
from howl.models import Measurement

@app.route('/')
@app.route('/home')
def home():
    """Renders the home page."""
    return render_template(
        'index.html',
        title='Home Page',
        year=datetime.now().year,
    )

@app.route('/list')
def list():
    """Renders the list page."""
    guid = request.args['guid']
    page = request.args.get('page', 1, type=int)
    pagination = Measurement.query.filter_by(sensor_guid=request.args['guid']).order_by(Measurement.id.desc()).paginate(page, app.config['ENTRIES_PER_PAGE'], False)

    return render_template(
        'list.html',
        title='List',
        year=datetime.now().year,
        guid=guid,
        pagination=pagination
    )
