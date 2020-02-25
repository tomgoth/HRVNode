const mongo = require('mongoose')

const HRVReading_Schema = new mongo.Schema({
    NNtoRR: {type: String, trim: true },
    AVNN: {type: String, trim: true },
    SDNN: {type: String, trim: true },
    SDANN: {type: String, trim: true },
    SDNNIDX: {type: String, trim: true },
    rMSSD: {type: String, trim: true },
    pNN50: {type: String, trim: true },
    TOTPWR: {type: String, trim: true },
    ULFPWR: {type: String, trim: true },
    VLFPWR: {type: String, trim: true },
    LFPWR: {type: String, trim: true },
    HFPWR: {type: String, trim: true },
    LFtoHF: {type: String, trim: true },

    createdAt: {type: Date, default: Date.now}

    
})

module.exports = mongo.model('HRVReading', HRVReading_Schema)