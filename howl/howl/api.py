from howl import app, db
from flask import request
from datetime import datetime
from howl.models import Measurement

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

    measurement = Measurement(
        sensor_guid=guid,
        gas_value=gas_value,
        gas_detected=gas_detected,
        flame_detected=flame_detected,
        created=datetime.now()
    )
    db.session.add(measurement)
    db.session.commit()

    return {'id': measurement.id}
