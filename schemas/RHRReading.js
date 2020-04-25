const mongo = require('mongoose')

const RHRReading_Schema = new mongo.Schema({
    restingHeartRate: {type: Number, trim: true },
    
    createdAt: {type: Date, default: Date.now, unique: true }

    
})

module.exports = mongo.model('RHRReading', RHRReading_Schema)