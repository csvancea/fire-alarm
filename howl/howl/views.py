"""
Routes and views for the flask application.
"""

from datetime import datetime
from flask import render_template, request
from howl import app, db
from howl.models import Measurement, Sensor

@app.route('/')
@app.route('/home')
def home():
    """Renders the home page."""
    return render_template(
        'index.html',
        title='Home Page',
        year=datetime.now().year,
    )

@app.route('/list', methods = ['POST', 'GET'])
def list():
    """Renders the list page."""
    guid = request.args['guid']
    page = request.args.get('page', 1, type=int)
    pagination = Measurement.query.filter_by(sensor_guid=guid).order_by(Measurement.id.desc()).paginate(page, app.config['ENTRIES_PER_PAGE'], False)
    sensor = Sensor.query.filter_by(guid=guid).first()

    if request.method == 'POST' and app.config['SEND_NOTIFICATIONS']:
        if sensor:
            sensor.push_token = request.form['token'] if request.form['token-btn'] == 'edit' else ''
        else:
            sensor = Sensor(
                guid=guid,
                push_token=request.form['token']
            )

        db.session.add(sensor)
        db.session.commit()

    if sensor and sensor.push_token == '':
        sensor = None

    return render_template(
        'list.html',
        title='List',
        year=datetime.now().year,
        guid=guid,
        sensor=sensor,
        pagination=pagination,
        send_notifications=app.config['SEND_NOTIFICATIONS']
    )
