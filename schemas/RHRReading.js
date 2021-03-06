const mongo = require('mongoose')

const RHRReading_Schema = new mongo.Schema({
    user: {
        type: mongo.Schema.Types.ObjectId,
        ref: 'user'
    },
    restingHeartRate: {type: Number, trim: true },
    createdAt: {type: Date, default: Date.now }

    
})
RHRReading_Schema.index({ "user": 1, "createdAt": 1 }, { unique: true })

module.exports = mongo.model('RHRReading', RHRReading_Schema)