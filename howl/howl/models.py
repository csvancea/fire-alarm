from howl import db
from datetime import datetime

class Measurement(db.Model):
    """Data model for measurements."""

    __tablename__ = 'measurements'
    id = db.Column(
        db.Integer,
        primary_key=True
    )
    sensor_guid = db.Column(
        db.String(36),
        index=False,
        unique=False,
        nullable=False
    )
    gas_value = db.Column(
        db.Integer,
        index=False,
        unique=False,
        nullable=False
    )
    gas_detected = db.Column(
        db.Boolean,
        index=False,
        unique=False,
        nullable=False,
        default=False
    )
    flame_detected = db.Column(
        db.Boolean,
        index=False,
        unique=False,
        nullable=False,
        default=False
    )
    created = db.Column(
        db.DateTime,
        index=False,
        unique=False,
        nullable=False,
        default=datetime.utcnow
    )

    def __repr__(self):
        return '<Measurement {}>'.format(self.id)
