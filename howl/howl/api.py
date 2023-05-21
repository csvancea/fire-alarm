from howl import app, db
from flask import request, url_for
from datetime import datetime
from howl.models import Measurement, Sensor
import requests

@app.route('/api/v1/measurements', methods=['GET'])
def list_measurements():
    """Gets a list of measurements."""
    guid = request.args['guid']
    page = request.args.get('page', 1, type=int)
    per_page = request.args.get('per_page', app.config['ENTRIES_PER_PAGE'], type=int)
    measurements = Measurement.query.filter_by(sensor_guid=guid).order_by(Measurement.id.desc())
    pagination = measurements.paginate(page, per_page, False)

    items = []
    for item in pagination.items:
        measurement = {
            'id': item.id,
            'sensor_guid': item.sensor_guid,
            'gas_value': item.gas_value,
            'gas_detected': item.gas_detected,
            'flame_detected': item.flame_detected,
            'created': item.created
        }
        items.append(measurement)

    return {
        'items': items,
        'total_count': measurements.count()
    }

@app.route('/api/v1/measurement/add', methods=['POST'])
def add_measurement():
    """Add a measurement via POST JSON."""

    if not request.is_json:
        return {'error': 'Data is not a JSON object'}, 400

    data = request.get_json()

    if not 'guid' in data:
        return {'error': 'GUID key missing'}, 400

    if not 'gas_value' in data:
        return {'error': 'gas_value key missing'}, 400

    guid = data['guid']
    gas_value = int(data['gas_value'])
    gas_detected = int(data.get('gas_detected', '0'))
    flame_detected = int(data.get('flame_detected', '0'))
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
