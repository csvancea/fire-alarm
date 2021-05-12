from howl import app, db
from flask import request, url_for
from datetime import datetime
from howl.models import Measurement, Sensor
import requests

@app.route('/api/v1/measurement/add', methods=['POST'])
def add_measurement():
    """Add a measurement via post form data."""

    if not 'X-API-Key' in request.headers:
        return {'error': 'X-API-Key header missing'}, 400

    if not 'gas_v' in request.form:
        return {'error': 'gas_v field missing'}, 400

    guid = request.headers['X-API-Key']
    gas_value = int(request.form['gas_v'])
    gas_detected = int(request.form.get('gas', '0'))
    flame_detected = int(request.form.get('flame', '0'))
    sensor = Sensor.query.filter_by(guid=guid).first()

    # ignore first message after alarm boots up
    if gas_value == 0:
        return {'id': '0'}

    measurement = Measurement(
        sensor_guid=guid,
        gas_value=gas_value,
        gas_detected=gas_detected,
        flame_detected=flame_detected,
        created=datetime.now()
    )
    db.session.add(measurement)
    db.session.commit()

    # let the user know if sensor picked up smoke/flames
    if app.config['SEND_NOTIFICATIONS'] and sensor and sensor.push_token != '' and (gas_detected != 0 or flame_detected != 0):
        if gas_detected != 0 and flame_detected != 0:
            body = 'Smoke and Flames detected!'
        elif gas_detected != 0:
            body = 'Smoke detected!'
        else:
            body = 'Flames detected!'
        body += f'\nSmoke level: {gas_value}/{app.config["MQ2_SENSOR_MAX_VALUE"]}'

        msg = {
            'body': body,
            'title': 'Fire Alarm',
            'type': 'link',
            'url': url_for('list', guid=guid, _external=True)
        }
        requests.post('https://api.pushbullet.com/v2/pushes', json=msg, headers={'Access-Token': sensor.push_token})

    return {'id': measurement.id}
